#include "pch.h"
#include "NinRD1SnesSeq.h"

DECLARE_FORMAT(NinRD1Snes);

//  ****************
//  NinRD1SnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

NinRD1SnesSeq::NinRD1SnesSeq(RawFile* file,
  NinRD1SnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(NinRD1SnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

NinRD1SnesSeq::~NinRD1SnesSeq(void) {
}

void NinRD1SnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool NinRD1SnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  uint16_t headerOffset = GetShort(dwOffset);

  VGMHeader* header = AddHeader(headerOffset, 0);

  header->AddSimpleItem(headerOffset, 1, L"Number of Tracks");
  uint8_t numOfTrack = GetByte(headerOffset);

  headerOffset++;
  headerOffset++;

  for (uint8_t trackIndex = 0; trackIndex < numOfTrack; trackIndex++) {
    uint32_t addrTrackLowPtr = headerOffset + (trackIndex * 2);

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());

    uint16_t addrTrackStart = GetShort(addrTrackLowPtr);
      NinRD1SnesTrack* track = new NinRD1SnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
  }

  return true;
}

bool NinRD1SnesSeq::GetTrackPointers(void) {
  return true;
}

void NinRD1SnesSeq::LoadEventMap() {
  int statusByte;
  // TODO: Help to solve mystery of notes.
  const uint8_t NOTE_DUR_TABLE_FF4[490000] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x1c,0x1e,0x20,0x24,0x28,0x2a,0x2c,
        0x30,0x34,0x36,0x38,0x3c,0x40,0x42,0x44,
        0x48,0x4c,0x4e,0x50,0x54,0x58,0x5a,0x5c,
        0x60
  };
  NOTE_DUR_TABLE.assign(std::begin(NOTE_DUR_TABLE_FF4), std::end(NOTE_DUR_TABLE_FF4));
  // This command table is similar to gba's mp2k sound engine,need check.
  if (version == NINRD1SNES_NONE) {
    return;
  }
  for (statusByte = 0x00; statusByte <= 0xb0; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  for (statusByte = 0xd0; statusByte <= 0xff; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE_PARAM;
  }
  EventMap[0xb1] = EVENT_END;
  EventMap[0xb2] = EVENT_JUMP;
  EventMap[0xb3] = EVENT_GOTO;
  EventMap[0xb4] = EVENT_RETURN;
  EventMap[0xb5] = EVENT_LOOP_UNTIL;
  EventMap[0xb6] = EventMap[0xb1];
  EventMap[0xb7] = EventMap[0xb1];
  EventMap[0xb8] = EventMap[0xb1];
  EventMap[0xb9] = EventMap[0xb1];
  EventMap[0xba] = EventMap[0xb1];
  EventMap[0xbb] = EventMap[0xb1];
  EventMap[0xbc] = EVENT_TEMPO;
  EventMap[0xbd] = EVENT_TRANSPOSE;
  EventMap[0xbe] = EVENT_PROGCHANGE;
  EventMap[0xbf] = EVENT_VELOCITY;
  EventMap[0xc0] = EVENT_PAN;
  EventMap[0xc1] = EVENT_VIBRATO_DEPTH;
  EventMap[0xc2] = EVENT_VIBRATO_RATE;
  EventMap[0xc3] = EVENT_LFO_SPEED;
  EventMap[0xc4] = EVENT_VIBRATO_DELAY;
  EventMap[0xc5] = EVENT_LFO_RATE;
  EventMap[0xc6] = EVENT_PAN_LFO_SPEED;
  EventMap[0xc7] = EventMap[0xb1];
  EventMap[0xc8] = EventMap[0xb1];
  EventMap[0xc9] = EVENT_TUNING;
  EventMap[0xca] = EVENT_NOP1;
  EventMap[0xcb] = EventMap[0xb1];
  EventMap[0xcc] = EventMap[0xb1];
  EventMap[0xcd] = EVENT_SUBCOMMAND;
  EventMap[0xce] = EVENT_FINE_TUNING;
  EventMap[0xcf] = EVENT_CHORDNOTE;
  // TODO: NinRD1SnesSeq::LoadEventMap
}

double NinRD1SnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa))) * (tempo / 256.0);
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  NinRD1SnesTrack
//  ******************

NinRD1SnesTrack::NinRD1SnesTrack(NinRD1SnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void NinRD1SnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  state = STATE_NOTE;
  len = 0;
  key = 0;
  dur = 0;
  curDuration = 0;
  inSubroutine = false;
}

bool NinRD1SnesTrack::ReadEvent(void) {
  NinRD1SnesSeq* parentSeq = (NinRD1SnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  
  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  NinRD1SnesSeqEventType eventType = (NinRD1SnesSeqEventType)0;
  std::map<uint8_t, NinRD1SnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
  if (pEventType != parentSeq->EventMap.end()) {
    eventType = pEventType->second;
  }

  switch (eventType) {
  case EVENT_UNKNOWN0:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    break;

  case EVENT_UNKNOWN1: {
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte
      << std::dec << std::setfill(L' ') << std::setw(0)
      << L"  Arg1: " << (int)arg1;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    break;
  }

  case EVENT_UNKNOWN2: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte
      << std::dec << std::setfill(L' ') << std::setw(0)
      << L"  Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    break;
  }

  case EVENT_UNKNOWN3: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte
      << std::dec << std::setfill(L' ') << std::setw(0)
      << L"  Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    break;
  }

  case EVENT_UNKNOWN4: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte
      << std::dec << std::setfill(L' ') << std::setw(0)
      << L"  Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3
      << L"  Arg4: " << (int)arg4;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, newTempo);
    break;
  }

  case EVENT_TUNING: {
    state = STATE_PITCHBEND;
    uint8_t newTempo = GetByte(curOffset++);
    //AddFineTuning(beginOffset, curOffset - beginOffset, newTempo);
    AddPitchBend(beginOffset, curOffset - beginOffset, ((int16_t)(newTempo - 0x40)) * 128);
    break;
  }

  case EVENT_FINE_TUNING: {
    state = STATE_TIE;
    if (GetByte(curOffset) > 0x7F) {
      AddNoteOnNoItem(prevKey, prevVel);
      AddGenericEvent(beginOffset,
        curOffset - beginOffset,
        L"Tie State + Tie (with prev key and vel)",
        L"",
        CLR_TIE);
    }
    else
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie State", L"", CLR_TIE);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t newProg = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProg);
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, newTempo);
    break;
  }

  case EVENT_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

  case EVENT_CHORDNOTE: {
    state = STATE_TIE_END;

    //yes, this seems to be how the actual driver code handles it.  Ex. Aria of Sorrow (U): 0x80D91C0 - handle 0xCE event
    if (GetByte(curOffset) > 0x7F)
    {
      AddNoteOffNoItem(prevKey);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"End Tie State + End Tie", L"", CLR_TIE);
    }
    else
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"End Tie State", L"", CLR_TIE);
    break;
  }

  case EVENT_NOP1: {
    vel = GetByte(curOffset++);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP.1", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_LFO_SPEED: {
    state = STATE_LFO_SPEED;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Speed: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"LFO Speed", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_LFO_RATE: {
    state = STATE_LFO_RATE;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Rate: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"LFO Rate", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_PAN_LFO_SPEED: {
    state = STATE_PAN_LFO_SPEED;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Speed: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan LFO Speed", desc.str().c_str(), CLR_PAN);
    break;
  }

  case EVENT_VIBRATO_DELAY: {
    state = STATE_MODULATION_DELAY;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Vibrato Delay: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Delay", desc.str().c_str(), CLR_MODULATION);
    break;
  }

  case EVENT_VIBRATO_RATE: {
    state = STATE_MODULATION_RATE;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Vibrato Rate: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Rate", desc.str().c_str(), CLR_MODULATION);
    break;
  }

  case EVENT_VIBRATO_DEPTH: {
    state = STATE_MODULATION;
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Vibrato Depth: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str().c_str(), CLR_MODULATION);
    break;
  }

  case EVENT_VELOCITY: {
    state = STATE_VOL;
    uint8_t nextVol = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, nextVol);
    break;
  }

  case EVENT_LOOP_UNTIL: {
    uint8_t loopTimes = GetByte(curOffset++);
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    if (loopCount == 0) {
      loopCount = loopTimes;
    }
    if (loopTimes == 0) {
      bContinue = AddLoopForever(beginOffset, curOffset - beginOffset, L"Loop Until");
    }
    else {
      desc << L"Loop Count: " << (int)loopTimes << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
        << std::uppercase << dest;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Until", desc.str(), CLR_LOOP, ICON_STARTREP);
      if (loopCount != 0) {
        loopCount--;
        curOffset = dest;
      }
    }
    break;
  }

  case EVENT_PAN: {
    uint8_t pan = GetByte(curOffset++);
    state = STATE_PAN;
    // TODO: apply volume scale
    AddPan(beginOffset, curOffset - beginOffset, pan);
    break;
  }

  case EVENT_RETURN: {
    if (inSubroutine == true) {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str(), CLR_LOOP, ICON_ENDREP);
      curOffset = returnAddr;
      inSubroutine = false;
    }
    else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str(), CLR_LOOP, ICON_ENDREP);
    }
    break;
  }

  case EVENT_GOTO: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << dest;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Goto Subroutine", desc.str(), CLR_LOOP, ICON_STARTREP);
    returnAddr = curOffset;
    curOffset = dest;
    inSubroutine = true;
    break;
  }

  case EVENT_JUMP: {
    const uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << dest;
    const auto length = curOffset - beginOffset;

    curOffset = dest;
    if (!IsOffsetUsed(dest)) {
      AddGenericEvent(beginOffset, length, L"Jump", desc.str(), CLR_LOOPFOREVER);
    }
    else {
      bContinue = AddLoopForever(beginOffset, length, L"Jump");
    }
    break;
  }

  case EVENT_REST: {
    uint8_t arg = GetByte(beginOffset);
    uint8_t durIndex = arg + 0x31;
    len = parentSeq->NOTE_DUR_TABLE[durIndex] - 1;
    desc
      << L"Duration: " << (int)len;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Wait", desc.str(), CLR_REST, ICON_REST);
    AddTime(len);
    break;
  }

  case EVENT_NOTE: {
    if (statusByte <= 0x7f) {
      switch (state) {
      case STATE_NOTE:
       // curDuration = parentSeq->NOTE_DUR_TABLE[statusByte + 0x31] * 2;
        if (GetByte(curOffset) <= 0x1F)    //if the next value is 0-127, it is a velocity value
        {
          vel = (GetByte(curOffset++) * 8) + 0x07;
          if (vel >= 0x100) {
            vel = 0xff;
          }
          //if (GetByte(curOffset) <= 0x7F)    //if the next value is 0-127, it is an _unknown_ value
          //  curOffset++;
        }
        else if (GetByte(curOffset) <= 0x23 && GetByte(curOffset) >= 0x20) {
          curDuration += GetByte(curOffset++);
        }
        AddNoteByDur(beginOffset, curOffset - beginOffset, statusByte, vel, curDuration);
        break;
      case STATE_TIE:
        if (GetByte(curOffset) <= 0x1F)    //if the next value is 0-127, it is a velocity value
        {
          vel = (GetByte(curOffset++) * 8) + 0x07;
          if (vel >= 0x100) {
            vel = 0xff;
          }
          //if (GetByte(curOffset) <= 0x7F)    //if the next value is 0-127, it is an _unknown_ value
          //  curOffset++;
        }
        else if (GetByte(curOffset) <= 0x23 && GetByte(curOffset) >= 0x20) {
          curDuration += GetByte(curOffset++);
        }
        AddNoteOn(beginOffset, curOffset - beginOffset, statusByte, vel, L"Tie");
        break;
      case STATE_TIE_END:
        //if (GetByte(curOffset) <= 0x7F)    //if the next value is 0-127, it is a velocity value
        //{
        //  vel = GetByte(curOffset++);
        //  if (GetByte(curOffset) <= 0x7F)    //if the next value is 0-127, it is an _unknown_ value
        //    curOffset++;
        //}
        AddNoteOff(beginOffset, curOffset - beginOffset, statusByte, L"End Tie");
        break;
      case STATE_VOL:
        AddVol(beginOffset, curOffset - beginOffset, statusByte);
        break;
      case STATE_PAN:
        AddPan(beginOffset, curOffset - beginOffset, statusByte);
        break;
      case STATE_PITCHBEND:
        AddPitchBend(beginOffset, curOffset - beginOffset, ((int16_t)(statusByte - 0x40)) * 128);
        break;
      case STATE_MODULATION:
        AddModulation(beginOffset, curOffset - beginOffset, statusByte);
        break;
      case STATE_MODULATION_RATE:
        AddBreath(beginOffset, curOffset - beginOffset, statusByte);
        break;
      case STATE_MODULATION_DELAY:
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Delay", desc.str().c_str(), CLR_MODULATION);
        break;
      case STATE_PAN_LFO_SPEED:
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan LFO Speed", desc.str().c_str(), CLR_MODULATION);
        break;
      case STATE_LFO_SPEED:
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"LFO Speed", desc.str().c_str(), CLR_MISC);
        break;
      case STATE_LFO_RATE:
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"LFO Rate", desc.str().c_str(), CLR_MISC);
        break;
      case STATE_TUNING:
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Microtune", desc.str().c_str(), CLR_PITCHBEND);
        break;
      }
      break;
    }
    else if (statusByte >= 0x80 && statusByte <= 0xB0) {
      //it's a rest event

      AddRest(beginOffset, curOffset - beginOffset, parentSeq->NOTE_DUR_TABLE[statusByte - 0x80]);
    }
    break;
  }

  case EVENT_NOTE_PARAM: {
    /*uint8_t arg = GetByte(beginOffset);
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t noteIndex = arg1;
    uint8_t durIndex = arg - 0xcf;
    dur = parentSeq->NOTE_DUR_TABLE[durIndex];
    if (arg1 == 0xb0) {
      arg1 = key;       // Replay previous note.
    }
    key = arg1;
    if (arg2 <= 0x7f) {
      vel = arg2;
      curOffset++;
    }
    curOffset--;
    desc
      << L"Duration: " << (int)dur;
    if (key != 0) {
      AddNoteByDur(beginOffset, curOffset - beginOffset, key, vel, dur);
      AddTime(dur);
    }
    else {
      AddRest(beginOffset, curOffset - beginOffset, len);
    }*/
    state = STATE_NOTE;
    curDuration = parentSeq->NOTE_DUR_TABLE[statusByte - 0xCF];
    if (GetByte(curOffset) > 0x7F) {
      AddNoteByDurNoItem(prevKey, prevVel, curDuration);
      AddGenericEvent(beginOffset,
        curOffset - beginOffset,
        L"Duration Note State + Note On (prev key and vel)",
        L"",
        CLR_DURNOTE);
    }
    else
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Duration Note State", L"", CLR_CHANGESTATE);
    break;
  }

  case EVENT_SUBCOMMAND: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0) {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    }
    else if (arg1 == 1) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"SRCN: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Sample Number", desc.str().c_str(), CLR_PROGCHANGE);
    }
    else if (arg1 == 2) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"AR: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Attack Rate", desc.str().c_str(), CLR_ADSR);
    }
    else if (arg1 == 3) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"DR: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Decay Rate", desc.str().c_str(), CLR_ADSR);
    }
    else if (arg1 == 4) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"SL: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Sustain Level", desc.str().c_str(), CLR_ADSR);
    }
    else if (arg1 == 5) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"SR: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Sustain Rate", desc.str().c_str(), CLR_ADSR);
    }
    else if (arg1 == 6) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"GAIN: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Release Rate (GAIN)", desc.str().c_str(), CLR_ADSR);
    }
    else if (arg1 == 7) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Echo Volume L / R: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo ON / Echo Volume", desc.str().c_str(), CLR_REVERB);
    }
    else if (arg1 == 8) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Echo Feedback: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str().c_str(), CLR_REVERB);
    }
    else if (arg1 == 9) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Echo Delay: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", desc.str().c_str(), CLR_REVERB);
    }
    else if (arg1 == 0x0a) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Echo FIR Index: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str().c_str(), CLR_REVERB);
    }
    else if (arg1 == 0x0b) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Echo: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo ON / OFF", desc.str().c_str(), CLR_REVERB);
    }
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"NinRD1(Sappy?)SnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

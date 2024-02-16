#include "pch.h"
#include "WinkySoftSnesSeq.h"

DECLARE_FORMAT(WinkySoftSnes);

//  ****************
//  WinkySoftSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

WinkySoftSnesSeq::WinkySoftSnesSeq(RawFile* file,
  WinkySoftSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(WinkySoftSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

WinkySoftSnesSeq::~WinkySoftSnesSeq(void) {
}

void WinkySoftSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool WinkySoftSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  uint16_t curOffset = dwOffset;
  VGMHeader* header = AddHeader(dwOffset, 0, L"Sequence Header");

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS;) {
    if (GetByte(curOffset) != 0x6e) {
      curOffset++;
    }
    else {
      if (GetByte(curOffset + 1) >= 0x08) {
        curOffset++;
      }
      else {
        std::wstringstream trackName;
        trackName << L"Track Pointer " << GetByte(curOffset + 1) + 1;
        header->AddSimpleItem(curOffset, 4, trackName.str());

        uint16_t addrTrackStart = GetShort(curOffset + 2);
        if (addrTrackStart != 0xffff) {
          WinkySoftSnesTrack* track = new WinkySoftSnesTrack(this, addrTrackStart);
          aTracks.push_back(track);
        }
        curOffset += 4;
        trackIndex++;
        if (trackIndex == 0x07) {
          trackIndex++;   //break loop
        }
        if (GetByte(curOffset) != 0x6e) {
          trackIndex = 0x08;    // track pointer must be together.
        }
      }
    }
  }
  WinkySoftSnesTrack* track = new WinkySoftSnesTrack(this, curOffset);
  aTracks.push_back(track);
  // TODO:set default tempo for old engine
  if (version == WINKYSOFTSNES_OLD) {
    uint8_t waiting = GetByte(0x23);
    tempoBPM = GetTempoInBPM(waiting);   // default tempo
  }
  if (version == WINKYSOFTSNES_SRT3) {
    tempoBPM = GetTempoInBPM(0x80);   // default tempo
  }
  else if (version == WINKYSOFTSNES_SRT4) {
    uint8_t tempoPointer = GetByte(0x18);
    uint8_t waiting = GetByte(0x0800 + tempoPointer);
    tempoBPM = GetTempoInBPM(waiting);  // default tempo
  }
  return true;
}

bool WinkySoftSnesSeq::GetTrackPointers(void) {
  return true;
}

void WinkySoftSnesSeq::LoadEventMap() {
  if (version == WINKYSOFTSNES_NONE) {
    return;
  }
  for (unsigned int statusByte = 0x00; statusByte <= 0x66; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  EventMap[0x67] = EVENT_PAN_ENVELOPE;
  EventMap[0x68] = EVENT_UNKNOWN1;
  EventMap[0x69] = EVENT_DSP_WRITE;
  EventMap[0x6a] = EVENT_VIBRATO_RATE;
  EventMap[0x6b] = EVENT_NOISE_CONFIG;
  EventMap[0x6c] = EVENT_PITCH_BEND;
  EventMap[0x6d] = EVENT_ECHO_PARAM;
  EventMap[0x6e] = EVENT_OPEN_TRACK;
  EventMap[0x6f] = EVENT_PERCUSSION;
  EventMap[0x70] = EVENT_DETUNE;
  EventMap[0x71] = EVENT_VIBRATO_DEPTH;
  EventMap[0x72] = EVENT_VOLUME;
  EventMap[0x73] = EVENT_PAN;
  EventMap[0x74] = EVENT_LOOP_START;
  EventMap[0x75] = EVENT_LOOP_END;
  EventMap[0x76] = EVENT_CALL;
  EventMap[0x77] = EVENT_PATTERN_END;
  EventMap[0x78] = EVENT_END;
  EventMap[0x79] = EVENT_TEMPO;
  EventMap[0x7a] = EVENT_TRANSPOSE;
  EventMap[0x7b] = EVENT_PROGCHANGE;
  EventMap[0x7c] = EVENT_REST;
  EventMap[0x7d] = EVENT_VETOCITY;
  EventMap[0x7e] = EVENT_LENGTH;
  EventMap[0x7f] = EVENT_DURATION;

  // TODO: WinkySoftSnesSeq::LoadEventMap
}

double WinkySoftSnesSeq::GetTempoInBPM() {
  return GetTempoInBPM(tempo);
}

double WinkySoftSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa)) * 2) * (tempo / 256.0);
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  WinkySoftSnesTrack
//  ******************

WinkySoftSnesTrack::WinkySoftSnesTrack(WinkySoftSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void WinkySoftSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  loopLevel = 0;
  perviousKey = 0;
  noteLength = 0;
  percussion = false;

}

bool WinkySoftSnesTrack::ReadEvent(void) {
  WinkySoftSnesSeq* parentSeq = (WinkySoftSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  WinkySoftSnesSeqEventType eventType = (WinkySoftSnesSeqEventType)0;
  std::map<uint8_t, WinkySoftSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_LOOP_START: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    loopReturnAddress[loopLevel] = curOffset;
    loopCount[loopLevel] = 0;
    loopLevel++;
    break;
  }

  case EVENT_LOOP_END: {
    uint8_t prevLoopLevel = (loopLevel != 0 ? loopLevel : WINKYSOFTSNES_LOOP_LEVEL_MAX) - 1;
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Loop Count: " << (int)arg1;
    if (arg1 == 0x00) {
      bContinue = AddLoopForever(beginOffset, curOffset - beginOffset);
    }
    else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
      if (loopCount[prevLoopLevel] == 0) {
        loopCount[prevLoopLevel] = arg1;
        curOffset = loopReturnAddress[prevLoopLevel];
      }
      else {
        loopCount[prevLoopLevel]--;
        if (loopCount[prevLoopLevel] != 0) {
          curOffset = loopReturnAddress[prevLoopLevel];
        }
        else {
          loopLevel = prevLoopLevel;
        }
      }
    }
    break;
  }

  case EVENT_CALL: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pattern Play", desc.str(), CLR_LOOP, ICON_CONTROL);
    patternReturnAddress = curOffset;
    curOffset = dest;
    break;
  }

  case EVENT_PATTERN_END: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pattern End", desc.str(), CLR_LOOP, ICON_CONTROL);
    curOffset = patternReturnAddress;
    break;
  }

  case EVENT_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

  case EVENT_PERCUSSION: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Percussion Switch", desc.str(), CLR_MISC, ICON_CONTROL);
    //percussion = true;
    break;
  }

  case EVENT_ECHO_PARAM: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc 
      << L"EON: " << (int)arg1
      << L"  EFB: " << (int)arg2
      << L"  EVOL(L): " << (int)arg3
      << L"  EVOL(R): " << (int)arg4;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_VETOCITY: {
    vel = GetByte(curOffset++);
    desc
      << L"Vetocity: " << (int)vel;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Vetocity", desc.str(), CLR_DURNOTE, ICON_CONTROL);
    break;
  }

  case EVENT_LENGTH: {
    noteLength = GetByte(curOffset++);
    desc
      << L"Length: " << (int)noteLength;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Length", desc.str(), CLR_DURNOTE, ICON_CONTROL);
    break;
  }

  case EVENT_DURATION: {
    dur = GetByte(curOffset++);
    if (noteLength == 0) {
      AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }
    else {
      desc
        << L"Duration: " << (int)dur;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Duration", desc.str(), CLR_DURNOTE, ICON_CONTROL);
    }
    break;
  }

  case EVENT_VIBRATO_DEPTH: {
    uint8_t arg1 = GetByte(curOffset++);
    detuneenvduration = GetByte(curOffset++);
    desc
      << L"Depth: " << (int)arg1;
    if (detuneenvduration == 0x00) {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str(), CLR_MODULATION, ICON_CONTROL);
    }
    else {
      if (arg1 <= 0x7f) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str(), CLR_MODULATION, ICON_CONTROL);
        MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
      }
      else {
        while (GetByte(curOffset) >= 0x7f) {
          AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str(), CLR_MODULATION, ICON_CONTROL);
          MakePrevDurNoteEnd(GetTime() + dur);
          AddTime(detuneenvduration);
          //AddRest(beginOffset, curOffset - beginOffset, detuneenvduration, L"Detune and Wait");
          curOffset++;
        }
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str(), CLR_MODULATION, ICON_CONTROL);
        detuneenvduration = GetByte(curOffset++);
        MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
        curOffset++;
      }
    }
    break;
  }

  case EVENT_VIBRATO_RATE: {
    uint8_t arg1 = GetByte(curOffset++); 
      desc
      << L"Rate: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Rate", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_DETUNE: {
    uint8_t arg1 = GetByte(curOffset++);
    detuneenvduration = GetByte(curOffset++);
    if (detuneenvduration == 0x00) {
      AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, arg1);
    }
    else {
      if (arg1 <= 0x7f) {
        AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, arg1);
        //MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
      }
      else {
        while (GetByte(curOffset) >= 0x7f) {
          AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, GetByte(curOffset) & 0x7f);
          //MakePrevDurNoteEnd(GetTime() + dur);
          AddTime(detuneenvduration);
          //AddRest(beginOffset, curOffset - beginOffset, detuneenvduration, L"Detune and Wait");
          curOffset++;
        }
        AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, GetByte(curOffset));
        detuneenvduration = GetByte(curOffset++);
        //MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
        curOffset++;
      }
    }
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t arg1 = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t arg1 = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    detuneenvduration = GetByte(curOffset++);
    if (detuneenvduration == 0x00) {
      AddVol(beginOffset, curOffset - beginOffset, arg1);
    }
    else {
      if (arg1 <= 0x7f) {
        AddVol(beginOffset, curOffset - beginOffset, arg1);
        MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
      }
      else {
        while (GetByte(curOffset) >= 0x7f) {
          AddVol(beginOffset, curOffset - beginOffset, arg1 - 0x80);
          MakePrevDurNoteEnd(GetTime() + dur);
          AddTime(detuneenvduration);
          //AddRest(beginOffset, curOffset - beginOffset, detuneenvduration, L"Detune and Wait");
          curOffset++;
        }
        AddVol(beginOffset, curOffset - beginOffset, arg1);
        detuneenvduration = GetByte(curOffset++);
        MakePrevDurNoteEnd(GetTime() + dur);
        AddTime(detuneenvduration);
        curOffset++;
      }
    }
    break;
  }

  case EVENT_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
    AddPan(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_PITCH_BEND: {
    uint8_t arg1 = GetByte(curOffset++);
    AddPitchBendRange(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_NOISE_CONFIG: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Noise Freq.: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    uint8_t duration = GetByte(curOffset++);
    parentSeq->tempo = newTempo;
    if (duration == 0x00) {
      AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    }
    else {
      AddTempoBPMSlide(beginOffset, curOffset - beginOffset, 0xff - duration, parentSeq->GetTempoInBPM(newTempo));
    }
    break;
  }

  case EVENT_DSP_WRITE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc 
      << L"Register: " << (int)arg1
      << L"  Byte: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Write to DSP", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_OPEN_TRACK: {
    uint8_t arg1 = GetByte(curOffset++);
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Track ID: " << (int)arg1 << L"Track Pointer: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Open Track", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_PAN_ENVELOPE: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Envelope: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan Envelope", desc.str(), CLR_PAN, ICON_CONTROL);
    break;
  }

  case EVENT_NOTE: {

    /* Modulation-able Note VCMD Length.
    Should be unable to make it right now.
    Sorry. */
    uint8_t len = 0;
    key = GetByte(beginOffset);
    //uint64_t vcmdLength = 0;
    bool cmdStatus = false;
    uint64_t nextyBYFDS = GetByte(curOffset);
    if (GetByte(beginOffset + 1) <= 0x7f) {
      if (GetByte(beginOffset + 1) == 0x7f) {
        curOffset++;
        len = GetByte(curOffset++);
        cmdStatus = true;
       // vcmdLength = 1;
      }
      if (GetByte(beginOffset + 1) == 0x7e) {
        if (cmdStatus == false) {
          curOffset++;
          noteLength = GetByte(curOffset++);
          cmdStatus = true;
        }
        //vcmdLength = 1;
      }
      if (GetByte(beginOffset + 1) == 0x7d) {
        if (cmdStatus == false) {
          curOffset++;
          vel = GetByte(curOffset++);
        }
      //  vcmdLength = 1;
      }
      //if (percussion == false)
      //AddNoteOffNoItem(prevKey);
      AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, key, vel, noteLength);
    }
    else if (GetByte(beginOffset + 1) >= 0x80) {
      if (cmdStatus == false) {
        vel = GetByte(curOffset++) - 0x80;
        noteLength = GetByte(curOffset++);
        len = GetByte(curOffset++);
        //vcmdLength = 3;
        //if (noteLength == 0xff && perviousKey == key) {
        //  MakePrevDurNoteEnd(GetTime() + dur);
        //  desc << L"Duration: " << (int)dur;
        //  AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE, ICON_NOTE);
        //}
        //else {
        //AddNoteOffNoItem(prevKey);
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, key, vel, noteLength);
        //}
        cmdStatus = true;
      }
    }
    else if (noteLength == 0xff) {
      if (cmdStatus == false) {
        if (GetByte(beginOffset + 1) == 0x7f) {
          curOffset++;
          len = GetByte(curOffset++);
          cmdStatus = true;
          // vcmdLength = 1;
        }
        if (GetByte(beginOffset + 1) == 0x7e) {
          if (cmdStatus == false) {
            curOffset++;
            noteLength = GetByte(curOffset++);
            cmdStatus = true;
          }
          //vcmdLength = 1;
        }
        if (GetByte(beginOffset + 1) == 0x7d) {
          if (cmdStatus == false) {
            curOffset++;
            vel = GetByte(curOffset++);
          }
          //  vcmdLength = 1;
        }
        //MakePrevDurNoteEnd(GetTime() + dur);
        //desc << L"Duration: " << (int)dur;
        //AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE, ICON_NOTE);
        //cmdStatus = true;
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, key, vel, noteLength);
      }
    }
    AddTime(len);
    perviousKey = key;
    break;
  }

  case EVENT_REST: {
    uint8_t len = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, len, L"Wait");
    perviousKey = 0;
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"WinkySoftSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

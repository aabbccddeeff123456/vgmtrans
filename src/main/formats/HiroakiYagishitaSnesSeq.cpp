#include "pch.h"
#include "HiroakiYagishitaSnesSeq.h"

DECLARE_FORMAT(HiroakiYagishitaSnes);

//  ****************
//  HiroakiYagishitaSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

HiroakiYagishitaSnesSeq::HiroakiYagishitaSnesSeq(RawFile* file,
  HiroakiYagishitaSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(HiroakiYagishitaSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

HiroakiYagishitaSnesSeq::~HiroakiYagishitaSnesSeq(void) {
}

void HiroakiYagishitaSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool HiroakiYagishitaSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);
  uint16_t curOffset = dwOffset;

  for (uint8_t trackIndex = 0; trackIndex < 8; trackIndex++) {
    if (GetByte(curOffset) >= 0x80) {
      break;
    }
    std::wstringstream trackEnabled;
    trackEnabled << L"Enable Track " << (trackIndex + 1);
    header->AddSimpleItem(curOffset, 1, trackEnabled.str());
    curOffset++;
    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(curOffset, 2, trackName.str());

    uint16_t addrTrackStart = GetShort(curOffset);
    curOffset += 2;
      HiroakiYagishitaSnesTrack* track = new HiroakiYagishitaSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
  }

  if (version == HIROAKIYAGISHITASNES_MAIN_SLAYERS) {
    header->AddSimpleItem(curOffset, 1, L"Echo Delay");
    header->AddSimpleItem(curOffset + 1, 1, L"Echo Feedback");
    header->AddSimpleItem(curOffset + 2, 1, L"Echo Volume L/R");
  }

  return true;
}

bool HiroakiYagishitaSnesSeq::GetTrackPointers(void) {
  return true;
}

void HiroakiYagishitaSnesSeq::LoadEventMap() {
  if (version == HIROAKIYAGISHITASNES_NONE) {
    return;
  }

  int statusByte;
  if (version == HIROAKIYAGISHITASNES_MAIN_SLAYERS) {
    for (int statusByte = 0x00; statusByte <= 0xdf; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    EventMap[0xe0] = EVENT_PROGCHANGE;
    EventMap[0xe1] = EVENT_SRCNCHANGE;
    EventMap[0xe2] = EVENT_ADSR1CHANGE;
    EventMap[0xe3] = EVENT_ADSR2CHANGE;
    EventMap[0xe4] = EVENT_TEMPO;
    EventMap[0xe5] = EVENT_UNKNOWN3;
    EventMap[0xe6] = EVENT_VOLUME;
    EventMap[0xe7] = EVENT_TREMOLO;
    EventMap[0xe8] = EVENT_PAN;
    EventMap[0xe9] = EVENT_PAN_LFO;
    EventMap[0xea] = EVENT_ECHO_ON;
    EventMap[0xeb] = EVENT_ECHO_OFF;
    EventMap[0xec] = EVENT_NOISE_ON;
    EventMap[0xed] = EVENT_NOISE_OFF;
    EventMap[0xee] = EVENT_UNKNOWN3;
    EventMap[0xef] = EVENT_TUNING;
    EventMap[0xf0] = EVENT_TIE_STATE;
    EventMap[0xf1] = EVENT_TIE_STATE_OFF;
    EventMap[0xf2] = EVENT_VIBRATO;
    EventMap[0xf3] = EVENT_VIBRATO_OFF;
    EventMap[0xf4] = EVENT_PITCH_ENVELOPE;
    EventMap[0xf5] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xf6] = EVENT_DURATION;
    EventMap[0xf7] = EVENT_LOOP_START;
    EventMap[0xf8] = EVENT_LOOP_END;
    EventMap[0xf9] = EVENT_SUBROUTINE_END;
    EventMap[0xfa] = EVENT_SUBROUTINE;
    EventMap[0xfb] = EVENT_SUBROUTINE_END_2;
    EventMap[0xfc] = EVENT_SUBROUTINE_2;
    EventMap[0xfd] = EVENT_JUMP;
    EventMap[0xfe] = EVENT_SFXTRACK_END;
    EventMap[0xff] = EVENT_TRACK_END;
  } else if (version == HIROAKIYAGISHITASNES_MAIN_KIDOU) {
    for (int statusByte = 0x00; statusByte <= 0xcf; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    // vcmd start at 0xd0
    EventMap[0xd0] = EVENT_PROGCHANGE;
    EventMap[0xd1] = EVENT_SRCNCHANGE;
    EventMap[0xd2] = EVENT_ADSR1CHANGE;
    EventMap[0xd3] = EVENT_ADSR2CHANGE;
    EventMap[0xd4] = EVENT_TEMPO;
    EventMap[0xd5] = EVENT_UNKNOWN3;
    EventMap[0xd6] = EVENT_VOLUME;
    EventMap[0xd7] = EVENT_TREMOLO;
    EventMap[0xd8] = EVENT_PAN;
    EventMap[0xd9] = EVENT_PAN_LFO;
    EventMap[0xda] = EVENT_UNKNOWN0;    // nop
    EventMap[0xdb] = EVENT_UNKNOWN0;    // nop
    EventMap[0xdc] = EVENT_NOISE_ON;
    EventMap[0xdd] = EVENT_NOISE_OFF;
    EventMap[0xde] = EVENT_UNKNOWN0;    // nop
    EventMap[0xdf] = EVENT_UNKNOWN0;    // nop
    EventMap[0xe0] = EVENT_TIE_STATE;
    EventMap[0xe1] = EVENT_TIE_STATE_OFF;
    EventMap[0xe2] = EVENT_TUNING;
    EventMap[0xe3] = EVENT_TUNING_OFF;
    EventMap[0xe4] = EVENT_VIBRATO;
    EventMap[0xe5] = EVENT_VIBRATO_OFF;
    EventMap[0xe6] = EVENT_PITCH_ENVELOPE;
    EventMap[0xe7] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xe8] = EVENT_UNKNOWN0;    // nop
    EventMap[0xe9] = EVENT_UNKNOWN0;    // nop
    EventMap[0xea] = EVENT_UNKNOWN0;    // nop
    EventMap[0xeb] = EVENT_UNKNOWN0;    // nop
    EventMap[0xec] = EVENT_UNKNOWN0;    // nop
    EventMap[0xed] = EVENT_UNKNOWN0;    // nop
    EventMap[0xee] = EVENT_UNKNOWN0;    // nop
    EventMap[0xef] = EVENT_UNKNOWN0;    // nop
    EventMap[0xf0] = EVENT_DURATION;
    EventMap[0xf1] = EVENT_LOOP_START;
    EventMap[0xf2] = EVENT_LOOP_END;
    EventMap[0xf3] = EVENT_SUBROUTINE_END;
    EventMap[0xf4] = EVENT_SUBROUTINE;
    EventMap[0xf5] = EVENT_SUBROUTINE_END_2;
    EventMap[0xf6] = EVENT_SUBROUTINE_2;
    EventMap[0xf7] = EVENT_UNKNOWN0;    // nop
    EventMap[0xf8] = EVENT_UNKNOWN0;    // nop
    EventMap[0xf9] = EVENT_UNKNOWN0;    // nop
    EventMap[0xfa] = EVENT_UNKNOWN0;    // nop
    EventMap[0xfb] = EVENT_UNKNOWN0;    // nop
    EventMap[0xfc] = EVENT_UNKNOWN0;    // nop
    EventMap[0xfd] = EVENT_JUMP;
    EventMap[0xfe] = EVENT_SFXTRACK_END;
    EventMap[0xff] = EVENT_TRACK_END;
  } else if (version == HIROAKIYAGISHITASNES_VER2) {
    // vcmd start at 0xd0
    for (int statusByte = 0x00; statusByte <= 0xcf; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    for (int statusByte = 0xd0; statusByte <= 0xd7; statusByte++) {
      EventMap[statusByte] = EVENT_EXPRESSION;
    }
    for (int statusByte = 0xf1; statusByte <= 0xf7; statusByte++) {
      EventMap[statusByte] = EVENT_OCTAVE_INSTANT;
    }
    EventMap[0xd8] = EVENT_DRUM_KIT_ON;
    EventMap[0xd9] = EVENT_DRUM_KIT_OFF;
    EventMap[0xda] = EVENT_PROGCHANGE;
    EventMap[0xdb] = EVENT_SRCNCHANGE;
    EventMap[0xdc] = EVENT_ADSR1CHANGE;
    EventMap[0xdd] = EVENT_ADSR2CHANGE;
    EventMap[0xde] = EVENT_TRANSPOSE;
    EventMap[0xdf] = EVENT_TUNING;
    EventMap[0xe0] = EVENT_VOLUME;
    EventMap[0xe1] = EVENT_PAN;
    EventMap[0xe2] = EVENT_EXPRESSION_BASE;
    EventMap[0xe3] = EVENT_DURATION;
    EventMap[0xe4] = EVENT_DURATION_RATE;
    EventMap[0xe5] = EVENT_PITCH_ENVELOPE_VER2;
    EventMap[0xe6] = EVENT_UNKNOWN0;    // nop
    EventMap[0xe7] = EVENT_UNKNOWN0;    // nop
    EventMap[0xe8] = EVENT_VIBRATO_VER2;
    EventMap[0xe9] = EVENT_PITCH_MODULATION_ON;
    EventMap[0xea] = EVENT_PITCH_MODULATION_OFF;
    EventMap[0xeb] = EVENT_NOISE_ON_NOTSET_NCK;
    EventMap[0xec] = EVENT_NOISE_OFF;
    EventMap[0xed] = EVENT_ECHO_ON;
    EventMap[0xee] = EVENT_ECHO_OFF;
    EventMap[0xef] = EVENT_ECHO_FIR;
    EventMap[0xf0] = EVENT_OCTAVE;

    EventMap[0xf8] = EVENT_TEMPO;
    EventMap[0xf9] = EVENT_LOOP;
    EventMap[0xfa] = EVENT_SUBROUTINE_START_END;
    EventMap[0xfb] = EVENT_JUMP;
    EventMap[0xfc] = EVENT_NOISE_CLOCK;
    EventMap[0xfd] = EVENT_UNKNOWN0;    // nop
    EventMap[0xfe] = EVENT_UNKNOWN0;    // nop
    EventMap[0xff] = EVENT_TRACK_END;
  }

  // TODO: HiroakiYagishitaSnesSeq::LoadEventMap
}

double HiroakiYagishitaSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * tempo));
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  HiroakiYagishitaSnesTrack
//  ******************

HiroakiYagishitaSnesTrack::HiroakiYagishitaSnesTrack(HiroakiYagishitaSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void HiroakiYagishitaSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  tieState = false;
  tieStateOff = false;
  duration = 0;
  loopCount = 0;
  subroutineReturnAddress = 0;
  subroutineReturnAddress2 = 0;
  noteBase = 0;
  durRate = 0;
  expressionBase = 0;
}

bool HiroakiYagishitaSnesTrack::ReadEvent(void) {
  HiroakiYagishitaSnesSeq* parentSeq = (HiroakiYagishitaSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  HiroakiYagishitaSnesSeqEventType eventType = (HiroakiYagishitaSnesSeqEventType)0;
  std::map<uint8_t, HiroakiYagishitaSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_NOTE: {
    uint8_t noteNum = (statusByte & 0x7f);
    if (parentSeq->version == HIROAKIYAGISHITASNES_VER2) {
      noteNum = (noteNum & 0x0f) + noteBase - 1;
    } else {
      noteNum += 0x17;  // pitch start at 0x200
    }
    uint8_t noteDur;
    if (statusByte >= 0x80) {
      noteDur = GetByte(curOffset++);
    } else {
      noteDur = duration;
    }
    if (statusByte == 0x80 || statusByte == 0) {
      AddRest(beginOffset, curOffset - beginOffset, noteDur);
    } else {
      if (noteNum == prevKey && tieState == true) {
        desc << L"Duration: " << (int)noteDur;
        if (tieStateOff == true) {
            tieStateOff = false;
            tieState = false;
        }
        MakePrevDurNoteEnd(GetTime() + noteDur);
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str(), CLR_TIE, ICON_NOTE);
        AddTime(noteDur);
      } else {
        if (tieState == true) {
          if (tieStateOff == true) {
            tieStateOff = false;
            tieState = false;
          }
        }
        AddNoteByDur(beginOffset, curOffset - beginOffset, noteNum, 255, noteDur);
        AddTime(noteDur);
      }
    }
    // idk why do this second time
    if (tieStateOff == true) {
      tieStateOff = false;
      tieState = false;
    }
    break;
  }

case EVENT_SFXTRACK_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset, L"Track End (SFX)");
    break;
  }

case EVENT_TRACK_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

case EVENT_LOOP: {
    if (loopCount == 0) {
      loopCount = GetByte(curOffset++);
      loopReturnAddress = curOffset;
      desc << L"Loop Count: " << (int)loopCount;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop", desc.str(), CLR_LOOP,
                      ICON_STARTREP);
    } else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP,
                      ICON_ENDREP);
      loopCount--;
      if (loopCount != 0x00) {
      curOffset = loopReturnAddress;
      }
    }
    break;
  }

case EVENT_LOOP_START: {
    loopCount = GetByte(curOffset++);
  loopReturnAddress = curOffset;
    desc << L"Loop Count: " << (int)loopCount;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    break;
  }

case EVENT_LOOP_END: {
    loopCount--;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
    if (loopCount != 0x00) {
      curOffset = loopReturnAddress;
  }
    break;
  }

case EVENT_SUBROUTINE_START_END: {
  if (subroutineReturnAddress != 0x00) {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str(),
                    CLR_LOOP, ICON_ENDREP);
     curOffset = subroutineReturnAddress;
      subroutineReturnAddress = 0;
  } else {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase
         << (int)dest;
    subroutineReturnAddress = curOffset;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Goto Subroutine", desc.str(),
                    CLR_LOOP, ICON_STARTREP);
    curOffset = dest;
  }
    break;
  }

case EVENT_SUBROUTINE: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;
    subroutineReturnAddress = curOffset;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Goto Subroutine.1", desc.str(), CLR_LOOP, ICON_STARTREP);
    curOffset = dest;
    break;
  }

case EVENT_SUBROUTINE_END: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine.1 End", desc.str(), CLR_LOOP, ICON_ENDREP);
    if (subroutineReturnAddress != 0x00) {
      curOffset = subroutineReturnAddress;
      subroutineReturnAddress = 0;
  }
    break;
  }

case EVENT_SUBROUTINE_2: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;
    subroutineReturnAddress2 = curOffset;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Goto Subroutine.2", desc.str(), CLR_LOOP, ICON_STARTREP);
    curOffset = dest;
    break;
  }

case EVENT_SUBROUTINE_END_2: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine.2 End", desc.str(), CLR_LOOP, ICON_ENDREP);
    if (subroutineReturnAddress2 != 0x00) {
      curOffset = subroutineReturnAddress2;
      subroutineReturnAddress2 = 0;
  }
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int) dest;
    uint32_t length = curOffset - beginOffset;

    curOffset = dest;
    if (!IsOffsetUsed(dest)) {
      AddGenericEvent(beginOffset, length, L"Goto", desc.str().c_str(), CLR_LOOPFOREVER);
    }
    else {
      bContinue = AddLoopForever(beginOffset, length, L"Loop");
    }
    break;
  }

case EVENT_DRUM_KIT_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Drum Kit ON", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

case EVENT_DRUM_KIT_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Drum Kit OFF", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

case EVENT_ECHO_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo ON", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

case EVENT_ECHO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo OFF", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

case EVENT_PITCH_MODULATION_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation ON", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

case EVENT_PITCH_MODULATION_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation OFF", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

case EVENT_EXPRESSION: {
  // it read from a table...
  // :V
  AddExpression(beginOffset, curOffset - beginOffset, (statusByte & 0x07) + expressionBase);
    break;
}

case EVENT_EXPRESSION_BASE: {
  expressionBase = GetByte(curOffset++);
  desc << L"Expression Base: " << (int)expressionBase;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Expression Base", desc.str(), CLR_EXPRESSION, ICON_CONTROL);
    break;
  }

case EVENT_NOISE_CLOCK: {
  uint8_t arg1 = GetByte(curOffset++);
  desc << L"Noise Clock: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Clock", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

    case EVENT_NOISE_ON: {
  uint8_t arg1 = GetByte(curOffset++);
  desc << L"Noise Clock: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise ON", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

    case EVENT_NOISE_ON_NOTSET_NCK: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise ON", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

case EVENT_NOISE_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise OFF", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

case EVENT_TUNING: {
    uint8_t arg1 = GetByte(curOffset++);
  AddFineTuning(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

case EVENT_TRANSPOSE: {
    uint8_t arg1 = GetByte(curOffset++);
  AddTranspose(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

case EVENT_OCTAVE: {
    noteBase = (GetByte(curOffset++)) + 0x24;
    desc << L"Note Base: " << (int)noteBase;
  AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Base", desc.str(), CLR_MISC);
    break;
  }

case EVENT_OCTAVE_INSTANT: {
    noteBase = (statusByte - 0xf1) + 0x24;
    desc << L"Octave: " << (int)(noteBase % 0x0c) + 2;
  AddGenericEvent(beginOffset, curOffset - beginOffset, L"Octave", desc.str(), CLR_MISC);
    break;
  }

case EVENT_TUNING_OFF: {
    AddFineTuning(beginOffset, curOffset - beginOffset, 0, L"Tuning Off");
    break;
  }

case EVENT_TIE_STATE: {
    tieState = true;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie / Slur State", desc.str(), CLR_TIE);
    break;
  }

case EVENT_TIE_STATE_OFF: {
    tieStateOff = true;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie / Slur State Off", desc.str(), CLR_TIE);
    break;
  }

case EVENT_DURATION: {
    duration = GetByte(curOffset++);
  desc << L"Duration: " << (int)duration;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Duration", desc.str(), CLR_MISC);
    break;
  }

case EVENT_DURATION_RATE: {
    durRate = GetByte(curOffset++);
  desc << L"Duration Rate: " << (int)durRate;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Duration Rate", desc.str(), CLR_DURNOTE);
  if (durRate == 0) {
      tieState = true;
  } else {
    tieStateOff = true;
  }
    break;
  }

case EVENT_ECHO_FIR: {
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"Echo FIR Index: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

case EVENT_VIBRATO_VER2: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3
      << L"  Arg4: " << (int)arg4;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION);
    break;
  }

case EVENT_VIBRATO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Delay: " << (int)arg1
      << L"  Rate: " << (int)arg2
      << L"  Depth: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION);
    break;
  }

case EVENT_VIBRATO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Off", desc.str(), CLR_MODULATION);
    break;
  }

case EVENT_PITCH_ENVELOPE_VER2: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3
      << L"  Arg4: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Envelope", desc.str(), CLR_PORTAMENTO);
    break;
  }

case EVENT_PITCH_ENVELOPE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Envelope", desc.str(), CLR_PORTAMENTO);
    break;
  }

case EVENT_PITCH_ENVELOPE_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Envelope Off", desc.str(), CLR_PORTAMENTO);
    break;
  }

case EVENT_TREMOLO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Delay: " << (int)arg1
      << L"  Rate: " << (int)arg2
      << L"  Depth: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tremolo", desc.str(), CLR_MODULATION);
    break;
  }

case EVENT_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
  AddVol(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

case EVENT_PAN_LFO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Delay: " << (int)arg1
      << L"  Rate: " << (int)arg2
      << L"  Depth: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan LFO", desc.str(), CLR_MODULATION);
    break;
  }

case EVENT_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
  AddPan(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t instrNum = GetByte(curOffset++);
    uint8_t adsr1 = GetByte(curOffset++);
    uint8_t adsr2 = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, instrNum);
    break;
  }

  case EVENT_SRCNCHANGE: {
    uint8_t instrNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, instrNum, L"SRCN Change");
    break;
  }

  case EVENT_ADSR1CHANGE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"ADSR1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR 1", desc.str(), CLR_ADSR, ICON_CONTROL);
    break;
  }

  case EVENT_ADSR2CHANGE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc << L"ADSR1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR 1", desc.str(), CLR_ADSR, ICON_CONTROL);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"HiroakiYagishitaSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

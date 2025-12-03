#include "pch.h"
#include "DSESeq.h"

// https://projectpokemon.org/docs/mystery-dungeon-nds/dse-smdl-format-r13/

DECLARE_FORMAT(DSE);

//  ****************
//  DSESeq
//  ****************
#define MAX_TRACKS 16
#define SEQ_PPQN 48

DSESeq::DSESeq(RawFile* file, DSEVersion ver, uint32_t seqdataOffset,
                             std::wstring newName)
    : VGMSeq(DSEFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

DSESeq::~DSESeq(void) {
}

void DSESeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool DSESeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* smdlHeader = AddHeader(dwOffset, 64, L"SMDL Chunk Header");
  smdlHeader->AddSimpleItem(dwOffset, 4, L"Signature");
  smdlHeader->AddSimpleItem(dwOffset + 8, 4, L"File Size");
  DSESeq::unLength = GetWord(dwOffset + 8);
  smdlHeader->AddSimpleItem(dwOffset + 0x0c, 2, L"Version");
  uint16_t myVer = GetShort(dwOffset + 0x0c); // the reason why we need this is
                                              //Luminous Arc has version 0x0402, it has a different header format
  smdlHeader->AddUnknownItem(dwOffset + 0x0e, 1);
  smdlHeader->AddUnknownItem(dwOffset + 0x0f, 1);

  smdlHeader->AddSimpleItem(dwOffset + 0x18, 8, L"Last Modified Date");

  const size_t NAME_SIZE = 16;
  char rawName[NAME_SIZE + 1] = {0};
  GetBytes(dwOffset + 0x20, NAME_SIZE, rawName);
  smdlHeader->AddSimpleItem(dwOffset + 0x20, 16, L"Song Name");

  // trim name text
  for (int i = NAME_SIZE - 1; i >= 0; i--) {
    if (rawName[i] != ' ') {
      break;
    }
    rawName[i] = '\0';
  }
  // set name to the sequence
  if (rawName[0] != ('\0')) {
    std::string nameStr = std::string(rawName);
    name = string2wstring(nameStr);
  }
  else {
    name = L"DSESeq";
  }
  smdlHeader->AddUnknownItem(dwOffset + 0x30, 4);
  smdlHeader->AddUnknownItem(dwOffset + 0x34, 4);
  smdlHeader->AddUnknownItem(dwOffset + 0x38, 4);
  smdlHeader->AddUnknownItem(dwOffset + 0x3c, 4);

  if (myVer != 0x0402) {
    VGMHeader* songHeader = AddHeader(dwOffset + 0x40, 64, L"SONG Chunk Header");
    uint32_t songOffset = dwOffset + 0x40;
    songHeader->AddSimpleItem(songOffset, 4, L"Signature");
    songHeader->AddUnknownItem(songOffset + 4, 4);
    songHeader->AddUnknownItem(songOffset + 8, 4);
    songHeader->AddUnknownItem(songOffset + 0x0c, 4);
    songHeader->AddUnknownItem(songOffset + 0x10, 2);
    songHeader->AddSimpleItem(songOffset + 0x12, 2, L"Resolution of quarter note");
    SetPPQN(GetShort(songOffset + 0x12));

    songHeader->AddUnknownItem(songOffset + 0x14, 2);
    songHeader->AddSimpleItem(songOffset + 0x16, 1, L"Number of Tracks");
    uint8_t nNumTrks = GetByte(songOffset + 0x16);
    songHeader->AddSimpleItem(songOffset + 0x17, 1, L"Number of Channels");
    songHeader->AddUnknownItem(songOffset + 0x18, 4);
    songHeader->AddUnknownItem(songOffset + 0x1c, 4);
    songHeader->AddUnknownItem(songOffset + 0x20, 4);
    songHeader->AddUnknownItem(songOffset + 0x24, 4);
    songHeader->AddUnknownItem(songOffset + 0x28, 2);
    songHeader->AddUnknownItem(songOffset + 0x2a, 2);
    songHeader->AddUnknownItem(songOffset + 0x2c, 4);
    songHeader->AddUnknownItem(songOffset + 0x30, 16);

    uint32_t curOffset = songOffset + 0x40;
    uint32_t trackSize;
    for (uint8_t trackIndex = 0; trackIndex < nNumTrks; trackIndex++) {
      curOffset += 0x0c;
      trackSize = GetWord(curOffset);
      curOffset += 0x04;
      DSETrack* track = new DSETrack(this, curOffset + 4, trackSize);
      aTracks.push_back(track);
      curOffset += trackSize;
      while (GetByte(curOffset) == 0x98) {  // in case
        curOffset++;
      }
    }
  } else {
    VGMHeader* songHeader = AddHeader(dwOffset + 0x40, 32, L"SONG Chunk Header");
    uint32_t songOffset = dwOffset + 0x40;
    songHeader->AddSimpleItem(songOffset, 4, L"Signature");
    songHeader->AddUnknownItem(songOffset + 4, 4);
    songHeader->AddUnknownItem(songOffset + 8, 4);
    songHeader->AddUnknownItem(songOffset + 0x0c, 4);
    songHeader->AddUnknownItem(songOffset + 0x10, 2);
    songHeader->AddSimpleItem(songOffset + 0x12, 2, L"Resolution of quarter note");
    SetPPQN(GetShort(songOffset + 0x12));

    songHeader->AddSimpleItem(songOffset + 0x14, 1, L"Number of Tracks");
    uint8_t nNumTrks = GetByte(songOffset + 0x14);
    songHeader->AddSimpleItem(songOffset + 0x15, 1, L"Number of Channels");
    songHeader->AddUnknownItem(songOffset + 0x16, 1);
    songHeader->AddUnknownItem(songOffset + 0x17, 1);
    songHeader->AddUnknownItem(songOffset + 0x18, 1);
    songHeader->AddUnknownItem(songOffset + 0x19, 1);
    songHeader->AddSimpleItem(songOffset + 0x1a, 1, L"Master Volume?");
    songHeader->AddSimpleItem(songOffset + 0x1b, 1, L"Master Pan?");
    songHeader->AddUnknownItem(songOffset + 0x1c, 4);

    uint32_t curOffset = songOffset + 0x20; // 0x0402 only got 0x20 size
    uint32_t trackSize;
    for (uint8_t trackIndex = 0; trackIndex < nNumTrks; trackIndex++) {
      curOffset += 0x0c;
      trackSize = GetWord(curOffset);
      curOffset += 0x04;
      DSETrack* track = new DSETrack(this, curOffset + 4, trackSize);
      aTracks.push_back(track);
      curOffset += trackSize;
      while (GetByte(curOffset) == 0x98) {  // in case
        curOffset++;
      }
    }
  }

  return true;
}

bool DSESeq::GetTrackPointers(void) {
  return true;
}

void DSESeq::LoadEventMap() {
  if (version == DSE_NONE) {
    return;
  }
    const uint8_t PAUSE_DUR_TABLE[16] = {
        96, 72, 64, 48, 36, 32, 24, 18, 16, 12, 9, 8, 6, 4, 3, 2
    };
    NOTE_DUR_TABLE.assign(std::begin(PAUSE_DUR_TABLE), std::end(PAUSE_DUR_TABLE));
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  for (statusByte = 0x80; statusByte <= 0x8f; statusByte++) {
    EventMap[statusByte] = EVENT_PAUSE;
  }

  EventMap[0x90] = EVENT_REPEAT_LAST_PAUSE;
  EventMap[0x91] = EVENT_ADD_TO_LAST_PAUSE;
  EventMap[0x92] = EVENT_PAUSE8;
  EventMap[0x93] = EVENT_PAUSE16;
  EventMap[0x94] = EVENT_PAUSE24;
  EventMap[0x95] = EVENT_PAUSE_UNTIL_REL;
  EventMap[0x98] = EVENT_END;
  EventMap[0x99] = EVENT_LOOP_POINT;
  EventMap[0x9c] = EVENT_UNKNOWN1;
  EventMap[0x9d] = EVENT_UNKNOWN1;
  EventMap[0x9e] = EVENT_UNKNOWN1;

  EventMap[0xa0] = EVENT_OCTAVE;
  EventMap[0xa1] = EVENT_OCTAVE_REL;

  EventMap[0xa4] = EVENT_TEMPO;
  EventMap[0xa5] = EVENT_TEMPO;
  EventMap[0xa8] = EVENT_UNKNOWN2;
  EventMap[0xa9] = EVENT_UNKNOWN1;
  EventMap[0xaa] = EVENT_UNKNOWN1;
  EventMap[0xab] = EVENT_NOP;
  EventMap[0xac] = EVENT_PROGRAM_CHANGE;
  EventMap[0xaf] = EVENT_UNKNOWN3;

  EventMap[0xb0] = EVENT_UNKNOWN0;
  EventMap[0xb1] = EVENT_UNKNOWN1;
  EventMap[0xb2] = EVENT_UNKNOWN1;
  EventMap[0xb3] = EVENT_UNKNOWN1;
  EventMap[0xb4] = EVENT_UNKNOWN2;
  EventMap[0xb5] = EVENT_UNKNOWN1;
  EventMap[0xb6] = EVENT_UNKNOWN1;
  EventMap[0xbc] = EVENT_UNKNOWN1;
  EventMap[0xbe] = EVENT_MODULATION;
  EventMap[0xbf] = EVENT_UNKNOWN1;

  EventMap[0xc0] = EVENT_UNKNOWN3;
  EventMap[0xc3] = EVENT_UNKNOWN1;
  EventMap[0xcb] = EVENT_NOP2;

  EventMap[0xd0] = EVENT_UNKNOWN1;
  EventMap[0xd1] = EVENT_UNKNOWN1;
  EventMap[0xd2] = EVENT_UNKNOWN1;
  EventMap[0xd3] = EVENT_UNKNOWN1;
  EventMap[0xd4] = EVENT_UNKNOWN1;
  EventMap[0xd5] = EVENT_UNKNOWN1;
  EventMap[0xd6] = EVENT_UNKNOWN1;
  EventMap[0xd7] = EVENT_PITCH_BEND;
  EventMap[0xd8] = EVENT_UNKNOWN2;
  EventMap[0xdb] = EVENT_UNKNOWN1;
  EventMap[0xdc] = EVENT_UNKNOWN5;
  EventMap[0xdd] = EVENT_UNKNOWN4;
  EventMap[0xdf] = EVENT_UNKNOWN1;

  EventMap[0xe0] = EVENT_VOLUME;
  EventMap[0xe1] = EVENT_UNKNOWN1;
  EventMap[0xe2] = EVENT_UNKNOWN3;
  EventMap[0xe3] = EVENT_EXPRESSION;
  EventMap[0xe4] = EVENT_UNKNOWN5;
  EventMap[0xe5] = EVENT_UNKNOWN4;
  EventMap[0xe7] = EVENT_UNKNOWN1;
  EventMap[0xe8] = EVENT_PAN;
  EventMap[0xe9] = EVENT_UNKNOWN1;
  EventMap[0xea] = EVENT_UNKNOWN3;
  EventMap[0xec] = EVENT_UNKNOWN5;
  EventMap[0xed] = EVENT_UNKNOWN4;
  EventMap[0xef] = EVENT_UNKNOWN1;

  EventMap[0xf0] = EVENT_UNKNOWN5;
  EventMap[0xf1] = EVENT_UNKNOWN4;
  EventMap[0xf2] = EVENT_UNKNOWN2;
  EventMap[0xf3] = EVENT_UNKNOWN3;
  EventMap[0xf6] = EVENT_UNKNOWN1;

  EventMap[0xf8] = EVENT_NOP2;
  // TODO: DSESeq::LoadEventMap
}

//  ******************
//  DSETrack
//  ******************

DSETrack::DSETrack(DSESeq* parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void DSETrack::ResetVars(void) {
  SeqTrack::ResetVars();

  spcDeltaTime = 0;
  spcNoteDuration = 0;
}

bool DSETrack::ReadEvent(void) {
  DSESeq* parentSeq = (DSESeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  const uint8_t cCom_bit0 = (GetByte(curOffset) & 0x01) >> 0;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit1 = (GetByte(curOffset) & 0x02) >> 1;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit2 = (GetByte(curOffset) & 0x04) >> 2;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit3 = (GetByte(curOffset) & 0x08) >> 3;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit4 = (GetByte(curOffset) & 0x10) >> 4;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit5 = (GetByte(curOffset) & 0x20) >> 5;    // Delta times
  const uint8_t cCom_bit6 = (GetByte(curOffset) & 0x40) >> 6;    // 0=Notes / 1=Controls
  const uint8_t cCom_bit7 = (GetByte(curOffset) & 0x80) >> 7;    // 0=Notes / 1=Controls
  //if (curOffset >= 0x1000000) {
  //  return false;
  //}

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  DSESeqEventType eventType = (DSESeqEventType)0;
  std::map<uint8_t, DSESeqEventType>::iterator pEventType =
      parentSeq->EventMap.find(statusByte);
  if (pEventType != parentSeq->EventMap.end()) {
    eventType = pEventType->second;
  }

  switch (eventType) {
    case EVENT_UNKNOWN0:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;

    case EVENT_UNKNOWN1: {
      uint8_t arg1 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte << std::dec << std::setfill(L' ') << std::setw(0) << L"  Arg1: "
           << (int)arg1;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN2: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte << std::dec << std::setfill(L' ') << std::setw(0) << L"  Arg1: "
           << (int)arg1 << L"  Arg2: " << (int)arg2;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN3: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte << std::dec << std::setfill(L' ') << std::setw(0) << L"  Arg1: "
           << (int)arg1 << L"  Arg2: " << (int)arg2 << L"  Arg3: " << (int)arg3;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN4: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      uint8_t arg4 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte << std::dec << std::setfill(L' ') << std::setw(0) << L"  Arg1: "
           << (int)arg1 << L"  Arg2: " << (int)arg2 << L"  Arg3: " << (int)arg3 << L"  Arg4: "
           << (int)arg4;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN5: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      uint8_t arg4 = GetByte(curOffset++);
      uint8_t arg5 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte << std::dec << std::setfill(L' ') << std::setw(0) << L"  Arg1: "
           << (int)arg1 << L"  Arg2: " << (int)arg2 << L"  Arg3: " << (int)arg3 << L"  Arg4: "
           << (int)arg4 << L"  Arg5: "
           << (int)arg5;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_NOTE: {
      vel = GetByte(beginOffset);
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t extraLength = (arg1 & 0xc0) >> 6;
      uint8_t octaveMod = (arg1 & 0x30) >> 4;
      uint8_t noteNumber = arg1 & 0x0f;
      uint8_t currOctave = octave;
        if (octaveMod == 0x01) {
          octave -= 1;
        } else if (octaveMod == 0x02) {
          octave -= 0;
        } else if (octaveMod == 0x03) {
          octave += 1;
        } else if (octaveMod == 0x00) {
          octave -= 2;
        }
        // for some reason, they use bigendian for note duration, but littleendian for rest
        // wtf???
      if (extraLength == 1) {
        spcNoteDuration = GetByte(curOffset++);
      } else if (extraLength == 2) {
        spcNoteDuration = GetShortBE(curOffset++);
        curOffset++;
      } else if (extraLength == 3) {
        uint32_t longHi = GetByte(curOffset++) * 0x100000;  // there no GetLong so...
        spcNoteDuration = GetShortBE(curOffset++);
        curOffset++;
        spcNoteDuration += longHi;
      }
        AddNoteByDur(beginOffset, curOffset - beginOffset, noteNumber + octave * 12, vel,
                            spcNoteDuration);
        //AddTime(spcNoteDuration);
      break;
    }

                   case EVENT_MODULATION: {
      uint8_t arg1 = GetByte(curOffset++);
                     AddModulation(beginOffset, curOffset - beginOffset, arg1, L"Modulation?");
      break;
    }

      case EVENT_PITCH_BEND: {
      uint16_t bend = GetShortBE(curOffset++);  // 00-7f inc, 81-ff dec
        curOffset++;
        AddPitchBend(beginOffset, curOffset - beginOffset, bend);
        break;
      }

    case EVENT_PAUSE: {
        uint8_t durIndex = GetByte(beginOffset) % parentSeq->NOTE_DUR_TABLE.size();
      dur = parentSeq->NOTE_DUR_TABLE[durIndex];
        AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }

    case EVENT_REPEAT_LAST_PAUSE: {
        AddRest(beginOffset, curOffset - beginOffset, dur, L"Repeat Last Rest");
      break;
    }

    case EVENT_ADD_TO_LAST_PAUSE: {
      uint8_t len = GetByte(curOffset++);
        AddRest(beginOffset, curOffset - beginOffset, dur + len, L"Add to Last Rest");
      break;
    }

    case EVENT_PAUSE8: {
      dur = GetByte(curOffset++);
        AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }

    case EVENT_PAUSE16: {
      dur = GetShort(curOffset++);
      curOffset++;
        AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }

    case EVENT_PAUSE24: {
      dur = GetShort(curOffset++);
      curOffset++;
      uint32_t longHi = GetByte(curOffset++) * 0x100000;  // there no GetLong so...
      dur += longHi;
        AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }

    case EVENT_PAUSE_UNTIL_REL: {
        AddRest(beginOffset, curOffset - beginOffset, spcNoteDuration, L"Rest until Note Release");
        break;
      }

    case EVENT_LOOP_POINT: {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Infinite Loop Point", L"", CLR_LOOP);
        break;
    }

                       case EVENT_VOLUME: {
      uint16_t arg1 = GetByte(curOffset++);
      AddVol(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

          case EVENT_EXPRESSION: {
        uint8_t expression = GetByte(curOffset++);
        AddExpression(beginOffset, curOffset - beginOffset, expression);
        break;
      }

                                            case EVENT_PAN: {
      uint16_t arg1 = GetByte(curOffset++);
      AddPan(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

   case EVENT_OCTAVE: {
      uint8_t newOctave = GetByte(curOffset++);
                         AddSetOctave(beginOffset, curOffset - beginOffset, newOctave);
      break;
    }

    case EVENT_OCTAVE_REL: {
      octave += GetByte(curOffset++);
                         AddSetOctave(beginOffset, curOffset - beginOffset, octave, L"Octave (Relative)");
      break;
    }

    case EVENT_PROGRAM_CHANGE: {
      uint16_t instrNum = GetByte(curOffset++);
      AddProgramChange(beginOffset, curOffset - beginOffset, instrNum);
      break;
    }

    case EVENT_TEMPO: {
      uint16_t newTempo = GetByte(curOffset++);
      AddTempoBPM(beginOffset, curOffset - beginOffset, newTempo);
      break;
    }

    case EVENT_NOP: {
      curOffset++;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP.1", L"", CLR_MISC);
        break;
    }

    case EVENT_NOP2: {
      curOffset++;
      curOffset++;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP.2", L"", CLR_MISC);
        break;
    }

                        case EVENT_END: {
        // end of track
                          bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
        break;
      }

    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
           << (int)statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem(
          (std::wstring(L"Zipped Music is not supported! Unknown Event - ") + desc.str()).c_str(),
          LOG_LEVEL_ERR, L"DSESeq"));
      bContinue = false;
      break;
  }

  // std::wostringstream ssTrace;
  // ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase <<
  // beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) <<
  // curOffset << std::endl; OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

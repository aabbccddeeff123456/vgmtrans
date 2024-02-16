#include "pch.h"
#include "WolfTeamSnesSeq.h"

DECLARE_FORMAT(WolfTeamSnes);

//  ****************
//  WolfTeamSnesSeq
//  ****************
#define MAX_TRACKS  15
#define SEQ_PPQN    48


WolfTeamSnesSeq::WolfTeamSnesSeq(RawFile *file, WolfTeamSnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
  : VGMSeq(WolfTeamSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {

  //AddContainer<TriAcePS1ScorePattern>(aScorePatterns);

  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);
}

WolfTeamSnesSeq::~WolfTeamSnesSeq(void) {

}

bool WolfTeamSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  uint32_t curOffset = dwOffset;
  uint32_t beginOffset = curOffset;
  musicBaseAddr = dwOffset;
  uint8_t statusByte = GetByte(curOffset++);
  VGMHeader* header = AddHeader(dwOffset, 0x50, L"Sequence Header");
  header->AddSimpleItem(dwOffset + 0x20, 2, L"Track Size");
  WolfTeamSnesSeq::unLength = GetShort(dwOffset + 0x20);
  header->AddTempo(dwOffset + 0x22, 1);
  tempoBPM = GetByte(dwOffset + 0x22);
  AlwaysWriteInitialTempo(tempoBPM);
  uint8_t currentChannel = 0;
  for (int i = 0; i < MAX_TRACKS; i++) {
    if (GetByte(dwOffset + 0x23 + (3 * i)) != 0x00) {
      trackSectionPointer[currentChannel] = dwOffset + GetShort(dwOffset + 0x23 + (3 * i) + 1);
      WolfTeamSnesTrack* track = new WolfTeamSnesTrack(this, dwOffset + GetShort(trackSectionPointer[currentChannel]));
      aTracks.push_back(track);
      currentChannel++;
      //VGMHeader* TrkInfoBlock = TrkInfoHeader->AddHeader(dwOffset + 0x23 + 3 * i, 6, L"Track Info");
    }
  }
  return true;
}

bool WolfTeamSnesSeq::GetTrackPointers(void) {
  //VGMHeader* TrkInfoHeader = header->AddHeader(dwOffset + 0x23, 3 * 15, L"Track Info Blocks");



  return true;
}

void WolfTeamSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

//void WolfTeamSnesSeq::LoadEventMap() {
//  for (unsigned int statusByte = 0x00; statusByte <= 0x8f; statusByte++) {
//    EventMap[statusByte] = EVENT_NOTE;
//  }
//  EventMap[0x90] = 0x90;
//}

void WolfTeamSnesSeq::LoadEventMap() {
  int statusByte;

  for (statusByte = 0x01; statusByte <= 0x8f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  EventMap[0x96] = EVENT_PROGCHANGE;

}

uint16_t WolfTeamSnesSeq::ConvertToAPUAddress(uint16_t offset) {
  return offset;
}

uint16_t WolfTeamSnesSeq::GetShortAddress(uint32_t offset) {
  return ConvertToAPUAddress(GetShort(offset));
}

double WolfTeamSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * tempo));
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  WolfTeamSnesTrack
//  ******************

WolfTeamSnesTrack::WolfTeamSnesTrack(WolfTeamSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void WolfTeamSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();
  currentPattern = 1;
  loopGroup = 0;
  currentChannel++;
}

bool WolfTeamSnesTrack::ReadEvent(void) {
  WolfTeamSnesSeq *parentSeq = (WolfTeamSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  uint8_t event_dur = 0;
  bool bContinue = true;

  std::wstringstream desc;

  //WolfTeamSnesSeqEventType eventType = (WolfTeamSnesSeqEventType) 0;
  //std::map<uint8_t, WolfTeamSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
  //if (pEventType != parentSeq->EventMap.end()) {
  //  eventType = pEventType->second;
  //}
  if (statusByte <= 0x7f) {
    AddTime(GetByte(curOffset++)); //Delta time from "Note on" to "Next command(op-code)".
    uint8_t note_dur;
    uint8_t note_number;
    uint8_t velocity;
    note_dur = GetByte(curOffset++);  //Delta time from "Note on" to "Note off".
    //else note_dur = impliedNoteDur;
    velocity = GetByte(curOffset++) / 2;
    //else velocity = impliedVelocity;
    if (parentSeq->version == WOLFTEAMSNES_LATER) {
      note_number = statusByte;
    }
    else {
      uint8_t noteOctave = ((statusByte >> 4) & 0x0f);
      note_number = (statusByte & 0x0f) + (noteOctave * 0x0c);
    }
    AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, note_number, velocity, note_dur);
    return bContinue;
  }
  switch (statusByte) {
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

  case 0xa2: {
    uint8_t arg1 = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case 0x9b: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Modulation Status", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case 0xaf: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"AR / DR: " << (int)arg1
      << L"  SR / RR: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Modity", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case 0x94: {
    AddTime(GetByte(curOffset++));
    uint8_t bend = GetByte(curOffset++);
    AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, bend);
    break;
  }

  case 0xad: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Reverb", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case 0xae: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Use Volume Table", desc.str(), CLR_VOLUME, ICON_CONTROL);
    break;
  }

  case 0xb0: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume From Table", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case 0xb2:
  {
    curOffset++;
    AddUnknown(beginOffset, curOffset - beginOffset);
    break;
  }

  case 0x95: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0x9c: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1
      << L"  Depth: " << (int)arg2
      << L"  Time: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Modulation", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case 0xa3: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Status", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case 0x92: {
    loopCount[loopGroup] = 0;
    loopReturnAddr[loopGroup] = curOffset;
    loopReturnPattern[loopGroup] = currentPattern;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    loopGroup++;
    break;
  }

  case 0x93: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Loop Count: " << (int)arg1;
    uint8_t prevLoopGroup = loopGroup - 1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", L"", CLR_LOOP, ICON_ENDREP);
    if (arg1 != 0) {
      if (loopCount[prevLoopGroup]++ != arg1) {
        currentPattern = loopReturnPattern[prevLoopGroup];
        curOffset = loopReturnAddr[prevLoopGroup];
      }
    }
    else {
      bContinue = false;
    }
    break;
  }

  case 0x97: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0x98: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddExpression(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0x99: {
    AddTime(GetByte(curOffset++));
    uint8_t pan = GetByte(curOffset++);
    AddPan(beginOffset, curOffset - beginOffset, pan);
    break;
  }

  case 0xaa: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"Left: " << (int)arg1
      << L"  Right: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Sound Trigger", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case 0x96: {
    uint8_t newProgNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProgNum, true);
    break;
  }

  case 0x90: {
    uint8_t rest = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, rest);
    break;
  }

  case 0xe0: {
    uint8_t rest = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, rest);
    break;
  }

  case 0xe1: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0xe2: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddPan(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0xe3: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Legato(No Key-Off) Status", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case 0xe4: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Modulation Status", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case 0xe5: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1
      << L"  Depth: " << (int)arg2
      << L"  Time: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Modulation", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case 0xe6: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddExpression(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0xe7: {
    AddTime(GetByte(curOffset++));
    uint8_t val = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, val);
    break;
  }

  case 0xec: {
    uint8_t newProgNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProgNum, true);
    break;
  }

  case 0xee: {
    AddTime(GetByte(curOffset++));
    uint8_t bend = GetByte(curOffset++);
    AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, bend);
    break;
  }

  case 0xef: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"Left: " << (int)arg1
      << L"  Right: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Sound Trigger", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }


  case 0xf0: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Status", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case 0xf2: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 >= 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Status", desc.str(), CLR_UNKNOWN, ICON_CONTROL);
    break;
  }

  case 0xf4: {
    uint8_t arg1 = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case 0xf7: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 == 0x01) {
      desc
        << L"Status: ON";
    }
    else if (arg1 == 0x00) {
      desc
        << L"Status: OFF";
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Unknown Status", desc.str(), CLR_UNKNOWN, ICON_CONTROL);
    break;
  }

  case 0xf8: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", L"", CLR_LOOP, ICON_ENDREP);
      bContinue = false;
    break;
  }

  case 0xf9: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    break;
  }

  case 0xe8:
  case 0xe9:
  case 0xea:
  case 0xeb:
  case 0xed:
  case 0xf1:
  case 0xf3:
  case 0xf5:
  case 0xf6:
  case 0xfa:
  case 0xfc:
  case 0xfe:
  {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP", desc.str(), CLR_MISC);
    break;
  }

  case 0xfd: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Score Pattern End", desc.str(), CLR_TRACKEND);
    uint8_t currentChannel = channel;
      if (currentChannel > 8) {
        currentChannel--;     //channel 9 reserved for drum for some reasons
      }
    if (GetShort(parentSeq->trackSectionPointer[currentChannel] + currentPattern * 2) != 0xffff) {
      curOffset = parentSeq->musicBaseAddr + GetShort(parentSeq->trackSectionPointer[currentChannel] + currentPattern * 2);
      currentPattern++;
    }
    else {
      bContinue = false;
    }
    break;
  }

  case 0xff: {
    AddEndOfTrack(beginOffset, curOffset - beginOffset);
    bContinue = false;
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
      LOG_LEVEL_ERR,
      std::wstring(L"WolfTeamSnesSeq")));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint16_t WolfTeamSnesTrack::ConvertToAPUAddress(uint16_t offset) {
  WolfTeamSnesSeq *parentSeq = (WolfTeamSnesSeq *) this->parentSeq;
  return parentSeq->ConvertToAPUAddress(offset);
}

uint16_t WolfTeamSnesTrack::GetShortAddress(uint32_t offset) {
  WolfTeamSnesSeq *parentSeq = (WolfTeamSnesSeq *) this->parentSeq;
  return parentSeq->GetShortAddress(offset);
}

// The following two functions are overridden so that events become children of the Score Patterns and not the tracks.
bool WolfTeamSnesTrack::IsOffsetUsed(uint32_t offset) {
  return false;
}


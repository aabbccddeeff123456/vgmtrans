#include "pch.h"
#include "SensibleSnesSeq.h"

DECLARE_FORMAT(SensibleSnes);

//  ****************
//  SensibleSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

SensibleSnesSeq::SensibleSnesSeq(RawFile* file,
  SensibleSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(SensibleSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
  headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

SensibleSnesSeq::~SensibleSnesSeq(void) {
}

void SensibleSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool SensibleSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0, L"Sequence Header");

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint16_t addrTrackLowPtr = dwOffset + (trackIndex * 2);
    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    uint16_t addrTrackOffset = GetShort(addrTrackLowPtr);
    uint16_t addrTrackStart = dwOffset + addrTrackOffset;
    if (addrTrackOffset != 0x0000) {
      header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());
      SensibleSnesTrack* track = new SensibleSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
    else {
      header->AddSimpleItem(addrTrackLowPtr, 2, L"NULL");
    }
  }

  return true;
}

bool SensibleSnesSeq::GetTrackPointers(void) {
  return true;
}

void SensibleSnesSeq::LoadEventMap() {
  if (version == SENSIBLESNES_NONE) {
    return;
  }
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  EventMap[0x80] = EVENT_PROGCHANGE;
  EventMap[0x81] = EVENT_PAN;
  EventMap[0x82] = EVENT_VOLENV;
  EventMap[0x83] = EVENT_NOISE;
  EventMap[0x84] = EVENT_TEMPO;
  EventMap[0x85] = EVENT_PITCHENV;
  EventMap[0x86] = EVENT_JUMP;
  EventMap[0x87] = EVENT_LOOPSTART;
  EventMap[0x88] = EVENT_LOOPEND;
  EventMap[0x89] = EVENT_TRACKEND;
  EventMap[0x8a] = EVENT_REST;
  EventMap[0x8b] = EVENT_TRANSPOSE;
  EventMap[0x8c] = EVENT_GOTOSUBROUTINE;
  EventMap[0x8d] = EVENT_SUBROUTINEEND;
  EventMap[0x8e] = EVENT_PANFADE;
  EventMap[0x8f] = EVENT_MASTERVOL;
  EventMap[0x90] = EVENT_GLOBALTRANSPOSE;
  EventMap[0x91] = EVENT_NOISENOTE;
  EventMap[0x92] = EVENT_TUNING;
  EventMap[0x93] = EVENT_PITCHFADE;
  EventMap[0x94] = EVENT_ECHO_ON;
  EventMap[0x95] = EVENT_ECHO_VOLUME;
  EventMap[0x96] = EVENT_ECHO_PARAM;
  EventMap[0x97] = EVENT_ADSR;
  EventMap[0x98] = EVENT_ECHO_FIR;

  // TODO: SensibleSnesSeq::LoadEventMap
}

double SensibleSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * T0FREQ)) * (tempo / 256.0);
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

//  ******************
//  SensibleSnesTrack
//  ******************

SensibleSnesTrack::SensibleSnesTrack(SensibleSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void SensibleSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  subroutineGroup = 0;
  loopGroup = 0;
  loopCount[loopGroup] = 0;

}

bool SensibleSnesTrack::ReadEvent(void) {
  SensibleSnesSeq* parentSeq = (SensibleSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  SensibleSnesSeqEventType eventType = (SensibleSnesSeqEventType)0;
  std::map<uint8_t, SensibleSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_ECHO_VOLUME: {
    uint8_t spcEVOL_L = GetByte(curOffset++);
    uint8_t spcEVOL_R = GetByte(curOffset++);
    desc << L"Volume Left: " << (int)spcEVOL_L << L"  Volume Right: " << (int)spcEVOL_R;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Echo Volume",
      desc.str().c_str(),
      CLR_REVERB,
      ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_ON: {
    uint8_t spcEON = GetByte(curOffset++);

    desc << L"EON Status: " << (int)spcEON;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_PARAM: {
    uint8_t spcEDL = GetByte(curOffset++);
    uint8_t spcEFB = GetByte(curOffset++);

    desc << L"Delay: " << (int)spcEDL << L"  Feedback: " << (int)spcEFB;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_FIR: {
    curOffset += 8; // FIR C0-C7
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ADSR: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc 
      << L"AR/DR: " << (int)arg1
      << L"  SR/RR: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_PITCHFADE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Duration: " << (int)arg1
      << L"  From: " << (int)arg2
      << L"  To: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Fade", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
    AddTime(arg1);
    break;
  }

  case EVENT_PANFADE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Duration: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan Fade", desc.str(), CLR_PAN, ICON_CONTROL);
    break;
  }

  case EVENT_TRACKEND: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

  case EVENT_LOOPSTART: {
    loopGroup = GetByte(curOffset++);
    uint8_t theLoopCount = GetByte(curOffset++);
    //if (loopCount[loopGroup] == 0) {
      loopCount[loopGroup] = theLoopCount;
    //}
    desc 
      << L"Group ID: " << (int)loopGroup
      << L"  Loop Count: " << (int)loopCount[loopGroup];
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    break;
  }

  case EVENT_LOOPEND: {
    loopGroup = GetByte(curOffset++);
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    dest += beginOffset; // relative offset to address
    dest += 2;
    desc
      << L"Group ID: " << (int)loopGroup
      << L"  Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);

    if (loopCount[loopGroup] != 0) {
      loopCount[loopGroup]--;
      curOffset = dest;
    }
    break;
  }

  case EVENT_GOTOSUBROUTINE: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    dest += beginOffset; // relative offset to address
    dest += 1;
    desc << "Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    subroutineReturnAddress[subroutineGroup] = curOffset;
    uint32_t length = curOffset - beginOffset;
    subroutineGroup++;
    curOffset = dest;
      AddGenericEvent(beginOffset, length, L"Goto Subroutine", desc.str().c_str(), CLR_LOOP, ICON_STARTREP);
    break;
  }

  case EVENT_SUBROUTINEEND: {
    uint16_t prevSubroutineGroup = subroutineGroup - 1;
    desc << "Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)subroutineReturnAddress[prevSubroutineGroup];
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str().c_str(), CLR_TRACKEND, ICON_ENDREP);
    curOffset = subroutineReturnAddress[prevSubroutineGroup];
    subroutineGroup--;
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    dest += beginOffset; // relative offset to address
    dest += 1;
    desc << "Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    uint32_t length = curOffset - beginOffset;

    curOffset = dest;
    if (!IsOffsetUsed(dest)) {
      AddGenericEvent(beginOffset, length, L"Jump", desc.str().c_str(), CLR_LOOPFOREVER);
    }
    else {
      AddGenericEvent(beginOffset, length, L"Jump", desc.str().c_str(), CLR_LOOPFOREVER);
      bContinue = false;
    }
    break;
  }

  case EVENT_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    //TODO: Fix the pan and volume
    uint8_t finalPanValue = arg1 / arg2;
    uint8_t finalVolValue = arg1 + arg2;
    desc
      << L"Right Volume Parameter: " << (int)arg1
      << L"  Left Volume Parameter: " << (int)arg2;
    AddPanNoItem(finalPanValue);
    AddVolNoItem(finalVolValue);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan & Volume", desc.str(), CLR_EXPRESSION, ICON_CONTROL);
    break;
  }

  case EVENT_VOLENV: {
    uint8_t arg1 = GetByte(curOffset++);
    desc 
      << L"Envelove ID: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume Envelove", desc.str(), CLR_VOLUME, ICON_CONTROL);
    break;
  }

  case EVENT_MASTERVOL: {
    uint8_t arg1 = GetByte(curOffset++);
    AddMasterVol(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_GLOBALTRANSPOSE: {
    uint8_t arg1 = GetByte(curOffset++);
    AddGlobalTranspose(beginOffset, curOffset - beginOffset, arg1);
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

  case EVENT_PITCHENV: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Envelove ID: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Envelove (Attack)", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    break;
  }

  case EVENT_NOISE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Settings", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_NOISENOTE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1
      << L"  Duration: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Note", desc.str(), CLR_NOTEON, ICON_CONTROL);
    AddTime(arg2);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t instrNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, instrNum);
    break;
  }

  case EVENT_NOTE: {
    uint8_t key = GetByte(beginOffset);
    uint8_t dur = GetByte(curOffset++);
    AddNoteByDur(beginOffset, curOffset - beginOffset, key, 0x7f, dur);
    AddTime(dur);
    break;
  }

  case EVENT_REST: {
    uint8_t dur = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, dur);
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"BeatManiacSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

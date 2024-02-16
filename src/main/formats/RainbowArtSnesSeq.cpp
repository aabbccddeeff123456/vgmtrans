#include "pch.h"
#include "RainbowArtSnesSeq.h"

DECLARE_FORMAT(RainbowArtSnes);

//  ****************
//  RainbowArtSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

RainbowArtSnesSeq::RainbowArtSnesSeq(RawFile* file,
  RainbowArtSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(RainbowArtSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
  headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;
  baseAddress = GetShort(0x42);

  AlwaysWriteInitialTempo(60000000.0 / (SEQ_PPQN * (125 * 0x85)));

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

RainbowArtSnesSeq::~RainbowArtSnesSeq(void) {
}

void RainbowArtSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool RainbowArtSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);
  if (dwOffset * MAX_TRACKS > 0x10000) {
    return false;
  }

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    if (GetByte(dwOffset + (trackIndex * 8)) != 0xff) {
      std::wstringstream trackName;
      trackName << L"Track " << (trackIndex + 1);

      VGMHeader* trackHeader = header->AddHeader(dwOffset + (trackIndex * 8), 8, trackName.str().c_str());
      trackHeader->AddSimpleItem(dwOffset + (trackIndex * 8), 1, L"Track Number");
      trackHeader->AddUnknownItem(dwOffset + (trackIndex * 8) + 1, 1);
      trackHeader->AddSimpleItem(dwOffset + (trackIndex * 8) + 2, 2, L"Track Pointer");
      trackHeader->AddSimpleItem(dwOffset + (trackIndex * 8) + 4, 2, L"Instrument Table Pointer");
      instrumentTablePointer[trackIndex] = GetShort(dwOffset + (trackIndex * 8) + 4) + baseAddress;
      trackHeader->AddSimpleItem(dwOffset + (trackIndex * 8) + 6, 2, L"Program Table Pointer");
      programTablePointer[trackIndex] = GetShort(dwOffset + (trackIndex * 8) + 6) + baseAddress;

      sectionTableAddress[trackIndex] = GetShort(dwOffset + (trackIndex * 8) + 2) + baseAddress;
      uint16_t addrTrackStart = GetShort(GetShort(dwOffset + (trackIndex * 8) + 2) + baseAddress) + baseAddress;
      if (addrTrackStart != 0xffff) {
        RainbowArtSnesTrack* track = new RainbowArtSnesTrack(this, addrTrackStart);
        aTracks.push_back(track);
      }

    }
    else {
      trackIndex = 99;
    }
  }

  return true;
}

bool RainbowArtSnesSeq::GetTrackPointers(void) {
  return true;
}

void RainbowArtSnesSeq::LoadEventMap() {
  if (version == RAINBOWARTSNES_NONE) {
    return;
  }
  int statusByte;

  for (statusByte = 0xe0; statusByte <= 0xf0; statusByte++) {
    EventMap[statusByte] = EVENT_PITCH_BEND;
  }
  for (statusByte = 0xd0; statusByte <= 0xe0; statusByte++) {
    EventMap[statusByte] = EVENT_PITCH_BEND_INSTANT;
  }
  for (statusByte = 0x80; statusByte <= 0xbf; statusByte++) {
    EventMap[statusByte] = EVENT_PROGCHANGE;
  }
  for (statusByte = 0x00; statusByte <= 0x7d; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  EventMap[0x7d] = EVENT_TIE;
  EventMap[0x7e] = EVENT_REST;
  EventMap[0x7f] = EVENT_REST;
  EventMap[0xc0] = EVENT_VOLUME;
  EventMap[0xc1] = EVENT_PAN;
  EventMap[0xff] = EVENT_END;

  // TODO: RainbowArtSnesSeq::LoadEventMap
}


//  ******************
//  RainbowArtSnesTrack
//  ******************

RainbowArtSnesTrack::RainbowArtSnesTrack(RainbowArtSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;

  prevDuration = 0;
}

void RainbowArtSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();
}

bool RainbowArtSnesTrack::ReadEvent(void) {
  RainbowArtSnesSeq* parentSeq = (RainbowArtSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  RainbowArtSnesSeqEventType eventType = (RainbowArtSnesSeqEventType)0;
  std::map<uint8_t, RainbowArtSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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
    key = GetByte(beginOffset);
    vel = GetByte(curOffset++);
    prevDuration = GetByte(curOffset++) + 1;
    AddNoteByDur(beginOffset, curOffset - beginOffset, key, vel, prevDuration);
    AddTime(prevDuration);
    break;
  }

  case EVENT_TIE: {
    prevDuration = GetByte(curOffset++) + 1;
    MakePrevDurNoteEnd(GetTime() + prevDuration);
    desc << L"Duration: " << (int)prevDuration;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE, ICON_NOTE);
    AddTime(prevDuration);
    break;
  }

  case EVENT_REST: {
    prevDuration = GetByte(curOffset++) + 1;
    AddRest(beginOffset, curOffset - beginOffset, prevDuration & 0x7f);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t arg1 = GetByte(beginOffset) & 0x3f;
    AddProgramChange(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
    AddPan(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_PITCH_BEND: {
    uint8_t arg1 = (GetByte(beginOffset) & 0x0f) - 0x08;
    uint8_t arg2 = GetByte(curOffset++) | arg1 << 8;
    uint8_t deltaTime = GetByte(curOffset++) + 1;

    AddPitchBend(beginOffset, curOffset - beginOffset, arg2);
    AddTime(deltaTime);
    break;
  }

  case EVENT_PITCH_BEND_INSTANT: {
    uint8_t arg1 = (GetByte(beginOffset) & 0x0f) - 0x08;
    uint8_t arg2 = GetByte(curOffset++) | arg1 << 8;

    AddPitchBend(beginOffset, curOffset - beginOffset, arg2);
    break;
  }

  case EVENT_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"RainbowArtSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

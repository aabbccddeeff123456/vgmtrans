#include "pch.h"
#include "MasumiSnesSeq.h"

// This song engine used zipped music...

DECLARE_FORMAT(MasumiSnes);

template<typename T>
T CPP_ROL(T n, const int bitN)
{
  const int BITLEN = sizeof(T) * 8;
  n = (n >> (BITLEN - bitN)) | (n << bitN);
  return n;
}

//  ****************
//  MasumiSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

MasumiSnesSeq::MasumiSnesSeq(RawFile* file,
  MasumiSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(MasumiSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

MasumiSnesSeq::~MasumiSnesSeq(void) {
}

void MasumiSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool MasumiSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  header->AddSimpleItem(dwOffset, 1, L"Compressed Switch");
  header->AddSimpleItem(dwOffset + 1, 1, L"Number of Tracks");
  uint8_t maxTracksAllow = GetByte(dwOffset + 1);

  for (uint8_t trackIndex = 0; trackIndex < maxTracksAllow; trackIndex++) {
    uint32_t addrTrackLowPtr = dwOffset + 2 + (trackIndex * 2);

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());

    uint16_t addrTrackStart = GetShort(addrTrackLowPtr) + dwOffset + 1;
      MasumiSnesTrack* track = new MasumiSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
  }

  return true;
}

bool MasumiSnesSeq::GetTrackPointers(void) {
  return true;
}

void MasumiSnesSeq::LoadEventMap() {
  if (version == MASUMISNES_NONE) {
    return;
  }

  int statusByte;

  for (statusByte = 0x00; statusByte <= 0xff; statusByte++) {
    EventMap[CPP_ROL(statusByte, 1) == 0x00] = EVENT_UNKNOWN0;
    EventMap[CPP_ROL(statusByte, 1) == 0x01] = EVENT_UNKNOWN0;
    EventMap[CPP_ROL(statusByte, 1) == 0x02] = EVENT_UNKNOWN0;
    EventMap[CPP_ROL(statusByte, 1) == 0x03] = EVENT_UNKNOWN0;
    EventMap[CPP_ROL(statusByte, 1) == 0x04] = EVENT_TEMPO;
    EventMap[CPP_ROL(statusByte, 1) == 0x05] = EVENT_PROGCHANGE;
  }
  // TODO: MasumiSnesSeq::LoadEventMap
}

double MasumiSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa))) * (tempo / 256.0);
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  MasumiSnesTrack
//  ******************

MasumiSnesTrack::MasumiSnesTrack(MasumiSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void MasumiSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();
}

bool MasumiSnesTrack::ReadEvent(void) {
  MasumiSnesSeq* parentSeq = (MasumiSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  MasumiSnesSeqEventType eventType = (MasumiSnesSeqEventType)0;
  std::map<uint8_t, MasumiSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_PROGCHANGE: {
    uint8_t instrNum = (statusByte - 0x60);
    AddProgramChange(beginOffset, curOffset - beginOffset, instrNum);
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
    pRoot->AddLogItem(new LogItem((std::wstring(L"Zipped Music is not supported! Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"MasumiSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

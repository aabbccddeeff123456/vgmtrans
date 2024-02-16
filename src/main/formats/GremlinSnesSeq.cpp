#include "pch.h"
#include "GremlinSnesSeq.h"

DECLARE_FORMAT(GremlinSnes);

//  ****************
//  GremlinSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

GremlinSnesSeq::GremlinSnesSeq(RawFile* file,
  GremlinSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(GremlinSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
  headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

GremlinSnesSeq::~GremlinSnesSeq(void) {
}

void GremlinSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool GremlinSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  uint16_t curOffset = dwOffset + 2;

  header->AddSimpleItem(curOffset, 2, L"Number of Sections");
  nNumSections = GetShort(curOffset) * 2 + 2;

  curOffset += 2;

  header->AddSimpleItem(curOffset, 2, L"Number of Tracks");
  uint8_t maxTracksAllow = GetByte(curOffset);

  curOffset += 2;

  header->AddTempo(curOffset, 2);
  uint16_t tempo = GetShort(curOffset);
  tempoBPM = GetTempoInBPM(GetShort(curOffset));
  AlwaysWriteInitialTempo(tempoBPM);

  curOffset += 2;

  header->AddUnknownItem(curOffset, 2);
  curOffset += 2;
  header->AddUnknownItem(curOffset, 1);
  curOffset++;
  header->AddUnknownItem(curOffset, 1);
  curOffset++;

  for (uint8_t trackIndex = 0; trackIndex < maxTracksAllow; trackIndex++) {

    std::wstringstream trackName;
    trackName << L"Track Playlist Pointer " << (trackIndex + 1);
    header->AddSimpleItem(curOffset, 2, trackName.str());

    uint16_t addrTrackStart = GetShort(curOffset) + dwOffset;
    currentSectionPointer[trackIndex] = curOffset + 2;
    curOffset += nNumSections;
    if (addrTrackStart != 0xffff) {
      GremlinSnesTrack* track = new GremlinSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool GremlinSnesSeq::GetTrackPointers(void) {
  return true;
}

void GremlinSnesSeq::LoadEventMap() {
  if (version == GREMLINSNES_NONE) {
    return;
  }

  EventMap[0x00] = EVENT_REST;
  EventMap[0xe4] = EVENT_PAN;
  EventMap[0xff] = EVENT_END;

  // TODO: GremlinSnesSeq::LoadEventMap
}

double GremlinSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa)) * 2) * (tempo / 256.0);
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  GremlinSnesTrack
//  ******************

GremlinSnesTrack::GremlinSnesTrack(GremlinSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void GremlinSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  currentSection = 0;
}

bool GremlinSnesTrack::ReadEvent(void) {
  GremlinSnesSeq* parentSeq = (GremlinSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  GremlinSnesSeqEventType eventType = (GremlinSnesSeqEventType)0;
  std::map<uint8_t, GremlinSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_PAN: {
    int8_t newPan = GetByte(curOffset++) / 2;
      AddPan(beginOffset, curOffset - beginOffset, newPan);
    break;
  }

  case EVENT_REST: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 <= 0x7f) {
      AddRest(beginOffset, curOffset - beginOffset, arg1);
    }
    else {
      uint8_t arg2 = GetByte(curOffset++);
      if (arg2 & 0x40) {
        if (arg2 & 0x20) {
          curOffset--;  //vcmd
          break;
        }
        else {
          arg1 = arg2 & 0x1f;
          arg1 = arg1 * 6;
          arg1 += 0x07;
          vel = arg1;
          AddGenericEvent(beginOffset, curOffset - beginOffset, L"Velocity", desc.str().c_str(), CLR_NOTEON, ICON_CONTROL);
        }
      }
      else if (arg2 & 0x20) {
        arg1 = arg2 & 0x1f;
        AddTranspose(beginOffset, curOffset - beginOffset, arg1);
      }
      else {

      }
    }
    break;
  }

  case EVENT_END: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Section End", desc.str().c_str(), CLR_TRACKEND, ICON_CONTROL);
    uint16_t nextSectionPtr = parentSeq->currentSectionPointer[channel] + currentSection * 2;
    currentSection++;
    if (nextSectionPtr != 0xffff) {
      curOffset = GetShort(nextSectionPtr) + parentSeq->dwOffset;
    }
    else {
      bContinue = false;
    }
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"GremlinSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

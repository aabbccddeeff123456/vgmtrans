#include "pch.h"
#include "InsomnyaSnesSeq.h"

DECLARE_FORMAT(InsomnyaSnes);

//  ****************
//  InsomnyaSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

InsomnyaSnesSeq::InsomnyaSnesSeq(RawFile *file,
                                   InsomnyaSnesVersion ver,
                                   uint32_t seqdataOffset,
                                   std::wstring newName)
    : VGMSeq(InsomnyaSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  AlwaysWriteInitialTempo(60000000.0 / (SEQ_PPQN * (125 * 0x50)));

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

InsomnyaSnesSeq::~InsomnyaSnesSeq(void) {
}

void InsomnyaSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
  currentTick = 1;
}

bool InsomnyaSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader *header = AddHeader(dwOffset, 0);
  if (dwOffset * MAX_TRACKS > 0x10000) {
    return false;
  }

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint32_t addrTrackLowPtr = dwOffset + (trackIndex * 2);

    std::wstringstream trackName;
    trackName << L"Track Playlist Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());

    channelPlaylistPointer[trackIndex] = GetShort(addrTrackLowPtr);
    uint32_t addrTrackStart = GetShort(channelPlaylistPointer[trackIndex]);

    if (addrTrackStart != 0xffff) {
      InsomnyaSnesTrack *track = new InsomnyaSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool InsomnyaSnesSeq::GetTrackPointers(void) {
  return true;
}

void InsomnyaSnesSeq::LoadEventMap() {
  if (version == INSOMNYASNES_NONE) {
    return;
  }
  int statusByte;
  for (statusByte = 0x00; statusByte <= 0x64; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  for (statusByte = 0x81; statusByte <= 0xff; statusByte++) {
    EventMap[statusByte] = EVENT_DELTA_TIME;
  }

  EventMap[0x65] = EVENT_PROGCHANGE;
  EventMap[0x66] = EVENT_TICK;
  EventMap[0x67] = EVENT_VOLUME;

  EventMap[0x80] = EVENT_PLAYLIST_END;
  // TODO: InsomnyaSnesSeq::LoadEventMap
}


//  ******************
//  InsomnyaSnesTrack
//  ******************

InsomnyaSnesTrack::InsomnyaSnesTrack(InsomnyaSnesSeq *parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;

  currentPlaylist[0] = 0;
  currentPlaylist[1] = 0;
  currentPlaylist[2] = 0;
  currentPlaylist[3] = 0;
  currentPlaylist[4] = 0;
  currentPlaylist[5] = 0;
  currentPlaylist[6] = 0;
  currentPlaylist[7] = 0;
}

void InsomnyaSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();
}

bool InsomnyaSnesTrack::ReadEvent(void) {
  InsomnyaSnesSeq *parentSeq = (InsomnyaSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  InsomnyaSnesSeqEventType eventType = (InsomnyaSnesSeqEventType) 0;
  std::map<uint8_t, InsomnyaSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
  if (pEventType != parentSeq->EventMap.end()) {
    eventType = pEventType->second;
  }

  switch (eventType) {
    case EVENT_UNKNOWN0:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;

    case EVENT_UNKNOWN1: {
      uint8_t arg1 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int) arg1;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN2: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int) arg1
          << L"  Arg2: " << (int) arg2;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN3: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int) arg1
          << L"  Arg2: " << (int) arg2
          << L"  Arg3: " << (int) arg3;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_UNKNOWN4: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      uint8_t arg4 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int) arg1
          << L"  Arg2: " << (int) arg2
          << L"  Arg3: " << (int) arg3
          << L"  Arg4: " << (int) arg4;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      break;
    }

    case EVENT_PROGCHANGE: {
      uint8_t arg1 = GetByte(curOffset++);
      AddProgramChange(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_TICK: {
      parentSeq->currentTick = GetByte(curOffset++);
      desc
        << L"Tick per note: " << (int)parentSeq->currentTick;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tick", desc.str(), CLR_TEMPO);
      break;
    }

    case EVENT_VOLUME: {
      uint8_t arg1 = GetByte(curOffset++);
      AddVol(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_PLAYLIST_END: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Section End", desc.str(), CLR_TRACKEND);
      currentPlaylist[channel]++;
      uint16_t nextSectionPointer = GetShort(parentSeq->channelPlaylistPointer[channel] + (currentPlaylist[channel] * 2));
      if (nextSectionPointer != 0x0000) {
        curOffset = nextSectionPointer;
      }
      else {
        bContinue = false;
      }
      break;
    }

    case EVENT_NOTE: {
      //AddNoteOffNoItem(prevKey);
      //MakePrevDurNoteEnd();
      key = GetByte(beginOffset);
      uint8_t durationBeforeNextNote = 0;
      uint16_t currentOffset = curOffset;
      while (GetByte(currentOffset) > 0x64) {
        if (GetByte(currentOffset) <= 0x80) {
          currentOffset++;
        }
        if (GetByte(currentOffset) >= 0x81) {
          durationBeforeNextNote += (GetByte(currentOffset) & 0x7f) * parentSeq->currentTick;
        }
        currentOffset++;
      }
      AddNoteByDur(beginOffset, curOffset - beginOffset, key, 0x7f, durationBeforeNextNote - 1);
      break;
    }

    case EVENT_DELTA_TIME: {
      uint8_t deltaTime = (GetByte(beginOffset) & 0x7f) * parentSeq->currentTick;
      AddRest(beginOffset, curOffset - beginOffset, deltaTime, L"Delta Time");
      break;
    }

    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
                                    LOG_LEVEL_ERR,
                                    L"InsomnyaSnesSeq"));
      bContinue = false;
      break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

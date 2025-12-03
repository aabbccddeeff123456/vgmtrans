#include "pch.h"
#include "DaviddeSnesSeq.h"

DECLARE_FORMAT(DaviddeSnes);

//  ****************
//  DaviddeSnesSeq
//  ****************
#define MAX_TRACKS 8
#define SEQ_PPQN 48

DaviddeSnesSeq::DaviddeSnesSeq(RawFile* file, DaviddeSnesVersion ver, uint32_t seqdataOffset,
                               std::wstring newName)
    : VGMSeq(DaviddeSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
      headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

DaviddeSnesSeq::~DaviddeSnesSeq(void) {
}

void DaviddeSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool DaviddeSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  uint16_t curOffset = dwOffset;

  for (uint8_t trackIndex = 0; trackIndex < 8; trackIndex++) {
    if (GetShort(curOffset) != 0x0000) {
      std::wstringstream trackName;
      trackName << L"Track Playlist Pointer " << (trackIndex + 1);
      header->AddSimpleItem(curOffset, 2, trackName.str());

      uint16_t addrTrackStart = GetShort(GetShort(curOffset));
      currentSectionPointer[trackIndex] = GetShort(curOffset);
      curOffset += 2;
      if (addrTrackStart != 0xffff) {
        DaviddeSnesTrack* track = new DaviddeSnesTrack(this, addrTrackStart);
        aTracks.push_back(track);
      }
    } else {
      header->AddSimpleItem(curOffset, 2, L"Header End");
      trackIndex = 9;
    }
  }
  return true;
}

bool DaviddeSnesSeq::GetTrackPointers(void) {
  return true;
}

void DaviddeSnesSeq::LoadEventMap() {
  if (version == DAVIDDESNES_NONE) {
    return;
  }
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  // TODO: DaviddeSnesSeq::LoadEventMap
}

double DaviddeSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa)) * 2) * (tempo / 256.0);
  } else {
    return 1.0;  // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  DaviddeSnesTrack
//  ******************

DaviddeSnesTrack::DaviddeSnesTrack(DaviddeSnesSeq* parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void DaviddeSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  currentSection = 0;
  restIsOn = false;
  delta = 0;
  noteNumber = 0;
}

bool DaviddeSnesTrack::ReadEvent(void) {
  DaviddeSnesSeq* parentSeq = (DaviddeSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  // Running Status

  bool bContinue = true;

  std::wstringstream desc;

    DaviddeSnesSeqEventType eventType = (DaviddeSnesSeqEventType)0;
  std::map<uint8_t, DaviddeSnesSeqEventType>::iterator pEventType =
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

        case EVENT_NOTE: {
          bool doNotAddEvent = false;
          uint8_t arg = GetByte(beginOffset);
          if (arg != 0x00) {
            vel = arg;
            uint8_t program = GetByte(curOffset++);
            if (program >= 0x7f) {
              AddProgramChangeNoItem(program, false);
              noteNumber = GetByte(curOffset++);
            } else {
              noteNumber = program;
            }
          }
          uint8_t duration = GetByte(curOffset++);
          if (duration == 0xff) {
            // next section
            uint16_t nextSectionPtr =
                parentSeq->currentSectionPointer[channel] + currentSection * 2;
            currentSection++;
            if (nextSectionPtr != 0xffff) {
              curOffset = GetShort(nextSectionPtr);
            } else {
              bContinue = false;
            }
            doNotAddEvent = true;
          } else if (duration == 0xfe) {
              uint16_t duration16Bit = GetShort(curOffset++) - 1;
              curOffset++;
              dur = duration16Bit;
             if (dur == 0xfffe){
             
              bContinue =
                  AddEndOfTrack(beginOffset, curOffset - beginOffset);
              doNotAddEvent = true;
            }
          } else if (duration == 0x00) {
            // ignore
            doNotAddEvent = true;
          } else {
            dur = duration - 1;
          }
          if (!doNotAddEvent) {
            if (arg == 0x00) {
              AddRest(beginOffset, curOffset - beginOffset, dur);
            } else {
              AddNoteByDur(beginOffset, curOffset - beginOffset, noteNumber, vel, dur);
            }
          }
          break;
        }

        default:
          desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
               << (int)statusByte;
          AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
          pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
                                        LOG_LEVEL_ERR, L"DaviddeSnesSeq"));
          bContinue = false;
          break;
      }
      // if (GetByte(curOffset + 1) <= 0x7f) {
      //   uint8_t len = GetByte(curOffset++);
      //   AddTime(len);
      // }

  // std::wostringstream ssTrace;
  // ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase <<
  // beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) <<
  // curOffset << std::endl; OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

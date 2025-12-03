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
  restIsOn = false;
  delta = 0;
}

bool GremlinSnesTrack::ReadEvent(void) {
  GremlinSnesSeq* parentSeq = (GremlinSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  // Running Status

  bool bContinue = true;

  std::wstringstream desc;

  if (restIsOn == true) {
    restIsOn = false;
  } else {
  if (statusByte <= 0xdf && statusByte >= 0xc0) {
    uint8_t newVol = (statusByte * 0x08 + 7);
    AddVol(beginOffset, curOffset - beginOffset, newVol);
  } else if (statusByte <= 0x9f && statusByte >= 0x80) {
    uint8_t progNumber = statusByte & 0x0f;
    uint8_t len = GetByte(curOffset++);
    AddPercNoteByDur(beginOffset, curOffset - beginOffset, progNumber, 0x7f, len);
    AddTime(len);
  } else if (statusByte <= 0xbf && statusByte >= 0xa0) {
    uint8_t progNumber = statusByte & 0x0f;
    AddProgramChange(beginOffset, curOffset - beginOffset, progNumber);
  } else if (statusByte <= 0x7f && statusByte != 0x00) {
    uint8_t progNumber = statusByte -1;
    uint8_t len = GetByte(curOffset++);
    AddNoteByDur(beginOffset, curOffset - beginOffset, progNumber, 0x7f, len);
    AddTime(len);
  } else if (statusByte == 0x00) {
    uint8_t len = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, len);
  }
  else {
    GremlinSnesSeqEventType eventType = (GremlinSnesSeqEventType)0;
    std::map<uint8_t, GremlinSnesSeqEventType>::iterator pEventType =
        parentSeq->EventMap.find(statusByte);
    if (pEventType != parentSeq->EventMap.end()) {
      eventType = pEventType->second;
    }

    switch (statusByte) {
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

case 0xe0: {
        int8_t newVol = GetByte(curOffset++);
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddVol(beginOffset, curOffset - beginOffset, newVol);
        break;
      }

         case 0xe1: {
        int8_t newTempo = GetByte(curOffset++);
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddTempo(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
        break;
      }

                  case 0xe2: {
        uint8_t newTempo = GetByte(curOffset++);
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddPortamentoTime(beginOffset, curOffset - beginOffset, newTempo, L"Portamento");
        break;
      }
       case 0xe3: {
        uint8_t arg1 = GetByte(curOffset++);
        uint8_t arg2 = GetByte(curOffset++);
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        desc << L"Arg1: " << (int)arg1 << L"  Arg2: " << (int)arg2;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Slide?", desc.str().c_str(),
                        CLR_PITCHBEND, ICON_CONTROL);
        break;
       }

      case 0xe4: {
        int8_t newPan = GetByte(curOffset++) / 2;
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddPan(beginOffset, curOffset - beginOffset, newPan);
        break;
      }

      case 0xe5: {
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
        break;
      }

      case 0xe6: {
        uint8_t arg1 = GetByte(curOffset++);
        uint8_t arg2 = GetByte(curOffset++);
        uint8_t arg3 = GetByte(curOffset++);
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        desc << L"Arg1: "
             << (int)arg1 << L"  Arg2: " << (int)arg2 << L"  Arg3: " << (int)arg3;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(),
                        CLR_REVERB, ICON_CONTROL);
        break;
      }

      case 0xe7: {
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddUnknown(beginOffset, curOffset - beginOffset, L"Conditional Key-Off", desc.str());
        break;
      }

      case 0xea:
      case 0xeb:
      case 0xec:
      case 0xed:
      case 0xee:
      case 0xef:
      case 0xf0:
      case 0xf1:
      case 0xf2:
      case 0xf3:
      case 0xf4:
      case 0xf5:
      case 0xf6:
      case 0xf7:
      case 0xf8:
      case 0xf9:
      case 0xfa:
      case 0xfb:
      case 0xfc:
      case 0xfd:
      case 0xfe:
      {
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP", desc.str().c_str(),
                        CLR_MISC, ICON_CONTROL);
        break;
      }

      case 0xe8: {
        uint8_t arg1 = GetByte(curOffset++);
        //if (GetByte(curOffset) <= 0x7f) {
        //  uint8_t len = GetByte(curOffset++);
        //  AddTime(len);
        //}
        desc << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Release Type", desc.str().c_str(), CLR_MISC,
                        ICON_CONTROL);
        break;
      }

      case 0xe9: {
        int8_t newPan = GetByte(curOffset++) / 2;
        if (GetByte(curOffset) <= 0x7f) {
          uint8_t len = GetByte(curOffset++);
          AddTime(len);
        }
        AddPan(beginOffset, curOffset - beginOffset, newPan, L"Conditional Pan?");
        
        break;
      }

      case EVENT_REST: {
        uint8_t arg1 = GetByte(curOffset++);
        if (arg1 <= 0x7f) {
          AddRest(beginOffset, curOffset - beginOffset, arg1);
        } else {
          uint8_t arg2 = GetByte(curOffset++);
          if (arg2 & 0x40) {
            if (arg2 & 0x20) {
              curOffset--;  // vcmd
              break;
            } else {
              arg1 = arg2 & 0x1f;
              arg1 = arg1 * 6;
              arg1 += 0x07;
              vel = arg1;
              AddGenericEvent(beginOffset, curOffset - beginOffset, L"Velocity", desc.str().c_str(),
                              CLR_NOTEON, ICON_CONTROL);
            }
          } else if (arg2 & 0x20) {
            arg1 = arg2 & 0x1f;
            AddTranspose(beginOffset, curOffset - beginOffset, arg1);
          } else {
          }
        }
        break;
      }

      case 0xff: {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Section End", desc.str().c_str(),
                        CLR_TRACKEND, ICON_CONTROL);
        uint16_t nextSectionPtr = parentSeq->currentSectionPointer[channel] + currentSection * 2;
        currentSection++;
        if (nextSectionPtr != 0xffff) {
          curOffset = GetShort(nextSectionPtr) + parentSeq->dwOffset;
        } else {
          bContinue = false;
        }
        break;
      }

      default:
        desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
             << (int)statusByte;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
        pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
                                      LOG_LEVEL_ERR, L"GremlinSnesSeq"));
        bContinue = false;
        break;
    }
    //if (GetByte(curOffset + 1) <= 0x7f) {
    //  uint8_t len = GetByte(curOffset++);
    //  AddTime(len);
   // }
  }
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

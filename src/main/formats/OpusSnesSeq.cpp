// VGMTrans Opus SPC Engine
// 2021 COMNERK
// Unfinished!
// Noteand Play Song doesn't work for now.

#include "pch.h"
#include "OpusSnesSeq.h"

DECLARE_FORMAT(OpusSnes);

//  ****************
//  OpusSnesSeq
//  ****************
#define MAX_TRACKS  16    // 0x0f
#define SEQ_PPQN    48

OpusSnesSeq::OpusSnesSeq(RawFile* file,
  OpusSnesVersion ver,
  uint32_t seqdataOffset,
  uint32_t seqpointerdata_offset,
  uint8_t songIndex,
  std::wstring newName)
  : VGMSeq(OpusSnesFormat::name, file, seqdataOffset, 0, newName), version(ver), songPointer(seqpointerdata_offset), currentSong(songIndex){
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();


}

OpusSnesSeq::~OpusSnesSeq(void) {
}

void OpusSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool OpusSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  //VGMHeader* header = AddHeader(dwOffset, 0);
  uint16_t curOffset = dwOffset;

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {

    uint16_t trackSize = GetShort(curOffset);
    uint8_t trackSizeHI = GetByte(curOffset + 1);

    std::wstringstream trackName;
    trackName << L"Track Size " << (trackIndex + 1);
    //header->AddSimpleItem(curOffset, 2, trackName.str());

    if (trackSizeHI != 0xff) {
      curOffset++;
      curOffset++;
      OpusSnesTrack* track = new OpusSnesTrack(this, curOffset, trackSize);
      aTracks.push_back(track);
      curOffset += trackSize;
    }
    else {
      trackIndex = 0x23;
    }

  }

  return true;
}

bool OpusSnesSeq::GetTrackPointers(void) {
  return true;
}

void OpusSnesSeq::LoadEventMap() {
  if (version == OPUSSNES_NONE) {
    return;
  }
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x1f; statusByte++) {
    EventMap[statusByte] = EVENT_PLAY_RELATIVE;
  }
  for (statusByte = 0x20; statusByte <= 0x3f; statusByte++) {
    EventMap[statusByte] = EVENT_STOP;
  }
  for (statusByte = 0x40; statusByte <= 0x5f; statusByte++) {
    EventMap[statusByte] = EVENT_PAUSE;
  }
  for (statusByte = 0x60; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_STOP_ABSOLUTE;
  }
  for (statusByte = 0x80; statusByte <= 0x9f; statusByte++) {
    EventMap[statusByte] = EVENT_STOP_PAUSE;
  }
  for (statusByte = 0xa0; statusByte <= 0xbf; statusByte++) {
    EventMap[statusByte] = EVENT_BEND;
  }
  for (statusByte = 0xd8; statusByte <= 0xdf; statusByte++) {
    EventMap[statusByte] = EVENT_VOLUME_RANGE;
  }
  for (statusByte = 0xe0; statusByte <= 0xff; statusByte++) {
    EventMap[statusByte] = EVENT_BEND_PAUSE;
  }

  EventMap[0xc0] = EVENT_VIBRATO;
  EventMap[0xc1] = EVENT_SONG;
  EventMap[0xc2] = EVENT_VOLUME;
  EventMap[0xc3] = EVENT_PAN;
  EventMap[0xc4] = EVENT_ECHO;
  EventMap[0xc5] = EVENT_PRIORITY;
  EventMap[0xc6] = EVENT_VCMDC1FLAG;
  EventMap[0xc7] = EVENT_VCMDC1FLAG;
  EventMap[0xc8] = EVENT_RESET_STATE;
  EventMap[0xc9] = EVENT_LEGATO;
  EventMap[0xca] = EVENT_PROGCHANGE;
  EventMap[0xcb] = EVENT_TEMPO;
  EventMap[0xcc] = EVENT_NOP;
  EventMap[0xcd] = EVENT_HALT;
  EventMap[0xce] = EVENT_HALT;
  EventMap[0xcf] = EVENT_TRACK;
  EventMap[0xd0] = EVENT_PLAY_VCMD;
  EventMap[0xd1] = EVENT_STOP_VCMD;
  EventMap[0xd2] = EVENT_NOP;
  EventMap[0xd3] = EVENT_SET_NOTE;
  EventMap[0xd4] = EVENT_BEND_VCMD;
  EventMap[0xd5] = EVENT_NOP;
  EventMap[0xd6] = EVENT_FINE_TUNING;
  EventMap[0xd7] = EVENT_PAUSE_255;

  // TODO: OpusSnesSeq::LoadEventMap
}

double OpusSnesSeq::GetTempoInBPM() {
  return GetTempoInBPM(tempo);
}

double OpusSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa)) * 2) * (tempo / 256.0);   // 0xfb?
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  OpusSnesTrack
//  ******************

OpusSnesTrack::OpusSnesTrack(OpusSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void OpusSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  dur = 0;
  key = 0;
  vel = 0;
  prevDur = 0;
  vcmdC1_Flag = 0;
  preservedCmd = 0;
}

bool OpusSnesTrack::ReadEvent(void) {
  OpusSnesSeq* parentSeq = (OpusSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  if (preservedCmd != 0) {
    if (preservedCmd >= 0xa0) {
      uint8_t arg1 = preservedCmd & 0x1f;
      uint8_t bend = 0;
      if (arg1 <= 0x0f) {
        bend += arg1;
      }
      else {
        arg1 -= 0x10;
        bend -= arg1;
      }
      AddPitchBend(beginOffset, 0, bend);
    }
        if(preservedCmd >= 0x20) {
          uint8_t arg1 = preservedCmd & 0x1f;
          if (arg1 <= 0x0f) {
            key += arg1;
          }
          else {
            arg1 -= 0x10;
            key -= arg1;
          }
          AddNoteOffNoItem(arg1);
        }

        preservedCmd = 0;
        curOffset--;
  }
  else {

    OpusSnesSeqEventType eventType = (OpusSnesSeqEventType)0;
    std::map<uint8_t, OpusSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

    case EVENT_VCMDC1FLAG: {
      vcmdC1_Flag = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)vcmdC1_Flag;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Set Flag for use in 0xC1", desc.str());
      break;
    }

    case EVENT_PRIORITY: {
      uint8_t newPriority = GetByte(curOffset++);
      desc << L"Priority: " << (int)newPriority;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Priority", desc.str().c_str(), CLR_PRIORITY);
      break;
    }

    case EVENT_VOLUME: {
      uint8_t arg1 = GetByte(curOffset++);
      AddVol(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_PAN: {
      uint8_t arg1 = GetByte(curOffset++);
      AddPan(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_PROGCHANGE: {
      uint8_t arg1 = GetByte(curOffset++);
      AddProgramChange(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_RESET_STATE: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Reset State", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_HALT: {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset, L"Halt");
      break;
    }

    case EVENT_VIBRATO: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION, ICON_CONTROL);
      break;
    }

    case EVENT_LEGATO: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Legato", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_SONG: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)arg1;
      if (vcmdC1_Flag == 0x04) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", desc.str(), CLR_REVERB, ICON_CONTROL);
      }
      else if (vcmdC1_Flag == 0x00) {
        AddUnknown(beginOffset, curOffset - beginOffset);
      }
      else if (vcmdC1_Flag == 0x01) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume L", desc.str(), CLR_REVERB, ICON_CONTROL);
      }
      else if (vcmdC1_Flag == 0x02) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume R", desc.str(), CLR_REVERB, ICON_CONTROL);
      }
      else if (vcmdC1_Flag == 0x03) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str(), CLR_REVERB, ICON_CONTROL);
      }
      else if (vcmdC1_Flag == 0x05) {
        AddUnknown(beginOffset, curOffset - beginOffset);
      }
      else if (vcmdC1_Flag == 0x06) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Jump to Song", desc.str(), CLR_MISC, ICON_TRACK);
      }
      else if (vcmdC1_Flag == 0x07) {
        AddUnknown(beginOffset, curOffset - beginOffset);
      }
      //if (arg1 == parentSeq->currentSong) {
      //  bContinue = AddLoopForever(beginOffset, curOffset - beginOffset, L"Loop Current Song");
      //}
      //else {
      //}
      break;
    }

    case EVENT_ECHO: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo", desc.str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_TRACK: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Track: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Track", desc.str(), CLR_MISC, ICON_TRACK);
      break;
    }

    case EVENT_TEMPO: {
      uint8_t arg1 = GetByte(curOffset++);
      AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(arg1));
      break;
    }

    case EVENT_NOP: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_PLAY_VCMD: {
      key = GetByte(curOffset++);
      AddNoteOn(beginOffset, curOffset - beginOffset, key, vel);
      break;
    }

    case EVENT_FINE_TUNING: {
      uint8_t newTuning = GetByte(curOffset++);
      AddFineTuning(beginOffset, curOffset - beginOffset, (newTuning / 256.0) * 100.0);
      break;
    }

    case EVENT_SET_NOTE: {
      key = GetByte(curOffset++);
      desc
        << L"Key: " << MidiEvent::GetNoteName(key);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Note Base", desc.str(), CLR_DURNOTE, ICON_CONTROL);
      preservedCmd = 0x20;
      break;
    }

    case EVENT_PAUSE: {
      dur = (GetByte(beginOffset) & 0x1f) + 1;
      desc
        << L"Duration: " << (int)dur;
      AddRest(beginOffset, curOffset - beginOffset, dur);
      break;
    }

    case EVENT_VOLUME_RANGE: {
      uint8_t range = GetByte(beginOffset) & 0x03;
      desc
        << L"Range: " << (int)range;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume Range", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_BEND_VCMD: {
      uint8_t arg1 = GetByte(curOffset++);
      AddPitchBend(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_PAUSE_255: {
      AddRest(beginOffset, curOffset - beginOffset, 0xff, L"Wait");
      break;
    }

    case EVENT_STOP_VCMD: {
      uint8_t arg1 = GetByte(curOffset++);
      AddNoteOff(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_STOP: {
      uint8_t arg1 = GetByte(beginOffset);
      if (arg1 <= 0x0f) {
        key += arg1;
      }
      else {
        arg1 -= 0x10;
        key -= arg1;
      }
      AddNoteOff(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_BEND_PAUSE: {
      uint8_t arg1 = GetByte(beginOffset) & 0x1f;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Bend (Pause)", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
      AddTime(prevDur);
      preservedCmd = arg1 + 0xa0;
      break;
    }

    case EVENT_BEND: {
      uint8_t arg1 = GetByte(beginOffset) & 0x1f;
      uint8_t bend = 0;
      if (arg1 <= 0x0f) {
        bend += arg1;
      }
      else {
        arg1 -= 0x10;
        bend -= arg1;
      }
      AddPitchBend(beginOffset, curOffset - beginOffset, bend);
      break;
    }

    case EVENT_STOP_PAUSE: {
      uint8_t arg1 = GetByte(beginOffset) & 0x1f;
      AddTime(prevDur);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Stop (Pause)", desc.str(), CLR_REST, ICON_CONTROL);
      preservedCmd = arg1 + 0x20;
      break;
    }

    case EVENT_STOP_ABSOLUTE: {
      uint8_t arg1 = (GetByte(beginOffset) & 0x1f) + 1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Stop (Absolute)", desc.str(), CLR_REST, ICON_CONTROL);
      AddTime(arg1);
      preservedCmd = 0x20;
      break;
    }

    case EVENT_PLAY_RELATIVE: {
      uint8_t arg1 = GetByte(beginOffset);
      if (arg1 >= 0x0f) {
        arg1 -= 0x10;
        key -= arg1;
      }
      else {
        key += arg1;
      }
      AddNoteOn(beginOffset, curOffset - beginOffset, key, vel);
      break;
    }

    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
        LOG_LEVEL_ERR,
        L"OpusSnesSeq"));
      bContinue = false;
      break;
    }
  }
  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

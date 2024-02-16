#include "pch.h"
#include "DougenSnesSeq.h"

DECLARE_FORMAT(DougenSnes);

//  ****************
//  DougenSnesSeq
//  ****************
#define MAX_TRACKS  6
#define SEQ_PPQN    48

DougenSnesSeq::DougenSnesSeq(RawFile* file,
  DougenSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(DougenSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

DougenSnesSeq::~DougenSnesSeq(void) {
}

void DougenSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool DougenSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  header->AddTempo(dwOffset, 1);

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {

    uint16_t addrTrackPointer = dwOffset + 1 + trackIndex * 2;

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackPointer, 2, trackName.str());

    uint16_t addrTrackAddLO = GetByte(addrTrackPointer);
    uint16_t addrTrackAddHI = GetByte(addrTrackPointer + 1);
    uint8_t addrTrackPointerLO = dwOffset;    // data loss
    uint16_t addrTrackPointerHI = dwOffset / 0x100;
    uint16_t realTrackPointerLO = addrTrackPointerLO + addrTrackAddLO;
    uint16_t realTrackPointerHI = addrTrackPointerHI + addrTrackAddHI;
    uint16_t addrTrackStart = realTrackPointerLO | realTrackPointerHI << 8;

      DougenSnesTrack* track = new DougenSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
  }

  return true;
}

bool DougenSnesSeq::GetTrackPointers(void) {
  return true;
}

void DougenSnesSeq::LoadEventMap() {
  if (version == DOUGENSNES_NONE) {
    return;
  }

  int statusByte;

  for (statusByte = 0x00; statusByte <= 0xf4; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  EventMap[0xf5] = EVENT_ECHO_VOLUME_LR;
  EventMap[0xf6] = EVENT_UNKNOWN1;
  EventMap[0xf7] = EVENT_UNKNOWN1;    //Does nothing(at least in Der Langrisser)
  EventMap[0xf8] = EVENT_LOOP_END;
  EventMap[0xf9] = EVENT_LOOP_START;
  EventMap[0xfa] = EVENT_PAN;
  EventMap[0xfb] = EVENT_TEMPO;
  EventMap[0xfc] = EVENT_PROGCHANGE;
  EventMap[0xfd] = EVENT_TUNING;
  EventMap[0xfe] = EVENT_END;
  // TODO: DougenSnesSeq::LoadEventMap
}

double DougenSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * tempo));
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

//  ******************
//  DougenSnesTrack
//  ******************

DougenSnesTrack::DougenSnesTrack(DougenSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void DougenSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  tieStatus = false;
  percussion = false;
  noteDeltaTime = 0;
  loopGroup = 0;
  loopCount[loopGroup] = 0;
}

bool DougenSnesTrack::ReadEvent(void) {
  DougenSnesSeq* parentSeq = (DougenSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  DougenSnesSeqEventType eventType = (DougenSnesSeqEventType)0;
  std::map<uint8_t, DougenSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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
    uint8_t currentNote = GetByte(beginOffset);
    if (currentNote == 0 || currentNote == 0x80) {
      if (currentNote == 0) {
        dur = GetByte(curOffset++);
      }
      AddRest(beginOffset, curOffset - beginOffset, dur);
    }
    else if (currentNote >= 0x81) {
      // Tie?
      key = currentNote & 0x7f;
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, key, vel, dur);
        AddTime(noteDeltaTime);
    }
    else {
      noteDeltaTime = GetByte(curOffset++);
      dur = GetByte(curOffset++) - 1;
      vel = GetByte(curOffset++);
      key = currentNote & 0x7f;
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, key, vel, dur);
        AddTime(noteDeltaTime);
    }
    break;
  }

  case EVENT_ECHO_VOLUME_LR: {
    uint8_t volL = GetByte(curOffset++);
    desc << L"Volume: " << (int)volL;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Echo Volume L / R",
      desc.str().c_str(),
      CLR_REVERB,
      ICON_CONTROL);
    break;
  }

  case EVENT_PAN: {
    uint8_t newPan = GetByte(curOffset++);
    AddPan(beginOffset, curOffset - beginOffset, newPan);
    break;
  }

  case EVENT_TUNING: {
    uint8_t newTuning = GetByte(curOffset++);

    AddFineTuning(beginOffset, curOffset - beginOffset, newTuning);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t newProg = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProg);
    if (newProg == 0x7f) {
      percussion = true;
    }
    else {
      percussion = false;
    }
    break;
  }

  case EVENT_LOOP_START: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    loopReturnAddress[loopGroup] = curOffset;
    loopGroup++;
    break;
  }

  case EVENT_LOOP_END: {
    uint8_t looptimes = GetByte(curOffset++);
    uint8_t prevLoopLevel = (loopGroup != 0 ? loopGroup : DOUGENSNES_LOOP_LEVEL_MAX) - 1;
    if (loopCount[prevLoopLevel] == 0) {
      loopCount[prevLoopLevel] = looptimes;
    }
    desc
      << L"Loop Count: " << (int)looptimes;
    if (looptimes == 0) {
      bContinue = AddLoopForever(beginOffset, curOffset - beginOffset);
    }
    else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
      loopCount[prevLoopLevel]--;
      if (loopCount[prevLoopLevel] != 0) {
        curOffset = loopReturnAddress[prevLoopLevel];
      }
      else {
       loopGroup--;
      }
    }
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    break;
  }

  case EVENT_END: {
    AddEndOfTrack(beginOffset, curOffset - beginOffset);
    bContinue = false;
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"DougenSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

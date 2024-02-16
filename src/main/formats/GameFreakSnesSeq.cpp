#include "pch.h"
#include "GameFreakSnesSeq.h"
#include "ScaleConversion.h"

DECLARE_FORMAT(GameFreakSnes);

//  ****************
//  GameFreakSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

GameFreakSnesSeq::GameFreakSnesSeq(RawFile* file,
  GameFreakSnesVersion ver,
  uint32_t seqdataOffset,
  uint16_t instrumentPointer,
  std::wstring newName)
  : VGMSeq(GameFreakSnesFormat::name, file, seqdataOffset, 0, newName), version(ver), instrumentPointer(instrumentPointer) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

GameFreakSnesSeq::~GameFreakSnesSeq(void) {
}

void GameFreakSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool GameFreakSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  header->AddUnknownItem(dwOffset, 1);
  header->AddSimpleItem(dwOffset + 1, 1, L"Enable Tracks");
  uint8_t targetChannels = GetByte(dwOffset + 1);
  uint8_t maxTracks = 0;
  for (uint8_t trackIndex = 0; trackIndex < 8; trackIndex++) {
    if ((targetChannels & (0x80 >> trackIndex)) != 0) {
      maxTracks++;
    }
  }
  for (uint8_t trackIndex = 0; trackIndex < maxTracks; trackIndex++) {
    uint32_t addrTrackLowPtr = dwOffset + 2 + (trackIndex * 2);

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(dwOffset + 2 + (trackIndex * 2), 2, trackName.str());

    uint16_t addrTrackStart = GetShort(addrTrackLowPtr);
    if (addrTrackStart != 0xffff) {
      GameFreakSnesTrack* track = new GameFreakSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool GameFreakSnesSeq::GetTrackPointers(void) {
  return true;
}

void GameFreakSnesSeq::LoadEventMap() {
  if (version == GAMEFREAKSNES_NONE) {
    return;
  }
  for (unsigned int statusByte = 0x00; statusByte <= 0x60; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE_PARAM;
  }
  for (unsigned int statusByte = 0x61; statusByte <= 0xde; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  EventMap[0xdf] = EVENT_REST;
  EventMap[0xe0] = EVENT_PROGCHANGE;
  EventMap[0xe1] = EVENT_VOLUME;
  EventMap[0xe2] = EVENT_VOLUME_REL;
  EventMap[0xe3] = EVENT_NOTE_LENGTH;
  EventMap[0xe4] = EVENT_ADSR_FROM_TABLE;
  EventMap[0xe5] = EVENT_TEMPO;
  EventMap[0xe6] = EVENT_VOLUME_REL_FOREVER;
  EventMap[0xe7] = EVENT_TRANSPOSE;
  EventMap[0xe8] = EVENT_UNKNOWN1;
  EventMap[0xe9] = EVENT_TIE_STATE;
  EventMap[0xea] = EVENT_VIBRATO;
  EventMap[0xeb] = EVENT_UNKNOWN0;
  EventMap[0xec] = EVENT_VIBRATO2;
  EventMap[0xed] = EVENT_VIBRATO_OFF;
  EventMap[0xee] = EVENT_ECHO_PARAM;
  EventMap[0xef] = EVENT_ECHO_OFF;
  EventMap[0xf0] = EVENT_ECHO_FIR;
  EventMap[0xf1] = EVENT_NOISE;
  EventMap[0xf2] = EVENT_NOISE_OFF;
  EventMap[0xf3] = EVENT_LOOP_UNITL;
  EventMap[0xf4] = EVENT_JUMP;
  EventMap[0xf5] = EVENT_CALL;
  EventMap[0xf6] = EVENT_PLAY_SONG;
  EventMap[0xf7] = EVENT_GLOBAL_TRANSPOSE;
  EventMap[0xff] = EVENT_END;
  // TODO: GameFreakSnesSeq::LoadEventMap
}

double GameFreakSnesSeq::GetTempoInBPM() {
  return GetTempoInBPM(tempo);
}

double GameFreakSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * 0x40) * 2) * (256.0 / tempo);
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  GameFreakSnesTrack
//  ******************

GameFreakSnesTrack::GameFreakSnesTrack(GameFreakSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void GameFreakSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  tieState = false;
  loopCount = 0;
  subroutineReturnAddress = 0;
  previousKey = 9898;
  volumerelativeforever = 0;
}

bool GameFreakSnesTrack::ReadEvent(void) {
  GameFreakSnesSeq* parentSeq = (GameFreakSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  GameFreakSnesSeqEventType eventType = (GameFreakSnesSeqEventType)0;
  std::map<uint8_t, GameFreakSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_GLOBAL_TRANSPOSE: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Global Transpose", desc.str(), CLR_TRANSPOSE, ICON_CONTROL);
    break;
  }

  case EVENT_NOISE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Noise Freq.: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_NOISE_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Off", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_PLAY_SONG: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Restart Song", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_FIR: {
    curOffset += 8; // FIR C0-C7
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Off", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO2: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1
      << L"  Arg2: " << (int)arg2
      << L"  Arg3: " << (int)arg3
      << L"  Arg4: " << (int)arg4;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_CALL: {
    uint16_t arg1 = GetShort(curOffset++);
    curOffset++;
    desc
      << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Call Subroutine", desc.str().c_str(), CLR_LOOP, ICON_STARTREP);
    subroutineReturnAddress = curOffset;
    curOffset = arg1;
    break;
  }

  case EVENT_TIE_STATE: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie State", desc.str().c_str(), CLR_TIE, ICON_CONTROL);
      tieState = true;
    break;
  }

  case EVENT_NOTE_LENGTH: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Length: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Length", desc.str().c_str(), CLR_DURNOTE);
    break;
  }

  case EVENT_ADSR_FROM_TABLE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR From Table", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_VOLUME_REL_FOREVER: {
    uint8_t arg1 = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, transpose + arg1, L"Transpose (Relative)");
    break;
  }

  case EVENT_VOLUME_REL: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 >= 0x80) {
      arg1 = 0x100 - arg1;
      spcVolL -= arg1;
      spcVolR -= arg1;
      AddVolLR(beginOffset, curOffset - beginOffset, spcVolL + volumerelativeforever, spcVolR + volumerelativeforever, L"Volume (Relative)");
    }
    else {
      spcVolL += arg1;
      spcVolR += arg1;
      AddVolLR(beginOffset, curOffset - beginOffset, spcVolL + volumerelativeforever, spcVolR + volumerelativeforever, L"Volume (Relative)");
    }
    break;
  }

  case EVENT_VOLUME: {
    int8_t newVolL = (int8_t)GetByte(curOffset++);
    int8_t newVolR = (int8_t)GetByte(curOffset++);

    spcVolL = newVolL;
    spcVolR = newVolR;
    AddVolLR(beginOffset, curOffset - beginOffset, spcVolL + volumerelativeforever, spcVolR + volumerelativeforever, L"Base Volume L/R");
    break;
  }

  case EVENT_TEMPO: {
    uint16_t newTempo = GetShort(curOffset);
    curOffset += 2;
    parentSeq->tempo = newTempo;
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM());
    break;
  }

  case EVENT_LOOP_UNITL: {
    uint8_t arg1 = GetByte(curOffset++);
    uint16_t arg2 = GetShort(curOffset++);
    curOffset++;
    desc 
      << L"Loop Count: " << (int)arg1
      << L"  Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)arg2;
    if (arg1 == 0) {
      if (!IsOffsetUsed(arg2)) {
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Jump", desc.str(), CLR_LOOPFOREVER);
        curOffset = arg2;
      }
      else {
        bContinue = AddLoopForever(beginOffset, curOffset - beginOffset);
      }
    }
    else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Until", desc.str(), CLR_LOOP, ICON_STARTREP);
      if (loopCount == 0) {
        loopCount = arg1;
        curOffset = arg2;
      }
      else {
        loopCount--;
        if (loopCount != 0) {
          curOffset = arg2;
        }
      }
    }
    break;
  }

  case EVENT_ECHO_PARAM: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc << L"Echo Volume L / R: " << (int)arg1
      << L"  Echo Feedback: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_NOTE_PARAM: {
    dur = GetByte(beginOffset);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Duration", desc.str().c_str(), CLR_DURNOTE);
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t newProg = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProg);

    uint16_t currentInstrPointer = GetShort(parentSeq->instrumentPointer) + (newProg * 4);
    spcVolL = GetByte(currentInstrPointer + 2);
    spcVolR = GetByte(currentInstrPointer + 2);
    AddVolLR(beginOffset, curOffset - beginOffset, spcVolL + volumerelativeforever, spcVolR + volumerelativeforever, L"Base Volume L/R");
    AddTranspose(beginOffset, curOffset - beginOffset, GetByte(currentInstrPointer));
    AddFineTuningNoItem(GetByte(currentInstrPointer + 1));
    break;
  }

  case EVENT_END: {
    if (subroutineReturnAddress != 0) {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str().c_str(), CLR_LOOP, ICON_ENDREP);
      curOffset = subroutineReturnAddress;
      subroutineReturnAddress = 0;
    }
    else {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    }
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t arg1 = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_ECHO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Off", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_NOTE: {
    key = GetByte(beginOffset) - 0x61;
    if (tieState == true) {
      if (key == previousKey) {
        MakePrevDurNoteEnd(GetTime() + dur);
        desc << L"Duration: " << (int)dur;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE, ICON_NOTE);
      }
      else {
        AddNoteByDur(beginOffset, curOffset - beginOffset, key, vel, dur);
        tieState = false;
      }
    }
    else {
      AddNoteByDur(beginOffset, curOffset - beginOffset, key, vel, dur);
    }
    previousKey = key;
    AddTime(dur);
    break;
  }

  case EVENT_REST: {
    AddRest(beginOffset, curOffset - beginOffset, dur);
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    uint32_t length = curOffset - beginOffset;

    curOffset = dest;
    if (!IsOffsetUsed(dest)) {
      AddGenericEvent(beginOffset, length, L"Jump", desc.str().c_str(), CLR_LOOPFOREVER);
    }
    else {
      AddGenericEvent(beginOffset, length, L"Loop", desc.str().c_str(), CLR_LOOPFOREVER);
      bContinue = false;
    }
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"GameFreakSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

void GameFreakSnesTrack::CalcVolPanFromVolLR(int8_t volL, int8_t volR, uint8_t& midiVol, uint8_t& midiPan) {
  double volumeLeft = abs(volL) / 128.0;
  double volumeRight = abs(volR) / 128.0;

  double volumeScale;
  uint8_t midiPanTemp = ConvertVolumeBalanceToStdMidiPan(volumeLeft, volumeRight, &volumeScale);
  volumeScale *= sqrt(3) / 2.0; // limit to <= 1.0

  midiVol = ConvertPercentAmpToStdMidiVal(volumeScale);
  if (volL != 0 || volR != 0) {
    midiPan = midiPanTemp;
  }
}

void GameFreakSnesTrack::AddVolLR(uint32_t offset,
  uint32_t length,
  int8_t spcVolL,
  int8_t spcVolR,
  const std::wstring& sEventName) {
  uint8_t newMidiVol;
  uint8_t newMidiPan;
  CalcVolPanFromVolLR(spcVolL, spcVolR, newMidiVol, newMidiPan);

  std::wostringstream desc;
  desc << L"Left Volume: " << spcVolL << L"  Right Volume: " << spcVolR;
  AddGenericEvent(offset, length, sEventName, desc.str(), CLR_VOLUME, ICON_CONTROL);

  // add MIDI events only if updated
  if (newMidiVol != vol) {
    AddVolNoItem(newMidiVol);
  }
  if (newMidiVol != 0 && newMidiPan != prevPan) {
    AddPanNoItem(newMidiPan);
  }
}

#include "pch.h"
#include "KazeSnesSeq.h"

DECLARE_FORMAT(KazeSnes);

//  ****************
//  KazeSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

KazeSnesSeq::KazeSnesSeq(RawFile* file,
  KazeSnesVersion ver,
  uint32_t seqdataOffset,
  uint16_t header_offset,
  uint16_t songBaseOffset,
  std::wstring newName)
  : VGMSeq(KazeSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
  headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  baseAddress = songBaseOffset;
  headerAddr = header_offset;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

KazeSnesSeq::~KazeSnesSeq(void) {
}

void KazeSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool KazeSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(headerAddr, 0);

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    

    uint16_t addrTrackStart = GetShort(headerAddr + (trackIndex * 2));
    if (addrTrackStart != 0x0000) {
      header->AddSimpleItem(headerAddr + (trackIndex * 2), 2, trackName.str());
      KazeSnesTrack* track = new KazeSnesTrack(this, addrTrackStart + baseAddress);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool KazeSnesSeq::GetTrackPointers(void) {
  return true;
}

void KazeSnesSeq::LoadEventMap() {
  if (version == KAZESNES_NONE) {
    return;
  }

  int statusByte;

  for (statusByte = 0x00; statusByte <= 0xc0; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  EventMap[0xc1] = EVENT_REST;
  EventMap[0xc2] = EVENT_ECHO_DELAY;
  EventMap[0xc3] = EVENT_ECHO_FEEDBACK;
  for (statusByte = 0xc4; statusByte <= 0xcb; statusByte++) {
    EventMap[statusByte] = EVENT_ECHO_FIR;
  }
  EventMap[0xcc] = EVENT_ADSR_MODE;
  EventMap[0xcd] = EVENT_GAIN_ENV_MODE;
  EventMap[0xce] = EVENT_TIE_STATE;
  EventMap[0xcf] = EVENT_NOISE_ON;
  EventMap[0xd0] = EVENT_NOISE_OFF;
  for (statusByte = 0xd1; statusByte <= 0xd8; statusByte++) {
    EventMap[statusByte] = EVENT_PROGCHANGE_FROMTABLE;
  }
  for (statusByte = 0xd9; statusByte <= 0xe0; statusByte++) {
    EventMap[statusByte] = EVENT_PROGTABLE_WRITE_2;
  }
  EventMap[0xe1] = EVENT_ECHO_VOLUME_PAN;
  EventMap[0xe2] = EVENT_ECHO_VOLUME;
  EventMap[0xe3] = EVENT_ECHO_ON;
  EventMap[0xe4] = EVENT_ECHO_OFF;
  for (statusByte = 0xe5; statusByte <= 0xec; statusByte++) {
    EventMap[statusByte] = EVENT_PROGTABLE_WRITE_3;
  }
  EventMap[0xed] = EVENT_MASTER_VOLUME_PAN;
  EventMap[0xee] = EVENT_MASTER_VOLUME;
  EventMap[0xef] = EVENT_PAN;
  EventMap[0xf0] = EVENT_VOLUME;
  EventMap[0xf1] = EVENT_NOTE_ON_DELAY;
  EventMap[0xf2] = EVENT_UNKNOWN0;
  EventMap[0xf3] = EVENT_UNKNOWN0;
  EventMap[0xf4] = EVENT_UNKNOWN0;
  EventMap[0xf5] = EVENT_LOOP_START;
  EventMap[0xf6] = EVENT_LOOP_END;
  EventMap[0xf7] = EVENT_SLUR_OFF;
  EventMap[0xf8] = EVENT_SLUR_ON;
  EventMap[0xf9] = EVENT_TRANSPOSE;
  EventMap[0xfa] = EVENT_TEMPO;
  EventMap[0xfb] = EVENT_MASTER_VOLUME_FADE;
  EventMap[0xfc] = EVENT_PATTERN;
  EventMap[0xfd] = EVENT_JUMP;
  EventMap[0xfe] = EVENT_PATTERN_END;
  EventMap[0xff] = EVENT_END;

  // TODO: KazeSnesSeq::LoadEventMap
}

double KazeSnesSeq::GetTempoInBPM(uint8_t tempo) {
  // cite: <http://www6.atpages.jp/appsouko/work/TAS/doc/fps.html>
  const double SNES_NTSC_FRAMERATE = 39375000.0 / 655171.0;

  unsigned int tempoValue = (tempo == 0) ? 256 : tempo;
  return 60000000.0 / (SEQ_PPQN * (1000000.0 / SNES_NTSC_FRAMERATE)) * (tempoValue / 32.0);
}

//  ******************
//  KazeSnesTrack
//  ******************

KazeSnesTrack::KazeSnesTrack(KazeSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void KazeSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  isTieState = false;
  patternGroup = 0;
  loopGroup = 0;
}

bool KazeSnesTrack::ReadEvent(void) {
  KazeSnesSeq* parentSeq = (KazeSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  KazeSnesSeqEventType eventType = (KazeSnesSeqEventType)0;
  std::map<uint8_t, KazeSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_LOOP_START: {
    uint8_t count = GetByte(curOffset++) - 1;
    desc 
      << L"Loop Count: " << (int)count;
    loopCount[loopGroup] = count;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
    loopReturnAddr[loopGroup] = curOffset;
    loopGroup++;
    break;
  }

  case EVENT_LOOP_END: {
    uint8_t prevLoopGroup = loopGroup - 1;
    if (loopCount[prevLoopGroup]-- != 0) {
      curOffset = loopReturnAddr[prevLoopGroup];
    }
    else {
      loopGroup--;
    }
    AddGenericEvent(beginOffset, 1, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
    break;
  }

  case EVENT_NOTE: {
    uint8_t noteNumber = statusByte;
    uint8_t len = GetByte(curOffset++);
    uint8_t noteDur = ((len & 0x1f) + 1) * 4;   //TODO: Duration
    if (isTieState) {
      desc << L"Duration: " << noteDur;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str(), CLR_TIE, ICON_NOTE);
      MakePrevDurNoteEnd(GetTime() + noteDur);
      AddTime(noteDur);
      isTieState = false;
    }
    else {
      AddNoteByDur(beginOffset, curOffset - beginOffset, noteNumber, 0x7f, noteDur);
      AddTime(noteDur);
      AddProgramChangeNoItem((((len & 0xf0) / 2) < 8), true);
    }
    break;
  }

  case EVENT_REST: {
    uint8_t arg1 = GetByte(curOffset++);
    AddRest(beginOffset, curOffset - beginOffset, ((arg1 & 0x1f) + 1) * 4);
    break;
  }

  case EVENT_ECHO_DELAY: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"EDL: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_ECHO_FEEDBACK: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"EFB: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_ECHO_FIR: {
    uint8_t arg1 = GetByte(curOffset++);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_ADSR_MODE: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR Mode", desc.str().c_str(), CLR_CHANGESTATE);
    break;
  }

  case EVENT_GAIN_ENV_MODE: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"GAIN Envelope Mode", desc.str().c_str(), CLR_CHANGESTATE);
    break;
  }

  case EVENT_TIE_STATE: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie State", desc.str().c_str(), CLR_TIE);
    isTieState = true;
    break;
  }

  case EVENT_NOISE_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise On", desc.str().c_str(), CLR_CHANGESTATE);
    break;
  }

  case EVENT_NOISE_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Off", desc.str().c_str(), CLR_CHANGESTATE);
    break;
  }

  case EVENT_PROGCHANGE_FROMTABLE: {
    uint8_t arg = (GetByte(beginOffset) & 0x0f) - 1;
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, arg, L"Program Change From Table");
    break;
  }

  case EVENT_PROGTABLE_WRITE_2: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Write to Program Table", desc.str().c_str(), CLR_PROGCHANGE);
    break;
  }

  case EVENT_PROGTABLE_WRITE_3: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Write to Program Table", desc.str().c_str(), CLR_PROGCHANGE);
    break;
  }

  case EVENT_ECHO_VOLUME_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Echo Pan", desc.str());
    break;
  }

  case EVENT_ECHO_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"EVOL: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_ECHO_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo On", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_ECHO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Off", desc.str().c_str(), CLR_REVERB);
    break;
  }

  case EVENT_SLUR_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur- On", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_SLUR_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur Off", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_PAN: {
    uint8_t pan = GetByte(curOffset++); // default value: 120
    AddPan(beginOffset, curOffset - beginOffset, pan / 2);
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t pan = GetByte(curOffset++); // default value: 120
    AddTranspose(beginOffset, curOffset - beginOffset, pan);
    break;
  }

  case EVENT_VOLUME: {
    uint8_t volume = GetByte(curOffset++); // default value: 120
    AddVol(beginOffset, curOffset - beginOffset, volume / 2);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++); // default value: 120
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    break;
  }

  case EVENT_NOTE_ON_DELAY: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Key On Delay", desc.str());
    break;
  }

  case EVENT_MASTER_VOLUME_PAN: {
    uint8_t arg1 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Master Pan", desc.str());
    break;
  }

  case EVENT_MASTER_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    AddMasterVol(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_MASTER_VOLUME_FADE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Arg1: " << (int)arg1;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Master Volume Fade", desc.str());
    break;
  }

  case EVENT_PATTERN: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    dest += beginOffset; // relative offset to address
    desc << "Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    uint32_t length = curOffset - beginOffset;
    patternReturnAddr[patternGroup] = curOffset;
    curOffset = dest;
    patternGroup++;
    AddGenericEvent(beginOffset, length, L"Pattern Play", desc.str().c_str(), CLR_LOOPFOREVER);
    break;
  }

  case EVENT_PATTERN_END: {
    patternGroup--;
    curOffset = patternReturnAddr[patternGroup];
    AddGenericEvent(beginOffset, 1, L"Pattern End", desc.str().c_str(), CLR_TRACKEND);
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    dest += beginOffset; // relative offset to address
    desc << "Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
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
      L"KazeSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

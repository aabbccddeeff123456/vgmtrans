#include "pch.h"
#include "ShuichiSnesSeq.h"
#include "ScaleConversion.h"

DECLARE_FORMAT(ShuichiSnes);

//  ****************
//  ShuichiSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

ShuichiSnesSeq::ShuichiSnesSeq(RawFile* file,
  ShuichiSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(ShuichiSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

ShuichiSnesSeq::~ShuichiSnesSeq(void) {
}

void ShuichiSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool ShuichiSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0, L"Sequence Header");

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint32_t addrTrackLowPtr = dwOffset + (trackIndex);
    uint32_t addrTrackHighPtr = dwOffset + MAX_TRACKS + (trackIndex);

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackLowPtr, 1, trackName.str() + L" (LSB)");
    header->AddSimpleItem(addrTrackHighPtr, 1, trackName.str() + L" (MSB)");

    uint16_t addrTrackStart = GetByte(addrTrackLowPtr) | (GetByte(addrTrackHighPtr) << 8);
    if (addrTrackStart != 0xffff || addrTrackStart != 0x0000) {
      ShuichiSnesTrack* track = new ShuichiSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool ShuichiSnesSeq::GetTrackPointers(void) {
  return true;
}

void ShuichiSnesSeq::LoadEventMap() {
  if (version == SHUICHISNES_NONE) {
    return;
  }

  //80 7f 7f 7d 7c 7a 78 76 73 70 6d 69 65 61 5d 58
  //54 4f 49 44 3e 39 33 2d 27 21 1a 13 0a 00

  const uint8_t SHUICHISNES_PAN_TABLE_STANDARD[30] = {
      0x00, 0x0A, 0x13, 0x1A, 0x21, 0x27, 0x2D, 0x33,
      0x39, 0x3E, 0x44, 0x49, 0x4F, 0x54, 0x58, 0x5D,
      0x61, 0x65, 0x69, 0x6D, 0x70, 0x73, 0x76, 0x78,
      0x7a, 0x7c, 0x7d, 0x7f, 0x7f, 0x80
  };

  if (panTable.empty()) {
    panTable.assign(std::begin(SHUICHISNES_PAN_TABLE_STANDARD), std::end(SHUICHISNES_PAN_TABLE_STANDARD));
  }

  int statusByte;

  for (statusByte = 0xac; statusByte <= 0xfe; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  EventMap[0x80] = EVENT_TRACK_END;
  EventMap[0x81] = EVENT_INFINITE_LOOP_START;
  EventMap[0x82] = EVENT_LOOP_FOREVER;
  EventMap[0x83] = EVENT_LOOP_START;
  EventMap[0x84] = EVENT_LOOP_END;
  EventMap[0x85] = EVENT_UNKNOWN0;
  EventMap[0x86] = EVENT_SUBROUTINE_START;
  EventMap[0x87] = EVENT_SUBROUTINE_END;
  EventMap[0x88] = EVENT_UNKNOWN0;
  EventMap[0x89] = EVENT_PROGCHANGE;
  EventMap[0x8a] = EVENT_RELEASE_RATE;
  EventMap[0x8b] = EVENT_TEMPO;
  EventMap[0x8c] = EVENT_TRANSPOSE;
  EventMap[0x8d] = EVENT_TRANSPOSE_REL;
  EventMap[0x8e] = EVENT_TUNING;
  EventMap[0x8f] = EVENT_PITCH_FADE;
  EventMap[0x90] = EVENT_DURATION_RATE;

  EventMap[0x92] = EVENT_VOLUME_PAN;
  EventMap[0x93] = EVENT_VOLUME;
  EventMap[0x94] = EVENT_VOLUME_REL;
  EventMap[0x95] = EVENT_VOLUME_REL_ONE_TIME;
  EventMap[0x96] = EVENT_PAN;

  EventMap[0x98] = EVENT_PAN_FADE;
  EventMap[0x99] = EVENT_PAN_LFO;
  EventMap[0x9a] = EVENT_ECHO_VOLUME;
  EventMap[0x9b] = EVENT_ECHO_PARAM;
  EventMap[0x9c] = EVENT_ECHO_ON;
  EventMap[0x9d] = EVENT_VIBRATO;
  EventMap[0x9e] = EVENT_VIBRATO_OFF;
  EventMap[0x9f] = EVENT_REST;
  EventMap[0xa0] = EVENT_UNKNOWN0;
  EventMap[0xa1] = EVENT_UNKNOWN0;

  EventMap[0xa3] = EVENT_UNKNOWN1;
  EventMap[0xa4] = EVENT_MUITI_FLAG;
  EventMap[0xa5] = EVENT_GAIN_DURATION_RATE;  // Not sure
  EventMap[0xa6] = EVENT_SEND_TO_APU2;
  EventMap[0xab] = EVENT_UNKNOWN1;

  EventMap[0xff] = EVENT_TIE;
  // TODO: ShuichiSnesSeq::LoadEventMap
  // TODO: Ardy Lightfoot has difficult vcmd table.(Should be in version "SHUICHISNES_VER2")
}

double ShuichiSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0 && GetByte(0xfa) != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * 0x0f)) * (tempo / 256.0);
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

//  ******************
//  ShuichiSnesTrack
//  ******************

ShuichiSnesTrack::ShuichiSnesTrack(ShuichiSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void ShuichiSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  loopGroup = 0;
  loopCount[loopGroup] = 0;
  subroutineGroup = 0;
  onetimeVolumeRel = false;
  tieState = false;
  duration = 0;
}

bool ShuichiSnesTrack::ReadEvent(void) {
  ShuichiSnesSeq* parentSeq = (ShuichiSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  ShuichiSnesSeqEventType eventType = (ShuichiSnesSeqEventType)0;
  std::map<uint8_t, ShuichiSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_GAIN_DURATION_RATE: {
    uint8_t arg1 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Duration Rate (Before GAIN)", desc.str().c_str(), CLR_DURNOTE, ICON_CONTROL);
    break;
  }

  case EVENT_MUITI_FLAG: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t newPan = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    double volumeScale;
    bool reverseLeft;
    bool reverseRight;
    int8_t midiPan = CalcPanValue(newPan, volumeScale, reverseLeft, reverseRight);
    //AddPan(beginOffset, curOffset - beginOffset, midiPan);
    AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
    desc 
      << L"Program: " << (int)arg1
      << L"  Volume: " << (int)arg2
      << L"  Pan: " << (int)midiPan
      << L"  Transpose: " << (int)arg4;
    AddProgramChangeNoItem(arg1, true);
    AddVolNoItem(arg2);
    AddPanNoItem(midiPan);
    //transpose = arg4;
    AddTranspose(beginOffset + 4, 1, arg4);
    AddGenericEvent(beginOffset, curOffset - beginOffset - 1, L"Muiti Event", desc.str().c_str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_PARAM: {
    uint8_t spcEDL = GetByte(curOffset++);
    uint8_t spcEFB = GetByte(curOffset++);
    uint8_t spcFIR = GetByte(curOffset++);

    desc << L"Delay: " << (int)spcEDL << L"  Feedback: " << (int)spcEFB << L"  FIR: " << (int)spcFIR;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_VOLUME: {
    uint8_t volL = GetByte(curOffset++);
    uint8_t volR = GetByte(curOffset++);
    desc << L"Volume Left: " << (int)volL << L"  Volume Right: " << (int)volR;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Echo Volume",
      desc.str().c_str(),
      CLR_REVERB,
      ICON_CONTROL);
    break;
  }

  case EVENT_DURATION_RATE: {
    uint8_t rate = GetByte(curOffset++) / 0xff * 100;
    if (rate == 0 || rate == 100) {
      rate = 100;
    }

    desc << L"Note Length: " << (int)rate << " %";
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Duration Rate",
      desc.str().c_str(),
      CLR_DURNOTE,
      ICON_CONTROL);
    break;
  }

  case EVENT_PAN_LFO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t orgArg3 = arg3;
    if (arg3 >= 0x7f) {
      arg3 = 0x100 - arg3;
    }
    uint8_t arg4 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1
      << L"  Duration: " << (int)arg2;
      if (orgArg3 >= 0x7f) {
        desc << L"  Target Volume (Relative): -" << (int)arg3;
      }
      else {
        desc << L"  Target Volume (Relative): " << (int)arg3;
      }
      desc << L"  Times: " << (int)arg4;
      if (orgArg3 <= 0x7f) {
        vol += (arg3 * arg4);
      }
      else {
        vol -= (arg3 * arg4);
      }
      desc << L"  Volume: " << (int)vol;
      AddVolNoItem(vol);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume Fade", desc.str(), CLR_VOLUME, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_ON: {
    uint8_t targetChannels = GetByte(curOffset++);

    desc << L"Tracks:";
    for (uint8_t trackIndex = 0; trackIndex < 8; trackIndex++) {
      if ((targetChannels & (0x80 >> trackIndex)) != 0) {
        desc << L" " << (trackIndex + 1);
      }
    }

    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo ON", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_PITCH_FADE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Duration: " << (int)arg1
      << L"  Semitones: " << (int)arg2
      << L"  Note (From): " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Fade", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1
      << L"  Speed: " << (int)arg2
      << L"  Rate: " << (int)arg3
      << L"  Depth: " << (int)arg4;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t newTempo = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
    break;
  }

  case EVENT_VOLUME_PAN: {
    uint8_t newProg = GetByte(curOffset++);
    uint8_t newPan = GetByte(curOffset++);

    double volumeScale;
    bool reverseLeft;
    bool reverseRight;
    int8_t midiPan = CalcPanValue(newPan, volumeScale, reverseLeft, reverseRight);
    desc
      << L"Volume: " << (int)newProg
      << L"  Pan: " << (int)midiPan;
    AddVolNoItem(newProg);
    AddPanNoItem(midiPan);
    AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume & Pan", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_VOLUME: {
    uint8_t newProg = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, newProg);
    break;
  }

  case EVENT_PAN: {
    // Pan calc:
    // 00 - 0e left,0f middle,10-1e right.
    // 00 - 38      40        42-7f         <-- possible values.
      uint8_t newPan = GetByte(curOffset++);

      double volumeScale;
      bool reverseLeft;
      bool reverseRight;
      int8_t midiPan = CalcPanValue(newPan, volumeScale, reverseLeft, reverseRight);
      AddPan(beginOffset, curOffset - beginOffset, midiPan);
      AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
      break;
  }

  case EVENT_PAN_FADE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Speed: " << (int)arg1
      << L"  Duration (Related-to Speed): " << (int)arg2 + arg1
      << L"  Rate: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan LFO", desc.str(), CLR_PAN, ICON_CONTROL);
    break;
  }

  case EVENT_VOLUME_REL: {
    uint8_t newProg = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, vol + newProg, L"Volume (Relative)");
    break;
  }

  case EVENT_VOLUME_REL_ONE_TIME: {
    onetimeVolumeRel = true;
    prevVolume = vol;
    uint8_t newProg = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, vol + newProg, L"Volume (Relative,One-Time)");
    break;
  }

  case EVENT_TUNING: {
    uint8_t newProg = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, newProg);
    break;
  }

  case EVENT_RELEASE_RATE: {
    uint8_t gain = GetByte(curOffset++) & 0x3f;
    desc << L"GAIN: " << (int)gain;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Release Rate (GAIN)",
      desc.str().c_str(),
      CLR_ADSR,
      ICON_CONTROL);
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t newProg = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, newProg);
    break;
  }

  case EVENT_TRANSPOSE_REL: {
    uint8_t newProg = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, transpose + newProg, L"Transpose (Relative)");
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t newProg = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, newProg);
    break;
  }

  case EVENT_SEND_TO_APU2: {
    uint8_t arg1 = GetByte(curOffset++);
    desc 
      << L"Arg1: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Send to APU2", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Off", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_INFINITE_LOOP_START: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Infinite Loop Point", desc.str(), CLR_LOOP, ICON_STARTREP);
    break;
  }

  case EVENT_LOOP_FOREVER: {
    bContinue = AddLoopForever(beginOffset, curOffset - beginOffset);
    break;
  }

  case EVENT_TRACK_END: {
    bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
    break;
  }

  case EVENT_NOTE: {
    uint8_t key = GetByte(beginOffset) - 0xac;
    if (GetByte(beginOffset + 1) <= 0x7f) {
      duration = GetByte(curOffset++);
    }
    if (tieState == false) {
      AddNoteByDur(beginOffset, curOffset - beginOffset, key, 0x7f, duration);
      if (onetimeVolumeRel == true) {
        onetimeVolumeRel = false;
        AddVolNoItem(prevVolume);
      }
    }
   else {
      if (prevKey = key) {
        MakePrevDurNoteEnd(GetTime() + duration);
        desc << L"Duration: " << (int)duration;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE, ICON_NOTE);
        tieState = false;
      }
      else {
        tieState = false;
        AddNoteByDur(beginOffset, curOffset - beginOffset, key, 0x7f, duration);
        if (onetimeVolumeRel == true) {
          onetimeVolumeRel = false;
          AddVolNoItem(prevVolume);
        }
      }
    }
    AddTime(duration);
    break;
  }

  case EVENT_TIE: {
    tieState = true;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie State", desc.str().c_str(), CLR_TIE, ICON_NOTE);
    /*if (GetByte(beginOffset + 1) >= 0xac) {
      tieState = true;
    }
    else {
      tieState = false;
    }*/
    break;
  }

  case EVENT_REST: {
    if (GetByte(beginOffset + 1) <= 0x7f) {
      duration = GetByte(curOffset++);
    }
    AddRest(beginOffset, curOffset - beginOffset, duration);
    tieState = false;
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
    uint8_t prevLoopLevel = (loopGroup != 0 ? loopGroup : SHUICHISNES_LOOP_LEVEL_MAX) - 1;
    if (loopCount[prevLoopLevel] == 0) {
      loopCount[prevLoopLevel] = looptimes;
    }
    desc
      << L"Loop Count: " << (int)looptimes;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
    loopCount[prevLoopLevel]--;
    if (loopCount[prevLoopLevel] != 0) {
      curOffset = loopReturnAddress[prevLoopLevel];
    }
    else {
      loopGroup--;
    }
    break;
  }

  case EVENT_SUBROUTINE_START: {
    uint16_t dest = GetShort(curOffset++);
    curOffset++;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4)
      << std::uppercase << (int)dest;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Goto Subroutine", desc.str(), CLR_LOOP, ICON_STARTREP);
    subroutineReturnAddress[subroutineGroup] = curOffset;
    curOffset = dest;
    subroutineGroup++;
    break;
  }

  case EVENT_SUBROUTINE_END: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str(), CLR_LOOP, ICON_ENDREP);
    uint8_t prevLoopLevel = (subroutineGroup != 0 ? subroutineGroup : SHUICHISNES_SUBROUTINE_LEVEL_MAX) - 1;
    curOffset = subroutineReturnAddress[prevLoopLevel];
    subroutineGroup--;
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
      LOG_LEVEL_ERR,
      L"ShuichiSnesSeq"));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint8_t ShuichiSnesTrack::ReadPanTable(uint16_t pan) {
  ShuichiSnesSeq* parentSeq = (ShuichiSnesSeq*)this->parentSeq;

  uint8_t panIndex = pan >> 8;
  uint8_t panFraction = pan & 0xff;

  uint8_t panMaxIndex = (uint8_t)(parentSeq->panTable.size() - 1);
  if (panIndex > panMaxIndex) {
    // unexpected behavior
    panIndex = panMaxIndex;
    panFraction = 0; // floor(pan)
  }

  uint8_t volumeRate = parentSeq->panTable[panIndex];

  // linear interpolation for pan fade
  uint8_t nextVolumeRate = (panIndex < panMaxIndex) ? parentSeq->panTable[panIndex + 1] : volumeRate;
  uint8_t volumeRateDelta = nextVolumeRate - volumeRate;
  volumeRate += (volumeRateDelta * panFraction) >> 8;

  return volumeRate;
}

void ShuichiSnesTrack::GetVolumeBalance(uint16_t pan, double& volumeLeft, double& volumeRight) {
  ShuichiSnesSeq* parentSeq = (ShuichiSnesSeq*)this->parentSeq;

  uint8_t paShuichidex = pan >> 8;
    uint8_t panMaxIndex = (uint8_t)(parentSeq->panTable.size() - 1);
    if (paShuichidex > panMaxIndex) {
      // unexpected behavior
      pan = panMaxIndex << 8;
      paShuichidex = panMaxIndex;
    }

    // actual engine divides pan by 256, though pan value is 7-bit
    // by the way, note that it is right-to-left pan
    volumeRight = ReadPanTable((panMaxIndex << 8) - pan) / 128.0;
    volumeLeft = ReadPanTable(pan) / 128.0;

    std::swap(volumeLeft, volumeRight);

}

int8_t ShuichiSnesTrack::CalcPanValue(uint8_t pan, double& volumeScale, bool& reverseLeft, bool& reverseRight) {
  ShuichiSnesSeq* parentSeq = (ShuichiSnesSeq*)this->parentSeq;

  uint8_t panIndex;
    panIndex = pan & 0x1f;
    reverseLeft = (pan & 0x80) != 0;
    reverseRight = (pan & 0x40) != 0;

  double volumeLeft;
  double volumeRight;
  GetVolumeBalance(panIndex << 8, volumeLeft, volumeRight);

  // TODO: correct volume scale of TOSE sequence
  int8_t midiPan = ConvertVolumeBalanceToStdMidiPan(volumeLeft, volumeRight, &volumeScale);
  volumeScale = min(volumeScale, 1.0); // workaround

  return midiPan;
}

#include "pch.h"
#include "ArcSystemWorksSnesSeq.h"

DECLARE_FORMAT(ArcSystemWorkSnes);

//  ****************
//  ArcSystemWorkSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48


ArcSystemWorkSnesSeq::ArcSystemWorkSnesSeq(RawFile *file, ArcSystemWorkSnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
: VGMSeq(ArcSystemWorkSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
bLoadTickByTick = true;
bAllowDiscontinuousTrackData = true;
bUseLinearAmplitudeScale = true;

UseReverb();
AlwaysWriteInitialReverb(0);

LoadEventMap();
}

ArcSystemWorkSnesSeq::~ArcSystemWorkSnesSeq(void) {
}

void ArcSystemWorkSnesSeq::ResetVars(void) {
VGMSeq::ResetVars();
}

bool ArcSystemWorkSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  uint32_t curOffset = dwOffset;
  uint32_t beginOffset = curOffset;
  uint8_t statusByte = GetByte(curOffset++);
  uint16_t startAddress = dwOffset + 0x01;
  VGMHeader* header = AddHeader(dwOffset, 0, L"Sequence Header");
  header->AddSimpleItem(dwOffset, 1, L"Number of Tracks");
  nNumTracks = GetByte(dwOffset);
  for (uint8_t trackIndex = 0; trackIndex < nNumTracks; trackIndex++) {
    uint16_t sectionListOffsetPtr = startAddress + (trackIndex * 6);
    std::wstringstream pointersheaderName;
    pointersheaderName << L"Track " << (trackIndex + 1);
    VGMHeader* aHeader = header->AddHeader(sectionListOffsetPtr, 6, pointersheaderName.str());
    aHeader->AddSimpleItem(sectionListOffsetPtr, 2, L"Track Pointer");
    aHeader->AddSimpleItem(sectionListOffsetPtr + 0x02, 1, L"Coarse Tuning");
    aHeader->AddSimpleItem(sectionListOffsetPtr + 0x03, 1, L"Program Number");
    aHeader->AddSimpleItem(sectionListOffsetPtr + 0x04, 2, L"Volume L / R");
    uint16_t sectionListAddress = GetShort(sectionListOffsetPtr);
    ArcSystemWorkSnesTrack* track = new ArcSystemWorkSnesTrack(this, sectionListAddress);
    aTracks.push_back(track);
  }


return true;
}

bool ArcSystemWorkSnesSeq::GetTrackPointers(void) {
  return true;
}

void ArcSystemWorkSnesSeq::LoadEventMap() {
  TIMER0_FREQUENCY = GetByte(0xfa);
  if (version == ARCSYSTEMSNES_SHINGFX) {
    for (int statusByte = 0x00; statusByte <= 0x56; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE_SHINGFX;
    }
    for (int statusByte = 0x57; statusByte <= 0x64; statusByte++) {
      EventMap[statusByte] = EVENT_PERCUSSION_NOTE_SHINGFX;
    }
    for (int statusByte = 0x80; statusByte <= 0xff; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE_WITHOUT_DUR;
    }
    EventMap[0x65] = EVENT_NOTE_PARAM_SHINGFX;
    EventMap[0x66] = EVENT_JUMP;
    EventMap[0x67] = EVENT_SUBROUTINE;
    EventMap[0x68] = EVENT_SUBROUTINE_END;
    EventMap[0x69] = EVENT_ECHO_PARAM;
    EventMap[0x6a] = EVENT_ECHO_ON;
    EventMap[0x6b] = EVENT_ECHO_OFF;
    EventMap[0x6c] = EVENT_END;
    EventMap[0x6d] = EVENT_VOLUME;
    EventMap[0x6e] = EVENT_PROGRAM_CHANGE;
    EventMap[0x6f] = EVENT_PROFESSIONAL_LOOP;
    EventMap[0x70] = EVENT_TRANSPOSE;
    EventMap[0x71] = EVENT_PITCH_SLIDE;
    EventMap[0x72] = EVENT_PITCH_LFO;
    EventMap[0x73] = EVENT_VIBRATO_OFF;
    EventMap[0x74] = EVENT_END;
    EventMap[0x75] = EVENT_MASTER_VOLUME;
    EventMap[0x76] = EVENT_VOLENV;
    EventMap[0x79] = EVENT_UNKNOWN2;
  }
  else {
    for (int statusByte = 0x00; statusByte <= 0xcf; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    for (int statusByte = 0xd0; statusByte <= 0xde; statusByte++) {
      EventMap[statusByte] = EVENT_PERCUSSION_NOTE;
    }
    EventMap[0xdf] = EVENT_NOTE_PARAM;
    EventMap[0xe0] = EVENT_JUMP;
    EventMap[0xe1] = EVENT_SUBROUTINE;
    EventMap[0xe2] = EVENT_SUBROUTINE_END;
    EventMap[0xe3] = EVENT_ECHO_PARAM;
    EventMap[0xe4] = EVENT_ECHO_ON;
    EventMap[0xe5] = EVENT_ECHO_OFF;
    EventMap[0xe6] = EVENT_END;
    EventMap[0xe7] = EVENT_VOLUME;
    EventMap[0xe8] = EVENT_PROGRAM_CHANGE;
    EventMap[0xe9] = EVENT_PROFESSIONAL_LOOP;
    EventMap[0xea] = EVENT_TRANSPOSE;
    EventMap[0xeb] = EVENT_PITCH_SLIDE;
    EventMap[0xec] = EVENT_PITCH_LFO;
    EventMap[0xed] = EVENT_VIBRATO_OFF;
    EventMap[0xef] = EVENT_MASTER_VOLUME;
    EventMap[0xf0] = EVENT_VOLENV;
    EventMap[0xf3] = EVENT_TEMPO;
    EventMap[0xf5] = EVENT_TIE;
    EventMap[0xf8] = EVENT_ADSR;
    EventMap[0xf9] = EVENT_NOISE_ADSR;
    EventMap[0xfa] = EVENT_NOISE_OFF;
    EventMap[0xfb] = EVENT_PROGADD;
    EventMap[0xfc] = EVENT_TUNING;
    // TODO: ArcSystemWorkSnesSeq::LoadEventMap
  }
}

double ArcSystemWorkSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0 && TIMER0_FREQUENCY != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * TIMER0_FREQUENCY)) * (tempo / 256.0);
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

uint16_t ArcSystemWorkSnesSeq::ConvertToAPUAddress(uint16_t offset) {
  return offset;
}

uint16_t ArcSystemWorkSnesSeq::GetShortAddress(uint32_t offset) {
  return ConvertToAPUAddress(GetShort(offset));
}

//  ******************
//  ArcSystemWorkSnesTrack
//  ******************

ArcSystemWorkSnesTrack::ArcSystemWorkSnesTrack(ArcSystemWorkSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void ArcSystemWorkSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  returnAddress = 0;
  noteDeltaTime = 0;
  noteDuration = 0;
  currentProgram = 0;
}

bool ArcSystemWorkSnesTrack::ReadEvent(void) {
  ArcSystemWorkSnesSeq* parentSeq = (ArcSystemWorkSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  ArcSystemWorkSnesSeqEventType eventType = (ArcSystemWorkSnesSeqEventType)0;
  std::map<uint8_t, ArcSystemWorkSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_VOLENV: {
    uint8_t envelopeIndex = GetByte(curOffset++);
    desc << L"Envelope: " << (int)envelopeIndex;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Volume Envelope",
      desc.str().c_str(),
      CLR_VOLUME,
      ICON_CONTROL);
    break;
  }

  case EVENT_PITCH_SLIDE: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc 
      << L"Delay: " << (int)arg1
      << L"  Duration: " << (int)arg2
      << L"  Note: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Slide", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
    break;
  }

  case EVENT_MASTER_VOLUME: {
    int8_t newVolL = (int8_t)GetByte(curOffset++);
    int8_t newVolR = (int8_t)GetByte(curOffset++);
    int8_t newVol = min(abs((int)newVolL) + abs((int)newVolR), 255) / 2;
    AddMasterVol(beginOffset, curOffset - beginOffset, newVol, L"Master Volume L/R");
    break;
  }

  case EVENT_ECHO_PARAM: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    uint8_t arg4 = GetByte(curOffset++);
    uint8_t arg5 = GetByte(curOffset++);
    desc 
      << L"FIR Index: " << (int)arg1
      << L"  Delay: " << (int)arg2
      << L"  Volume Left: " << (int)arg3
      << L"  Volume Right: " << (int)arg4
      << L"  Feedback: " << (int)arg5;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo On", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Off", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_NOISE_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Off", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_VIBRATO_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Off", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_TRANSPOSE: {
    uint8_t arg1 = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, transpose + arg1,L"Transpose (Relative)");
    break;
  }

  case EVENT_NOTE: {
    uint8_t note = GetByte(beginOffset);
    uint8_t testanythedur = GetByte(curOffset++);
    uint8_t lengthnote = 1;
      if (testanythedur <= 0x60) {
        noteDuration = testanythedur;
        lengthnote = 2;
      }
      else if (testanythedur == 0xfd) {
        uint16_t arg2 = GetShort(curOffset++);
        curOffset++;
        //;uint8_t arg3 = GetByte(curOffset++);
        noteDuration = arg2;
          lengthnote = 4;
      }
      else {
        curOffset--;
        lengthnote = 1;
      }
      AddNoteByDur(beginOffset, lengthnote, note, 0x7f, noteDuration);
      AddTime(noteDuration);
    break;
  }

case EVENT_NOTE_SHINGFX: {
    uint8_t note = GetByte(beginOffset);
    uint8_t testanythedur = GetByte(curOffset++);
    uint8_t lengthnote = 2;
      if (testanythedur == 0x79) {
          uint16_t arg2 = GetShort(curOffset++);
          curOffset++;
          //;uint8_t arg3 = GetByte(curOffset++);
          noteDuration = arg2;
          lengthnote = 4;
      }
noteDuration = testanythedur;
      AddNoteByDur(beginOffset, lengthnote, note, 0x7f, noteDuration);
      AddTime(noteDuration);
          break;
  }

case EVENT_NOTE_WITHOUT_DUR: {
  uint8_t note = GetByte(beginOffset);
  uint8_t lengthnote = 1;
  uint8_t noteNumber = note -= 0x80;
  AddNoteByDur(beginOffset, lengthnote, note, 0x7f, noteDuration);
  AddTime(noteDuration);
  break;
}

case EVENT_PERCUSSION_NOTE_SHINGFX: {
  uint8_t note = GetByte(beginOffset);
  uint8_t testanythedur = GetByte(curOffset++);
  uint8_t lengthnote = 2;
  uint8_t noteNumber = note -= 0x57;
  noteDuration = testanythedur;
  if (testanythedur == 0x79) {
    uint16_t arg2 = GetShort(curOffset++);
    curOffset++;
    //;uint8_t arg3 = GetByte(curOffset++);
    noteDuration = arg2;
    lengthnote = 4;
  }
  AddPercNoteByDur(beginOffset,
    lengthnote,
    noteNumber,
    0x7f,
    noteDuration, L"Percussion Note");
  AddTime(noteDuration);
  break;
}


  case EVENT_PERCUSSION_NOTE: {
    uint8_t noteNumber = GetByte(beginOffset) - 0xd0;
    uint8_t testanythedur = GetByte(curOffset++);
    uint8_t lengthnote = 1;
    if (testanythedur <= 0x60) {
      noteDuration = testanythedur;
      lengthnote = 2;
    }
    else if (testanythedur == 0xfd) {
      uint16_t arg2 = GetShort(curOffset++);
      curOffset++;
      //;uint8_t arg3 = GetByte(curOffset++);
      noteDuration = arg2;
      lengthnote = 4;
    }
    else {
      curOffset--;
      lengthnote = 1;
    }
    AddPercNoteByDur(beginOffset,
      lengthnote,
      noteNumber,
      0x7f,
      noteDuration, L"Percussion Note");
    AddTime(noteDuration);
    break;
  }

  case EVENT_NOTE_PARAM_SHINGFX: {
    uint8_t arg1 = GetByte(curOffset++);
      noteDeltaTime = arg1;
      if (arg1 == 0x79) {
        uint16_t arg2 = GetShort(curOffset++);
        curOffset++;
        //;uint8_t arg3 = GetByte(curOffset++);
        noteDeltaTime = arg2;
      }
      AddTime(noteDeltaTime);
      desc
        << L"Delta Time: " << (int)noteDeltaTime;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Param", desc.str(), CLR_REST, ICON_CONTROL);
    break;
  }

  case EVENT_NOTE_PARAM: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 >= 0x75 && arg1 != 0xfd) {
      curOffset--;
    }
    else if (arg1 == 0xfd) {
      uint16_t arg2 = GetShort(curOffset++);
      curOffset++;
      //;uint8_t arg3 = GetByte(curOffset++);
      noteDeltaTime = arg2;
      AddTime(noteDeltaTime);
      desc << L"Param ID: " << (int)arg1
        << L"  Delta Time: " << (int)noteDeltaTime;
    }
    else {
      noteDeltaTime = arg1;
      AddTime(noteDeltaTime);
      desc
        << L"Delta Time: " << (int)noteDeltaTime;
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Param", desc.str(), CLR_REST, ICON_CONTROL);
    break;
  }

  case EVENT_TEMPO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    if (arg1 == 0x00) {
      AddTempoBPM(beginOffset, curOffset - beginOffset, arg2);
    }
    else {
      AddTempoSlide(beginOffset, curOffset - beginOffset, arg1, arg2);
    }
    break;
  }

  case EVENT_PITCH_LFO: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Delay: " << (int)arg1
      << L"  Rate: " << (int)arg2
      << L"  Depth: " << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION, ICON_CONTROL);
    break;
  }

  case EVENT_TUNING: {
    uint8_t arg1 = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, arg1, L"Tuning");
    break;
  }

  case EVENT_NOISE_ADSR: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Noise Frequency(NCK): " << (int)arg1
      << L"ADSR1: " << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)arg2
      << L"  ADSR2: " << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise & ADSR", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_ADSR: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc
      << L"ADSR1: " << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)arg1
      << L"  ADSR2: " << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR", desc.str(), CLR_MISC, ICON_CONTROL);
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShortAddress(curOffset);
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
      //bContinue = AddLoopForever(beginOffset, length, L"Jump");
    }
    break;
  }

  case EVENT_TIE: {
    uint8_t arg1 = GetByte(curOffset++);
    if (arg1 >= 0x75) {
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Arg1: " << (int)arg1;
      desc << L"  Arg2: " << (int)arg2;
    }
    else {
      desc
        << L"Duration: " << (int)arg1;
      AddTime(arg1);
    }
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie Param", desc.str(), CLR_TIE, ICON_NOTE);
    break;
  }

  case EVENT_SUBROUTINE: {
    uint16_t dest = GetShortAddress(curOffset);
    curOffset += 2;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    uint32_t length = curOffset - beginOffset;
    AddGenericEvent(beginOffset, length, L"Call Subroutine", desc.str(), CLR_LOOP, ICON_STARTREP);
    returnAddress = beginOffset += 0x03;
    curOffset = dest;
    break;
  }

  case EVENT_SUBROUTINE_END: {
    if (returnAddress != 0x00) {
      curOffset = returnAddress;
    }
    AddGenericEvent(beginOffset, 1, L"Subroutine End", desc.str(), CLR_LOOP, ICON_ENDREP);
    break;
  }

  case EVENT_PROGADD: {
    uint8_t arg1 = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, currentProgram + arg1, L"Program Add");
    break;
  }

  case EVENT_PROGRAM_CHANGE: {
    uint8_t arg1 = GetByte(curOffset++);
    currentProgram = arg1;
    AddProgramChange(beginOffset, curOffset - beginOffset, arg1);
    break;
  }

  case EVENT_END: {
    AddEndOfTrack(beginOffset, curOffset - beginOffset);
    bContinue = false;
    break;
  }

  case EVENT_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    desc 
      << L"Volume Left: " << (int)arg1
      << L"  Volume Right: " << (int)arg2;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume L / R", desc.str(), CLR_VOLUME, ICON_CONTROL);
    break;
  }

  case EVENT_PROFESSIONAL_LOOP: {
    uint8_t repeatSlot = GetByte(curOffset++);
    uint8_t times = GetByte(curOffset++);
    uint16_t dest = GetShortAddress(curOffset++);
    curOffset += 1;
    wchar_t* repeatEventName;
    if (repeatSlot == 0)
      repeatEventName = L"Repeat Until #1";
    if (repeatSlot == 1)
      repeatEventName = L"Repeat Until #2";
    if (repeatSlot == 2)
      repeatEventName = L"Repeat Until #3";
    if (repeatSlot == 3)
      repeatEventName = L"Repeat Until #4";
    desc 
      << L"Slot Index: " << (int)repeatSlot
      << L"  Loop Count: " << (int)times
      << L"  Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    if (times == 0 && repeatCount[repeatSlot] == 0) {
      // infinite loop
      bContinue = AddLoopForever(beginOffset, curOffset - beginOffset, repeatEventName);

      if (readMode == READMODE_ADD_TO_UI) {
        if (GetByte(curOffset) == 0x17) {
          AddEndOfTrack(curOffset, 1);
        }
      }
    }
    else {
      AddGenericEvent(beginOffset, curOffset - beginOffset, repeatEventName, desc.str(), CLR_LOOP, ICON_STARTREP);
      if (repeatCount[repeatSlot] == 0) {
        repeatCount[repeatSlot] = times;
        curOffset = dest;
      }
      else {
        repeatCount[repeatSlot]--;
        if (repeatCount[repeatSlot] != 0) {
          curOffset = dest;
        }
      }
    }
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
      LOG_LEVEL_ERR,
      std::wstring(L"ArcSystemWorkSnesSeq")));
    bContinue = false;
    break;
  }


  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint16_t ArcSystemWorkSnesTrack::ConvertToAPUAddress(uint16_t offset) {
  ArcSystemWorkSnesSeq* parentSeq = (ArcSystemWorkSnesSeq*)this->parentSeq;
  return parentSeq->ConvertToAPUAddress(offset);
}

uint16_t ArcSystemWorkSnesTrack::GetShortAddress(uint32_t offset) {
  ArcSystemWorkSnesSeq* parentSeq = (ArcSystemWorkSnesSeq*)this->parentSeq;
  return parentSeq->GetShortAddress(offset);
}




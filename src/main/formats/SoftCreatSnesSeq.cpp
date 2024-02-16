#include "pch.h"
#include "SoftCreatSnesSeq.h"

DECLARE_FORMAT(SoftCreatSnes);

//  ****************
//  SoftCreatSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

SoftCreatSnesSeq::SoftCreatSnesSeq(RawFile *file,
                                   SoftCreatSnesVersion ver,
                                   uint32_t seqdataOffset,
                                   uint8_t headerAlignSize,
                                   std::wstring newName)
    : VGMSeq(SoftCreatSnesFormat::name, file, seqdataOffset, 0, newName), version(ver),
      headerAlignSize(headerAlignSize) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

SoftCreatSnesSeq::~SoftCreatSnesSeq(void) {
}

void SoftCreatSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool SoftCreatSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader *header = AddHeader(dwOffset, headerAlignSize * MAX_TRACKS * 2);
  if (dwOffset + headerAlignSize * MAX_TRACKS > 0x10000) {
    return false;
  }

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint32_t addrTrackLowPtr = dwOffset + (trackIndex * 2 * headerAlignSize);
    uint32_t addrTrackHighPtr = addrTrackLowPtr + headerAlignSize;
    if (addrTrackLowPtr + 1 > 0x10000 || addrTrackHighPtr + 1 > 0x10000) {
      return false;
    }

    std::wstringstream trackName;
    trackName << L"Track Pointer " << (trackIndex + 1);
    header->AddSimpleItem(addrTrackLowPtr, 1, trackName.str() + L" (LSB)");
    header->AddSimpleItem(addrTrackHighPtr, 1, trackName.str() + L" (MSB)");

    uint16_t addrTrackStart = GetByte(addrTrackLowPtr) | (GetByte(addrTrackHighPtr) << 8);
    if (addrTrackStart != 0xffff) {
      SoftCreatSnesTrack *track = new SoftCreatSnesTrack(this, addrTrackStart);
      aTracks.push_back(track);
    }
  }

  return true;
}

bool SoftCreatSnesSeq::GetTrackPointers(void) {
  return true;
}

void SoftCreatSnesSeq::LoadEventMap() {
  if (version == SOFTCREATSNES_NONE) {
    return;
  }
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
  for (statusByte = 0xc7; statusByte <= 0xff; statusByte++) {
    EventMap[statusByte] = EVENT_APU_PORTS;
  }
  EventMap[0x80] = EVENT_END;
  EventMap[0x81] = EVENT_GOTO;
  EventMap[0x82] = EVENT_CALL_SUBROUTINE;
  EventMap[0x83] = EVENT_SUBROUTINE_END;
  EventMap[0x84] = EVENT_REPEAT_START;
  EventMap[0x85] = EVENT_REPEAT_END;
  EventMap[0x86] = EVENT_DEFAULT_NOTE_LENGTH;
  EventMap[0x87] = EVENT_DEFAULT_NOTE_LENGTH_CLEAR;
  EventMap[0x88] = EVENT_TRANSPOSE;
  EventMap[0x89] = EVENT_PROGCHANGE;
  EventMap[0x8a] = EVENT_VOLUME_L;
  EventMap[0x8b] = EVENT_VOLUME_R;
  EventMap[0x8c] = EVENT_GAIN_FADE_ENVELOPE;
  EventMap[0x8d] = EVENT_TUNING;
  EventMap[0x8e] = EVENT_PITCH_FADE_ENVELOPE;
  EventMap[0x8f] = EVENT_PITCH_FADE_ENVELOPE;
  EventMap[0x90] = EVENT_VIBRATO_FADE;
  EventMap[0x91] = EVENT_VIBRATO_OFF;
  EventMap[0x92] = EVENT_DURATION_DIRECT;
  EventMap[0x93] = EVENT_DURATION_SUBTRACT;
  EventMap[0x94] = EVENT_PITCH_FADE;
  EventMap[0x95] = EVENT_PITCH_FADE_OFF;
  EventMap[0x96] = EVENT_VIBRATO;
  EventMap[0x97] = EVENT_GAIN_PRESET;
  EventMap[0x98] = EVENT_NOISE_TOGGLE;
  EventMap[0x99] = EVENT_NOISE_TOGGLE;
  EventMap[0x9a] = EVENT_NOISE_FREQ;
  EventMap[0x9b] = EVENT_SLUR_ON;      
  EventMap[0x9c] = EVENT_SLUR_OFF;      
  EventMap[0x9d] = EVENT_SLUR_OFF_2;    
  EventMap[0x9e] = EVENT_SLUR_ON;       
  EventMap[0x9f] = EVENT_SLUR_OFF;      
  EventMap[0xa0] = EVENT_UNKNOWN0;     
  EventMap[0xa1] = EVENT_LOAD_SONG;
  EventMap[0xa2] = EVENT_GAIN_FADE_MANUALLY;  
  EventMap[0xa3] = EVENT_JUMP;
  EventMap[0xa4] = EVENT_JUMP_A4;
  EventMap[0xa5] = EVENT_UNKNOWN1;      
  EventMap[0xa6] = EVENT_UNKNOWN1;      
  EventMap[0xa7] = EVENT_JUMP;
  EventMap[0xa8] = EVENT_JUMP;
  EventMap[0xa9] = EVENT_JUMP;
  EventMap[0xaa] = EVENT_DISABLE_VOL;
  EventMap[0xab] = EVENT_ECHO_ON;
  EventMap[0xac] = EVENT_ECHO_ON;
  EventMap[0xad] = EVENT_VOLUME_PAN;
  EventMap[0xae] = EVENT_PAN_LFO;
  EventMap[0xaf] = EVENT_UNKNOWN1;
  EventMap[0xb0] = EVENT_MUTE;        
  EventMap[0xb1] = EVENT_UNKNOWN0;    
  EventMap[0xb2] = EVENT_UNKNOWN0;   
  EventMap[0xb3] = EVENT_VOLUME_PAN;  
  EventMap[0xb4] = EVENT_PAN_LFO;
  EventMap[0xb5] = EVENT_UNKNOWN1;    
  EventMap[0xb6] = EVENT_TEMPO;
  EventMap[0xb7] = EVENT_DISABLE_VOL;   
  EventMap[0xb8] = EVENT_UNKNOWN1;      
  EventMap[0xb9] = EVENT_APU_PORTS;     

  EventMap[0xba] = EventMap[0xa9];      
  EventMap[0xbb] = EVENT_UNKNOWN0;      
  EventMap[0xbc] = EVENT_UNKNOWN0;     
  EventMap[0xbd] = EVENT_VOLUME_L;     
  EventMap[0xbe] = EVENT_VOLUME_R;      
  EventMap[0xbf] = EVENT_VETOCITY_ON;
  EventMap[0xc0] = EVENT_VETOCITY_OFF;
  EventMap[0xc1] = EVENT_PERCESSION;   
  EventMap[0xc2] = EVENT_PERCESSION_OFF;  
  EventMap[0xc3] = EVENT_UNKNOWN1;
  EventMap[0xc4] = EVENT_UNKNOWN1;   
  EventMap[0xc5] = EVENT_RESET;
  EventMap[0xc6] = EVENT_UNKNOWN0;

if (version == SOFTCREATSNES_V1 || version == SOFTCREATSNES_V2 || version == SOFTCREATSNES_V3) {
  EventMap[0xa3] = EVENT_UNKNOWN1; 
  EventMap[0xa4] = EVENT_UNKNOWN1; 
  EventMap[0xa7] = EVENT_UNKNOWN1; 
  EventMap[0xa8] = EVENT_UNKNOWN1; 
  EventMap[0xa9] = EVENT_UNKNOWN1;  
  EventMap[0xaa] = EVENT_ECHO_ON;
  EventMap[0xab] = EVENT_ECHO_ON;
  EventMap[0xac] = EVENT_ECHO_VOLUME_LEFT;
  EventMap[0xad] = EVENT_ECHO_VOLUME_RIGHT;
  EventMap[0xae] = EVENT_ECHO_FEEDBACK;
  EventMap[0xaf] = EVENT_ECHO_FIR;
  }

  if (version == SOFTCREATSNES_V5) {
    EventMap[0xb8] = EVENT_END;
    EventMap[0xb9] = EVENT_UNKNOWN1;
  }

  // TODO: SoftCreatSnesSeq::LoadEventMap
}

double SoftCreatSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0 && TIMER2_FREQUENCY != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * TIMER2_FREQUENCY));
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}


//  ******************
//  SoftCreatSnesTrack
//  ******************

SoftCreatSnesTrack::SoftCreatSnesTrack(SoftCreatSnesSeq *parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void SoftCreatSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  subroutineStack = 0;
  subroutineReturnAddress[subroutineStack] = 0;
  loopStack = 0;
  loopCount[loopStack] = 0;
  loopReturnAddress[loopStack] = 0;
  defaultNoteLength = 0;
  duration = 0;
  durationSubtract = 0;
  lastDuration = 0;
  percessionPointer = 0;
  percession = false;
  vetocitySwitch = false;
}

bool SoftCreatSnesTrack::ReadEvent(void) {
  SoftCreatSnesSeq *parentSeq = (SoftCreatSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  SoftCreatSnesSeqEventType eventType = (SoftCreatSnesSeqEventType) 0;
  std::map<uint8_t, SoftCreatSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

    case EVENT_VETOCITY_ON: {
      vetocitySwitch = true;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Velocity On", desc.str(), CLR_EXPRESSION, ICON_CONTROL);
      break;
    }

    case EVENT_VETOCITY_OFF: {
      vetocitySwitch = false;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note Velocity Off", desc.str(), CLR_EXPRESSION, ICON_CONTROL);
      break;
    }

    case EVENT_RESET: {
      AddUnknown(beginOffset, curOffset - beginOffset, L"Reset", desc.str());
      break;
    }

    case EVENT_PERCESSION_OFF: {
      AddUnknown(beginOffset, curOffset - beginOffset, L"Percussion Off", desc.str());
      percession = false;
      break;
    }

    case EVENT_PERCESSION: {
      // Percussion Table:
      // 00 - Program Number, 01 - GAIN Envolope, 02 - Pan, 03 - Note
      uint16_t dest = GetShort(curOffset++);
      curOffset++;
      desc << L"First Note (C-0) Pointer: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Percussion On", desc.str());
      percession = true;
      percessionPointer = dest;
      break;
    }

    case EVENT_TEMPO: {
      uint8_t arg1 = GetByte(curOffset++);
      parentSeq->TIMER2_FREQUENCY = arg1;
      AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(arg1));
      break;
    }

    case EVENT_VOLUME_PAN: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc 
        << L"Left Volume: " << (int)arg1
        << L"  Right Volume: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume L / R", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_PAN_LFO: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Rate: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pan LFO", desc.str(), CLR_PAN, ICON_CONTROL);
      break;
    }

    case EVENT_APU_PORTS: {
      AddUnknown(beginOffset, curOffset - beginOffset, L"Send Current Data to APU", desc.str());
      break;
    }

    case EVENT_DISABLE_VOL: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume Off", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_MUTE: {
      uint8_t arg1 = GetByte(curOffset++);
      desc 
        << L"Duration: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume Fade Out", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_JUMP: {
      uint8_t arg1 = GetByte(curOffset++);
      uint16_t dest = GetShort(curOffset++);
      curOffset++;
      desc 
        << L"Total Jump Times: " << (int)arg1;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Complexed Jump", desc.str());
      if (arg1 != 0) {
        curOffset = dest;
      }
      break;
    }

    case EVENT_JUMP_A4: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)arg1
        << L"  Arg2: " << (int)arg1
        << L"  Arg3: " << (int)arg1;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Complexed Jump (Alt)", desc.str());
      break;
    }

    case EVENT_GAIN_FADE_MANUALLY: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      uint8_t arg4 = GetByte(curOffset++);
      uint8_t arg5 = GetByte(curOffset++);
      uint8_t arg6 = GetByte(curOffset++);
      uint8_t arg7 = GetByte(curOffset++);
      desc
        << L"Duration (FGV): " << (int)arg1
        << L"  FGV: " << (int)arg2
        << L"  Duration (DGV): " << (int)arg3
        << L"  DGV: " << (int)arg4
        << L"  Duration (Delay): " << (int)arg5
        << L"  Delay: " << (int)arg6
        << L"  FGV: " << (int)arg7;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Gain Fade", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_ECHO_FIR: {
      uint8_t newFIR[8];
      GetBytes(curOffset, 8, newFIR);
      curOffset += 8;

      desc << L"Filter: ";
      for (int iFIRIndex = 0; iFIRIndex < 8; iFIRIndex++) {
        if (iFIRIndex != 0)
          desc << L" ";
        desc << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int)newFIR[iFIRIndex];
      }

      AddGenericEvent(beginOffset,
        curOffset - beginOffset,
        L"Echo FIR",
        desc.str().c_str(),
        CLR_REVERB,
        ICON_CONTROL);
      break;
    }

    case EVENT_NOTE: {
      uint8_t key = GetByte(beginOffset);
          if (vetocitySwitch == true && GetByte(beginOffset + 1) <= 0x7f) {
            vel = GetByte(curOffset++);
          }
        else {
          vel = prevVel;
        }
      if (defaultNoteLength == 0) {
        if (duration != 0) {
          dur = duration;
          curOffset++;
        }
        else {
          dur = GetByte(curOffset++);
        }
      }
      else {
        dur = defaultNoteLength;
      }

      if (durationSubtract != 0) {
        uint8_t len = dur - durationSubtract;
        dur = len;
      }

      if (key == 0x00) {
        AddRest(beginOffset, curOffset - beginOffset, dur);
      }
      else {
        if (percession == true) {
          uint16_t currentPercussion = percessionPointer + ((key - 0x12) * 4);
          AddProgramChangeNoItem(GetByte(currentPercussion), true);
          AddPanNoItem(GetByte(currentPercussion + 2));
          key = GetByte(currentPercussion + 3);
          AddNoteByDur(beginOffset, curOffset - beginOffset, key + 1, vel, dur, L"Percussion Note with Duration");
        }
        else {
          AddNoteByDur(beginOffset, curOffset - beginOffset, key + 1, vel, dur);
        }
        AddTime(dur);
      }
      break;
    }

    case EVENT_ECHO_VOLUME_LEFT: {
      uint8_t evol = GetByte(curOffset++);

      desc << L"Echo Volume: " << evol;
      AddGenericEvent(beginOffset,
        curOffset - beginOffset,
        L"Echo Volume Left",
        desc.str().c_str(),
        CLR_REVERB,
        ICON_CONTROL);

      break;
    }

    case EVENT_ECHO_VOLUME_RIGHT: {
      uint8_t evol = GetByte(curOffset++);

      desc << L"Echo Volume: " << evol;
      AddGenericEvent(beginOffset,
        curOffset - beginOffset,
        L"Echo Volume Right",
        desc.str().c_str(),
        CLR_REVERB,
        ICON_CONTROL);

      break;
    }

    case EVENT_ECHO_FEEDBACK: {
      uint8_t param = GetByte(curOffset++);
      desc << L"Feedback: " << param;

      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_NOISE_FREQ: {
      uint8_t param = GetByte(curOffset++);

        uint8_t newNCK = (param >> 2) & 15;
        desc << L"Noise Frequency (NCK): " << newNCK;

      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Param", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
      break;
    }

    case EVENT_GAIN_PRESET: {
      uint8_t arg1 = GetByte(curOffset++);
      desc 
        << L"Preset: " << (int)arg1;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Load GAIN Preset", desc.str());
      break;
    }

    case EVENT_LOAD_SONG: {
      AddUnknown(beginOffset, curOffset - beginOffset, L"Reset Current Song");
      break;
    }

    case EVENT_SLUR_OFF_2: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur On (Keep Current GAIN Value)", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_ECHO_ON: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Toggle", desc.str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_SLUR_OFF: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur Off", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_SLUR_ON: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur On", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_NOISE_TOGGLE: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Toggle", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_FADE_OFF: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Fade Off", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_FADE: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc 
        << L"Arg1: " << (int)arg1
        << L"  Arg2: " << (int)arg2;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Fade", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
      break;
    }

    case EVENT_DURATION_DIRECT: {
      duration = GetByte(curOffset++);
      desc 
        << L"Duration: " << (int)duration;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Duration (Direct)", desc.str(), CLR_DURNOTE, ICON_CONTROL);
      break;
    }

    case EVENT_DURATION_SUBTRACT: {
      durationSubtract = GetByte(curOffset++);
      desc
        << L"Duration: -" << (int)durationSubtract;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Duration (Subtract)", desc.str(), CLR_DURNOTE, ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_FADE_ENVELOPE: {
      // Is this vibrato or pitch fade?
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc
        << L"Delay: " << (int)arg1
        << L"  Rate: " << (int)arg2
        << L"  Depth: " << (int)arg3;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Fade Envelope", desc.str(), CLR_PITCHBEND, ICON_CONTROL);
      break;
    }

    case EVENT_VIBRATO: {
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

    case EVENT_VIBRATO_OFF: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Off", desc.str(), CLR_MODULATION, ICON_CONTROL);
      break;
    }

    case EVENT_VIBRATO_FADE: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Arg1: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Fade", desc.str(), CLR_MODULATION, ICON_CONTROL);
      break;
    }

    case EVENT_GAIN_FADE_ENVELOPE: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc 
        << L"Arg1: " << (int)arg1
        << L"  Arg2: " << (int)arg2
        << L"  Arg3: " << (int)arg3;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"GAIN Envelope", desc.str(), CLR_MISC, ICON_CONTROL);
      break;
    }

    case EVENT_TRANSPOSE: {
      uint8_t arg1 = GetByte(curOffset++);
      AddTranspose(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_TUNING: {
      uint8_t arg1 = GetByte(curOffset++);
      AddFineTuning(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_VOLUME_L: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Left Volume: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume (Left)", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_VOLUME_R: {
      uint8_t arg1 = GetByte(curOffset++);
      desc
        << L"Right Volume: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Volume (Right)", desc.str(), CLR_VOLUME, ICON_CONTROL);
      break;
    }

    case EVENT_DEFAULT_NOTE_LENGTH: {
      defaultNoteLength = GetByte(curOffset++);
      duration = 0;
      //durationSubtract = 0;
      desc 
        << L"Duration: " << (int)defaultNoteLength;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Default Note Length", desc.str(), CLR_DURNOTE, ICON_CONTROL);
      break;
    }

    case EVENT_DEFAULT_NOTE_LENGTH_CLEAR: {
      defaultNoteLength = 0;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Clear Default Note Length", desc.str(), CLR_DURNOTE, ICON_CONTROL);
      break;
    }

    case EVENT_REPEAT_START: {
      loopCount[loopStack] = GetByte(curOffset++);
      desc 
        << L"Loop Count: " << (int)loopCount[loopStack];
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Repeat Start", desc.str().c_str(), CLR_LOOP, ICON_STARTREP);
      loopReturnAddress[loopStack] = curOffset;
      loopStack = (loopStack + 1) % SOFTCREATSNES_LOOP_LEVEL_MAX;
      break;
    }

    case EVENT_REPEAT_END: {
      uint8_t prevLoopStack = (loopStack != 0 ? loopStack : SOFTCREATSNES_LOOP_LEVEL_MAX) - 1;
      if (loopCount[prevLoopStack] == 0)
      {
        bContinue = AddLoopForever(beginOffset, curOffset - beginOffset, L"Repeat Forever");
        break;
      }
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Repeat End", desc.str().c_str(), CLR_LOOP, ICON_ENDREP);
      loopCount[prevLoopStack]--;
        if (loopCount[prevLoopStack] != 0) {
          curOffset = loopReturnAddress[prevLoopStack];;
        }
        else {
          loopStack = prevLoopStack;
        }
      break;
    }

    case EVENT_CALL_SUBROUTINE: {
      uint16_t dest = GetShort(curOffset++);
      curOffset++;
      desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Call Subroutine", desc.str().c_str(), CLR_LOOP, ICON_STARTREP);
      subroutineReturnAddress[subroutineStack] = curOffset;
      subroutineStack = (subroutineStack + 1) % SOFTCREATSNES_SUBROUTINE_LEVEL_MAX;
      curOffset = dest;
      break;
    }

    case EVENT_SUBROUTINE_END: {
      uint8_t prevsubroutineStack = (subroutineStack != 0 ? subroutineStack : SOFTCREATSNES_SUBROUTINE_LEVEL_MAX) - 1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Subroutine End", desc.str().c_str(), CLR_LOOP, ICON_ENDREP);
      curOffset = subroutineReturnAddress[prevsubroutineStack];
      subroutineStack = prevsubroutineStack;
      break;
    }

    case EVENT_GOTO: {
      uint16_t dest = GetShort(curOffset);
      curOffset += 2;
      desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
      uint32_t length = curOffset - beginOffset;

      curOffset = dest;
      if (!IsOffsetUsed(dest)) {
        AddGenericEvent(beginOffset, length, L"Jump", desc.str().c_str(), CLR_LOOPFOREVER);
      }
      else {
        bContinue = AddLoopForever(beginOffset, length, L"Jump");
      }
      break;
    }

    case EVENT_END: {
      AddEndOfTrack(beginOffset, curOffset - beginOffset);
      bContinue = false;
      break;
    }

    case EVENT_PROGCHANGE: {
      uint8_t newProg = GetByte(curOffset++);
      AddProgramChange(beginOffset, curOffset - beginOffset, newProg, true);
      break;
    }

    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
                                    LOG_LEVEL_ERR,
                                    L"SoftCreatSnesSeq"));
      bContinue = false;
      break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

#include "pch.h"
#include "OceanSnesSeq.h"
#include <ScaleConversion.h>

DECLARE_FORMAT(OceanSnes);

//  ****************
//  OceanSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

OceanSnesSeq::OceanSnesSeq(RawFile *file, OceanSnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
    : VGMSeq(OceanSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

OceanSnesSeq::~OceanSnesSeq(void) {
}

void OceanSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool OceanSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader *header = AddHeader(dwOffset, 0);
  uint16_t curOffset = dwOffset;
  if (version == OCEANSNES_NSPC || version == OCEANSNES_NSPC2) {
    for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {

     uint16_t sectionListOffsetPtr = GetShort(curOffset + (trackIndex * 2));
     if (sectionListOffsetPtr != 0x0000) {
      std::wstringstream trackSignName;
      trackSignName << L"Track " << (trackIndex + 1) << L" Pointer";
      header->AddSimpleItem(curOffset + (trackIndex * 2), 2, trackSignName.str());
        sectionPointer[trackIndex] = sectionListOffsetPtr;
        uint16_t currentTrackPointer = GetShort(sectionPointer[trackIndex]);
        OceanSnesTrack *track = new OceanSnesTrack(this, currentTrackPointer);
        aTracks.push_back(track);
      } 
    }
  } else if (version == OCEANSNES_STANDALONE) {
     for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
       sectionPointerLater = curOffset;
     uint16_t sectionListOffsetPtr = GetShort(sectionPointerLater);
     if (sectionListOffsetPtr != 0xffff) {
      std::wstringstream trackSignName;
      trackSignName << L"Track " << (trackIndex + 1) << L" Pointer";
    //  header->AddSimpleItem(curOffset + (trackIndex * 2), 2, trackSignName.str());
        
        uint16_t currentTrackPointer = GetShort(sectionListOffsetPtr + (trackIndex * 2));
        OceanSnesTrack *track = new OceanSnesTrack(this, currentTrackPointer);
        aTracks.push_back(track);
      } 
    }
  }

  return true;
}

bool OceanSnesSeq::GetTrackPointers(void) {
  return true;
}

void OceanSnesSeq::LoadEventMap() {
  int statusByte;
        const uint8_t OceanSNES_VOL_TABLE_STANDARD[16] = {
      0x19, 0x33, 0x4c, 0x66, 0x72, 0x7f, 0x8c, 0x99,
      0xa5, 0xb2, 0xbf, 0xcc, 0xd8, 0xe5, 0xf2, 0xfc,
  };

  const uint8_t OceanSNES_DUR_TABLE_STANDARD[8] = {
      0x33, 0x66, 0x7f, 0x99, 0xb2, 0xcc, 0xe5, 0xfc,
  };

  const uint8_t OceanSNES_PAN_TABLE_STANDARD[21] = {
      0x00, 0x01, 0x03, 0x07, 0x0d, 0x15, 0x1e, 0x29,
      0x34, 0x42, 0x51, 0x5e, 0x67, 0x6e, 0x73, 0x77,
      0x7a, 0x7c, 0x7d, 0x7e, 0x7f,
  };

  // N-SPC VER
  if (version == OCEANSNES_NSPC) {

          if (volumeTable.empty()) {
        volumeTable.assign(std::begin(OceanSNES_VOL_TABLE_STANDARD), std::end(OceanSNES_VOL_TABLE_STANDARD));
      }

      if (durRateTable.empty()) {
        durRateTable.assign(std::begin(OceanSNES_DUR_TABLE_STANDARD), std::end(OceanSNES_DUR_TABLE_STANDARD));
      }

      if (panTable.empty()) {
        panTable.assign(std::begin(OceanSNES_PAN_TABLE_STANDARD), std::end(OceanSNES_PAN_TABLE_STANDARD));
      }
  for (statusByte = 0x01; statusByte < 0x80; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE_PARAM;
  }

  for (statusByte = 0x80; statusByte <= 0xc9; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
      EventMap[0xc8] = EVENT_TIE;
  EventMap[0xc9] = EVENT_REST;
  EventMap[0x00] = EVENT_END;
    EventMap[0xca] = EVENT_PROGCHANGE;
    EventMap[0xcb] = EVENT_PAN;
    EventMap[0xcc] = EVENT_PAN_FADE;
    EventMap[0xcd] = EVENT_VIBRATO_ON;
    EventMap[0xce] = EVENT_VIBRATO_OFF;
    EventMap[0xcf] = EVENT_MASTER_VOLUME;
    EventMap[0xd0] = EVENT_MASTER_VOLUME_FADE;
    EventMap[0xd1] = EVENT_TEMPO;
    EventMap[0xd2] = EVENT_TEMPO_FADE;
    EventMap[0xd3] = EVENT_GLOBAL_TRANSPOSE;
    EventMap[0xd4] = EVENT_TRANSPOSE;
    EventMap[0xd5] = EVENT_TREMOLO_ON;
    EventMap[0xd6] = EVENT_TREMOLO_OFF;
    EventMap[0xd7] = EVENT_VOLUME;
    EventMap[0xd8] = EVENT_VOLUME_FADE;
    EventMap[0xd9] = EVENT_PROGCHANGE;
    EventMap[0xda] = EVENT_VIBRATO_FADE;
    EventMap[0xdb] = EVENT_PITCH_ENVELOPE_TO;
    EventMap[0xdc] = EVENT_PITCH_ENVELOPE_FROM;
    EventMap[0xdd] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xde] = EVENT_TUNING;
    EventMap[0xdf] = EVENT_ECHO_ON;
    EventMap[0xe0] = EVENT_ECHO_OFF;
    EventMap[0xe1] = EVENT_ECHO_PARAM;
    EventMap[0xe2] = EVENT_ECHO_VOLUME_FADE;
    EventMap[0xe3] = EVENT_PITCH_SLIDE;
    EventMap[0xe4] = EVENT_UNKNOWN1;
    EventMap[0xe5] = EVENT_NOP;
    EventMap[0xe6] = EVENT_PROGCHANGE;
    EventMap[0xe7] = EVENT_ADSR_GAIN;
    EventMap[0xe8] = EVENT_INSTRUMENT_EFFECT_ON;
    EventMap[0xe9] = EVENT_INSTRUMENT_EFFECT_OFF;
    EventMap[0xea] = EVENT_UNKNOWN1;
    EventMap[0xeb] = EVENT_UNKNOWN1;
    EventMap[0xec] = EVENT_UNKNOWN1;
    EventMap[0xed] = EVENT_UNKNOWN0;
  } else if (version == OCEANSNES_NSPC2) {
    
          if (volumeTable.empty()) {
        volumeTable.assign(std::begin(OceanSNES_VOL_TABLE_STANDARD), std::end(OceanSNES_VOL_TABLE_STANDARD));
      }

      if (durRateTable.empty()) {
        durRateTable.assign(std::begin(OceanSNES_DUR_TABLE_STANDARD), std::end(OceanSNES_DUR_TABLE_STANDARD));
      }

      if (panTable.empty()) {
        panTable.assign(std::begin(OceanSNES_PAN_TABLE_STANDARD), std::end(OceanSNES_PAN_TABLE_STANDARD));
      }
  for (statusByte = 0x01; statusByte < 0x80; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE_PARAM;
  }

  for (statusByte = 0x80; statusByte <= 0xc7; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
    EventMap[0xc8] = EVENT_TIE;
  EventMap[0xc9] = EVENT_REST;
  EventMap[0x00] = EVENT_END;
    EventMap[0xe0] = EVENT_PROGCHANGE;
    EventMap[0xe1] = EVENT_PAN;
    EventMap[0xe2] = EVENT_PAN_FADE;
    EventMap[0xe3] = EVENT_VIBRATO_ON;
    EventMap[0xe4] = EVENT_VIBRATO_OFF;
    EventMap[0xe5] = EVENT_MASTER_VOLUME;
    EventMap[0xe6] = EVENT_MASTER_VOLUME_FADE;
    EventMap[0xe7] = EVENT_TEMPO;
    EventMap[0xe8] = EVENT_TEMPO_FADE;
    EventMap[0xe9] = EVENT_GLOBAL_TRANSPOSE;
    EventMap[0xea] = EVENT_TRANSPOSE;
    EventMap[0xeb] = EVENT_TREMOLO_ON;
    EventMap[0xec] = EVENT_TREMOLO_OFF;
    EventMap[0xed] = EVENT_VOLUME;
    EventMap[0xee] = EVENT_VOLUME_FADE;
    EventMap[0xef] = EVENT_PROGCHANGE;
    EventMap[0xf0] = EVENT_VIBRATO_FADE;
    EventMap[0xf1] = EVENT_PITCH_ENVELOPE_TO;
    EventMap[0xf2] = EVENT_PITCH_ENVELOPE_FROM;
    EventMap[0xf3] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xf4] = EVENT_TUNING;
    EventMap[0xf5] = EVENT_ECHO_ON;
    EventMap[0xf6] = EVENT_ECHO_OFF;
    EventMap[0xf7] = EVENT_ECHO_PARAM;
    EventMap[0xf8] = EVENT_ECHO_VOLUME_FADE;
    EventMap[0xf9] = EVENT_PITCH_SLIDE;
    EventMap[0xfa] = EVENT_UNKNOWN1;
    EventMap[0xfb] = EVENT_NOP;
    EventMap[0xfc] = EVENT_PROGCHANGE;
    EventMap[0xfd] = EVENT_ADSR_GAIN;
    EventMap[0xfe] = EVENT_INSTRUMENT_EFFECT_ON;
    EventMap[0xff] = EVENT_INSTRUMENT_EFFECT_OFF;
  }
  // STANDALONE VER
  else if (version == OCEANSNES_STANDALONE) {
    for (statusByte = 0x00; statusByte < 0x80; statusByte++) {
    EventMap[statusByte] = EVENT_WAIT;
  }
    for (statusByte = 0xf0; statusByte <= 0xff; statusByte++) {
    EventMap[statusByte] = EVENT_VOL;
  }for (statusByte = 0xea; statusByte < 0xf0; statusByte++) {
    EventMap[statusByte] = EVENT_PROGCHANGE_ONEBYTE;
  }
    for (statusByte = 0x80; statusByte < 0xbe; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }
    EventMap[0xbe] = EVENT_RELEASE_NOTE;
    EventMap[0xbf] = EVENT_NOTE_ON; // ??
    EventMap[0xc0] = EVENT_PROGCHANGE; 
    EventMap[0xc1] = EVENT_PAN;
    EventMap[0xc2] = EVENT_VOLUME_REL;
    EventMap[0xc3] = EVENT_EXPRESSION;
    EventMap[0xc4] = EVENT_UNKNOWN1;
    EventMap[0xc5] = EVENT_UNKNOWN1;
    EventMap[0xc6] = EVENT_UNKNOWN1;
    EventMap[0xc7] = EVENT_END;
    EventMap[0xc8] = EVENT_UNKNOWN1;
    EventMap[0xc9] = EVENT_UNKNOWN0;
    EventMap[0xca] = EVENT_UNKNOWN1;
    EventMap[0xcb] = EVENT_TICK;
    EventMap[0xcc] = EVENT_UNKNOWN1;
    EventMap[0xcd] = EVENT_PORTAMENTO;
    EventMap[0xce] = EVENT_CHANGE_EFFECT;
    EventMap[0xcf] = EVENT_PITCH_FADE;
    EventMap[0xd0] = EVENT_UNKNOWN1;
    EventMap[0xd1] = EVENT_UNKNOWN1;
    EventMap[0xd2] = EVENT_TRACKEND;
    EventMap[0xd3] = EVENT_UNKNOWN1;
    EventMap[0xd4] = EVENT_UNKNOWN1;
    EventMap[0xd5] = EVENT_EFFECT_SPEED;
    EventMap[0xd6] = EVENT_EFFECT_DEPTH;
    EventMap[0xd7] = EVENT_EFFECT_RESET;
    EventMap[0xd8] = EVENT_UNKNOWN1;
    EventMap[0xd9] = EVENT_UNKNOWN1;
    EventMap[0xda] = EVENT_KEY_OFF_TRIGGER_ONOFF;
    EventMap[0xdb] = EVENT_UNKNOWN1;
    EventMap[0xdc] = EVENT_ECHO_FIR;
    EventMap[0xdd] = EVENT_ECHO_VOLUME_L;
    EventMap[0xde] = EVENT_ECHO_VOLUME_R;
    EventMap[0xdf] = EVENT_ECHO_FEEDBACK;
    EventMap[0xe0] = EVENT_ECHO_VOLUME;
    EventMap[0xe1] = EVENT_UNKNOWN1;
    EventMap[0xe2] = EVENT_UNKNOWN1;
    EventMap[0xe3] = EVENT_UNKNOWN1;
    EventMap[0xe4] = EVENT_UNKNOWN1;
    EventMap[0xe5] = EVENT_UNKNOWN1;
    EventMap[0xe6] = EVENT_UNKNOWN1;
    EventMap[0xe7] = EVENT_UNKNOWN0;
    EventMap[0xe8] = EVENT_JUMP;
    EventMap[0xe9] = EVENT_SLUR;
  }
  // TODO: OceanSnesSeq::LoadEventMap
}

uint16_t OceanSnesSeq::ConvertToAPUAddress(uint16_t offset) {
    return offset;
}

uint16_t OceanSnesSeq::GetShortAddress(uint32_t offset) {
  return ConvertToAPUAddress(GetShort(offset));
}

uint16_t OceanSnesSeq::GetShortAddressBE(uint32_t offset) {
  return ConvertToAPUAddress(GetShortBE(offset));
}

double OceanSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa))) * (tempo / 256.0);   // 0xfb?
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

//  ******************
//  OceanSnesTrack
//  ******************

OceanSnesTrack::OceanSnesTrack(OceanSnesSeq *parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void OceanSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

  prevDuration = 0;
  currentSection[0] = 0;
  currentSection[1] = 0;
  currentSection[2] = 0;
  currentSection[3] = 0;
  currentSection[4] = 0;
  currentSection[5] = 0;
  currentSection[6] = 0;
  currentSection[7] = 0;

  loopCount = 0;

  OceanSnesSeq* parentSeq = (OceanSnesSeq*)this->parentSeq;

  currentSectionListPointer[0] = parentSeq->sectionPointer[0];
  currentSectionListPointer[1] = parentSeq->sectionPointer[1];
  currentSectionListPointer[2] = parentSeq->sectionPointer[2];
  currentSectionListPointer[3] = parentSeq->sectionPointer[3];
  currentSectionListPointer[4] = parentSeq->sectionPointer[4];
  currentSectionListPointer[5] = parentSeq->sectionPointer[5];
  currentSectionListPointer[6] = parentSeq->sectionPointer[6];
  currentSectionListPointer[7] = parentSeq->sectionPointer[7];

  spcNoteDurRate = 0xff;
  spcNoteVolume = 0xff;

  prevDuration = 0;

  tick = GetByte(0x59);

  if (parentSeq->version == OCEANSNES_STANDALONE) {
    AddTempoBPMNoItem(60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa))));
  }

  keyOff = false;
  //transpose = parentSeq->channelTranspose[channel];
}

bool OceanSnesTrack::ReadEvent(void) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;


  std::wstringstream desc;

  OceanSnesSeqEventType eventType = (OceanSnesSeqEventType) 0;
  std::map<uint8_t, OceanSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

    case EVENT_INSTRUMENT_EFFECT_ON: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Per-Instrument Special Effect On", desc.str(), CLR_CHANGESTATE);
      break;
    }

case EVENT_INSTRUMENT_EFFECT_OFF: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Per-Instrument Special Effect Off", desc.str(), CLR_CHANGESTATE);
      break;
    }

  case EVENT_ADSR_GAIN: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      uint8_t arg3 = GetByte(curOffset++);
      desc << L"ADSR1: $" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) arg1
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  ADSR2: $" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) arg2
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  GAIN: $" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) arg3
          << std::dec << std::setfill(L' ') << std::setw(0);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR & GAIN", desc.str(), CLR_MISC);
      break;
    }

case EVENT_SLUR: {
      uint8_t arg1 = GetByte(curOffset++);
      desc 
          << L"Arg1: " << (int) arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Slur?", desc.str(), CLR_CHANGESTATE);
      break;
    }

case EVENT_RELEASE_NOTE: {
      AddNoteOffNoItem(prevKey);
      if (GetByte(curOffset) <= 0x7f) {
        prevDuration = GetByte(curOffset++);
      }
      AddTime(prevDuration);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Release Note", desc.str(), CLR_NOTEOFF);
      break;
    }

    case EVENT_TICK: {
      tick = GetByte(curOffset++);
      desc 
          << L"Tick: " << (int) tick;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tick", desc.str(), CLR_TEMPO);
      break;
    }
                       
    case EVENT_NOTE_PARAM: {
      OnEventNoteParam:
      // param #0: duration
      spcNoteDuration = statusByte;
      desc << L"Duration: " << (int) spcNoteDuration;

      // param #1: quantize and velocity (optional)
      if (curOffset + 1 < 0x10000 && GetByte(curOffset) <= 0x7f) {
        uint8_t quantizeAndVelocity = GetByte(curOffset++);

        spcNoteVolume = quantizeAndVelocity;

        desc 
            << L"  Velocity: "  << (int) spcNoteVolume;
      }

      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Note Param",
                      desc.str().c_str(),
                      CLR_DURNOTE,
                      ICON_CONTROL);
      break;
    }

   case EVENT_JUMP: {
        uint16_t dest = GetShort(curOffset);
        curOffset += 2;
        desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int) dest;
        uint32_t length = curOffset - beginOffset;

        if (!IsOffsetUsed(dest)) {
          AddGenericEvent(beginOffset, length, L"Jump", desc.str().c_str(), CLR_LOOPFOREVER);
        }
        else {
          bContinue = AddLoopForever(beginOffset, length, L"Jump");
        }
        curOffset = dest;
        break;
      }

    case EVENT_WAIT: {
      prevDuration = statusByte * tick;
      AddRest(beginOffset, curOffset - beginOffset, prevDuration);
      break;
    }

    case EVENT_NOTE_ON: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Note On (Trigger Key-On)", desc.str().c_str(), CLR_DURNOTE);
      break;
    }

    case EVENT_KEY_OFF_TRIGGER_ONOFF: {
      uint8_t arg1 = GetByte(curOffset++);
      if (arg1 != 0) {
        keyOff = true;
      } else {
        keyOff = false;
      }
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Switch Key-Off for next Note", desc.str().c_str(), CLR_CHANGESTATE);
      break;
    }

    case EVENT_VOL: {
      uint8_t volume = (statusByte & 0x0f) * 0x10;
      if (GetByte(curOffset) <= 0x7f) {
        prevDuration = GetByte(curOffset++);
      }
      AddTime(prevDuration);
      AddVol(beginOffset, curOffset - beginOffset, volume);
      break;
    }

   case EVENT_VOLUME_REL: {
      vol += GetByte(curOffset++);
      if (GetByte(curOffset) <= 0x7f) {
        prevDuration = GetByte(curOffset++);
      }
      AddTime(prevDuration);
      AddVol(beginOffset, curOffset - beginOffset, vol);
      break;
    }

    case EVENT_EXPRESSION: {
      uint8_t vol = GetByte(curOffset++);
      AddExpression(beginOffset, curOffset - beginOffset, vol);
      break;
    }

                         case EVENT_ECHO_FIR: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"FIR: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo FIR",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

   case EVENT_ECHO_VOLUME_L: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Volume: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Volume L",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

  case EVENT_ECHO_FEEDBACK: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Feedback: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Feedback",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

case EVENT_ECHO_DELAY: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Delay: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Delay",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

   case EVENT_ECHO_VOLUME: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Volume L: " << (int) arg1 << L"  Volume R: " << (int) arg2;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Volume",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

   case EVENT_ECHO_VOLUME_R: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Volume: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Volume R",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

case EVENT_EFFECT_SPEED: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Rate: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Effect Rate",
                      desc.str().c_str(),
                      CLR_CHANGESTATE,
                      ICON_CONTROL);
      break;
    }

                       case EVENT_EFFECT_RESET: {
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Reset Effect Rate Counter",
                      desc.str().c_str(),
                      CLR_CHANGESTATE,
                      ICON_CONTROL);
      break;
    }

case EVENT_EFFECT_DEPTH: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"Depth: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Effect Depth?",
                      desc.str().c_str(),
                      CLR_CHANGESTATE,
                      ICON_CONTROL);
      break;
    }

    case EVENT_CHANGE_EFFECT: {
      uint8_t arg1 = GetByte(curOffset++);

      desc << L"ID: " << (int) arg1;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Change Effect",
                      desc.str().c_str(),
                      CLR_CHANGESTATE,
                      ICON_CONTROL);
      break;
    }

    case EVENT_PORTAMENTO: {
      uint8_t arg1 = GetByte(curOffset++);
      uint8_t arg2 = GetByte(curOffset++);
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int) arg1
          << L"  Arg2: " << (int) arg2;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Portamento?", desc.str());
      break;
    }

    case EVENT_NOTE: {
      uint8_t noteNumber = statusByte - 0x80;
      uint8_t duration = (spcNoteDuration * spcNoteDurRate) >> 8;
      if (parentSeq->version != OCEANSNES_STANDALONE) {
        duration = min(max(duration, (uint8_t)1), (uint8_t)(spcNoteDuration - 2));

        // Note: Konami engine can have volume=0
        AddNoteByDur(beginOffset, curOffset - beginOffset, noteNumber, spcNoteVolume / 2, duration,
                     L"Note");
        AddTime(spcNoteDuration);
      } else {
        //duration = prevDuration - keyOff;
        AddNoteOffNoItem(prevKey);
        
        if (GetByte(curOffset) <= 0x7f) {
        prevDuration = GetByte(curOffset++);
      }
      AddTime(prevDuration);
      AddNoteOn(beginOffset, curOffset - beginOffset, noteNumber, spcNoteVolume / 2,
                     L"Note");
      }
      break;
    }

    case EVENT_TIE: {
      uint8_t duration = (spcNoteDuration * spcNoteDurRate) >> 8;
      duration = min(max(duration, (uint8_t) 1), (uint8_t) (spcNoteDuration - 2));
      desc << L"Duration: " << (int) duration;
      MakePrevDurNoteEnd(GetTime() + duration);
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str().c_str(), CLR_TIE);
      AddTime(spcNoteDuration);
      break;
    }

    case EVENT_REST:
      AddRest(beginOffset, curOffset - beginOffset, spcNoteDuration);
      break;


      case EVENT_PROGCHANGE_ONEBYTE: {
      uint8_t newProgNum = statusByte - 0xea;
      AddProgramChange(beginOffset, curOffset - beginOffset, newProgNum, true);
      break;
    }

    case EVENT_PROGCHANGE: {
      uint8_t newProgNum = GetByte(curOffset++);


      AddProgramChange(beginOffset, curOffset - beginOffset, newProgNum, true);
      break;
    }

    case EVENT_TRACKEND: {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
      break;
    }

    case EVENT_PAN: {
      if (parentSeq->version != OCEANSNES_STANDALONE) {
        uint8_t newPan = GetByte(curOffset++);

        double volumeScale;
        bool reverseLeft;
        bool reverseRight;
        int8_t midiPan = CalcPanValue(newPan, volumeScale, reverseLeft, reverseRight);
        AddPan(beginOffset, curOffset - beginOffset, midiPan);
        AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
      } else {
        uint8_t newPan = GetByte(curOffset++);
        AddPan(beginOffset, curOffset - beginOffset, newPan);
      }
      break;
    }

    case EVENT_PAN_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      uint8_t newPan = GetByte(curOffset++);

      double volumeLeft;
      double volumeRight;
      GetVolumeBalance(newPan << 8, volumeLeft, volumeRight);

      uint8_t midiPan = ConvertVolumeBalanceToStdMidiPan(volumeLeft, volumeRight);

      // TODO: fade in real curve
      // TODO: apply volume scale
      AddPanSlide(beginOffset, curOffset - beginOffset, fadeLength, midiPan);
      break;
    }

    case EVENT_VIBRATO_ON: {
      uint8_t vibratoDelay = GetByte(curOffset++);
      uint8_t vibratoRate = GetByte(curOffset++);
      uint8_t vibratoDepth = GetByte(curOffset++);

      desc << L"Delay: " << (int) vibratoDelay << L"  Rate: " << (int) vibratoRate << L"  Depth: "
          << (int) vibratoDepth;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Vibrato",
                      desc.str().c_str(),
                      CLR_MODULATION,
                      ICON_CONTROL);
      break;
    }

    case EVENT_VIBRATO_OFF: {
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Vibrato Off",
                      desc.str().c_str(),
                      CLR_MODULATION,
                      ICON_CONTROL);
      break;
    }

    case EVENT_MASTER_VOLUME: {
      uint8_t newVol = GetByte(curOffset++);
      AddMasterVol(beginOffset, curOffset - beginOffset, newVol / 2);
      break;
    }

    case EVENT_MASTER_VOLUME_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      uint8_t newVol = GetByte(curOffset++);

      desc << L"Length: " << (int) fadeLength << L"  Volume: " << (int) newVol;
      AddMastVolSlide(beginOffset, curOffset - beginOffset, fadeLength, newVol / 2);
      break;
    }

    case EVENT_TEMPO: {
      uint8_t newTempo = GetByte(curOffset++);
      AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
      break;
    }

    case EVENT_TEMPO_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      uint8_t newTempo = GetByte(curOffset++);

      AddTempoSlide(beginOffset,
                    curOffset - beginOffset,
                    fadeLength,
                    (int) (60000000 / parentSeq->GetTempoInBPM(newTempo)));
      break;
    }

    case EVENT_GLOBAL_TRANSPOSE: {
      int8_t semitones = GetByte(curOffset++);
      AddGlobalTranspose(beginOffset, curOffset - beginOffset, semitones);
      break;
    }

    case EVENT_TRANSPOSE: {
      int8_t semitones = GetByte(curOffset++);
      spcTranspose = semitones;
      AddTranspose(beginOffset, curOffset - beginOffset, semitones);
      break;
    }

    case EVENT_TREMOLO_ON: {
      uint8_t tremoloDelay = GetByte(curOffset++);
      uint8_t tremoloRate = GetByte(curOffset++);
      uint8_t tremoloDepth = GetByte(curOffset++);

      desc << L"Delay: " << (int) tremoloDelay << L"  Rate: " << (int) tremoloRate << L"  Depth: "
          << (int) tremoloDepth;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Tremolo",
                      desc.str().c_str(),
                      CLR_MODULATION,
                      ICON_CONTROL);
      break;
    }

    case EVENT_TREMOLO_OFF: {
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Tremolo Off",
                      desc.str().c_str(),
                      CLR_MODULATION,
                      ICON_CONTROL);
      break;
    }

    case EVENT_VOLUME: {
      uint8_t newVol = GetByte(curOffset++);
      AddVol(beginOffset, curOffset - beginOffset, newVol / 2);
      break;
    }

    case EVENT_VOLUME_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      uint8_t newVol = GetByte(curOffset++);
      AddVolSlide(beginOffset, curOffset - beginOffset, fadeLength, newVol / 2);
      break;
    }


    case EVENT_CALL: {
      uint16_t dest = GetShortAddress(curOffset);
      curOffset += 2;
      uint8_t times = GetByte(curOffset++);

      loopReturnAddress = curOffset;
      loopStartAddress = dest;
      loopCount = times;

      desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int) dest
          << std::dec << std::setfill(L' ') << std::setw(0) << L"  Times: " << (int) times;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pattern Play",
                      desc.str().c_str(),
                      CLR_LOOP,
                      ICON_STARTREP);


      curOffset = loopStartAddress;
      break;
    }

    case EVENT_VIBRATO_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      desc << L"Length: " << (int) fadeLength;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Vibrato Fade",
                      desc.str().c_str(),
                      CLR_MODULATION,
                      ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_ENVELOPE_TO: {
      uint8_t pitchEnvDelay = GetByte(curOffset++);
      uint8_t pitchEnvLength = GetByte(curOffset++);
      int8_t pitchEnvSemitones = (int8_t) GetByte(curOffset++);

      desc << L"Delay: " << (int) pitchEnvDelay << L"  Length: " << (int) pitchEnvLength << L"  Semitones: "
          << (int) pitchEnvSemitones;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pitch Envelope (To)",
                      desc.str().c_str(),
                      CLR_PITCHBEND,
                      ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_ENVELOPE_FROM: {
      uint8_t pitchEnvDelay = GetByte(curOffset++);
      uint8_t pitchEnvLength = GetByte(curOffset++);
      int8_t pitchEnvSemitones = (int8_t) GetByte(curOffset++);

      desc << L"Delay: " << (int) pitchEnvDelay << L"  Length: " << (int) pitchEnvLength << L"  Semitones: "
          << (int) pitchEnvSemitones;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pitch Envelope (From)",
                      desc.str().c_str(),
                      CLR_PITCHBEND,
                      ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_ENVELOPE_OFF: {
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pitch Envelope Off",
                      desc.str().c_str(),
                      CLR_PITCHBEND,
                      ICON_CONTROL);
      break;
    }

    case EVENT_TUNING: {
      uint8_t newTuning = GetByte(curOffset++);
      AddFineTuning(beginOffset, curOffset - beginOffset, (newTuning / 256.0) * 100.0);
      break;
    }



    case EVENT_ECHO_ON: {
      uint8_t spcEON = GetByte(curOffset++);
      uint8_t spcEVOL_L = GetByte(curOffset++);
      uint8_t spcEVOL_R = GetByte(curOffset++);

      desc << L"Channels: ";
      for (int channelNo = MAX_TRACKS - 1; channelNo >= 0; channelNo--) {
        if ((spcEON & (1 << channelNo)) != 0) {
          desc << (int) channelNo;
          //parentSeq->aTracks[channelNo]->AddReverbNoItem(40);
        }
        else {
          desc << L"-";
          //parentSeq->aTracks[channelNo]->AddReverbNoItem(0);
        }
      }

      desc << L"  Volume Left: " << (int) spcEVOL_L << L"  Volume Right: " << (int) spcEVOL_R;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_ECHO_OFF: {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Off", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_ECHO_PARAM: {
      uint8_t spcEDL = GetByte(curOffset++);
      uint8_t spcEFB = GetByte(curOffset++);
      uint8_t spcFIR = GetByte(curOffset++);

      desc << L"Delay: " << (int) spcEDL << L"  Feedback: " << (int) spcEFB << L"  FIR: " << (int) spcFIR;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
      break;
    }

    case EVENT_ECHO_VOLUME_FADE: {
      uint8_t fadeLength = GetByte(curOffset++);
      uint8_t spcEVOL_L = GetByte(curOffset++);
      uint8_t spcEVOL_R = GetByte(curOffset++);

      desc << L"Length: " << (int) fadeLength << L"  Volume Left: " << (int) spcEVOL_L << L"  Volume Right: "
          << (int) spcEVOL_R;
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Echo Volume Fade",
                      desc.str().c_str(),
                      CLR_REVERB,
                      ICON_CONTROL);
      break;
    }

    case EVENT_PITCH_SLIDE: {
      uint8_t pitchSlideDelay = GetByte(curOffset++);
      uint8_t pitchSlideLength = GetByte(curOffset++);
      uint8_t pitchSlideTargetNote = GetByte(curOffset++);

      desc << L"Delay: " << (int) pitchSlideDelay << L"  Length: " << (int) pitchSlideLength << L"  Note: "
          << (int) (pitchSlideTargetNote - 0x80);
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pitch Slide",
                      desc.str().c_str(),
                      CLR_PITCHBEND,
                      ICON_CONTROL);
      break;
    }

case EVENT_PITCH_FADE: {
      uint8_t pitchSlideTargetNote = GetByte(curOffset++);
            uint8_t pitchSlideLength = GetByte(curOffset++);

      desc << L"Length: " << (int) pitchSlideLength << L"  Note: "
          << (int) (pitchSlideTargetNote - 0x80);
      AddGenericEvent(beginOffset,
                      curOffset - beginOffset,
                      L"Pitch Slide",
                      desc.str().c_str(),
                      CLR_PITCHBEND,
                      ICON_CONTROL);
      AddTime(pitchSlideLength);
      break;
    }


    case EVENT_END: {
      currentSection[channel]++;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Section End", desc.str().c_str(), CLR_TRACKEND, ICON_CONTROL);
      if (parentSeq->version == OCEANSNES_NSPC || parentSeq->version == OCEANSNES_NSPC2) {
        if (GetShort(parentSeq->sectionPointer[channel] + currentSection[channel] * 2) >= 0x1000 && GetShort(parentSeq->sectionPointer[channel] + currentSection[channel] * 2) <= 0xfff0) {
          curOffset = GetShort(parentSeq->sectionPointer[channel] + currentSection[channel] * 2);
        } else {
          bContinue = false;
        }
      } else {
        if (GetShort(parentSeq->sectionPointerLater + currentSection[channel] * 2) != 0xffff) {
          curOffset = GetShort(GetShort(parentSeq->sectionPointerLater + currentSection[channel] * 2) + channel * 2);
        } else {
          bContinue = false;
        }
      }
      break;
    }


    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
                                    LOG_LEVEL_ERR,
                                    std::wstring(L"OceanSnesSeq")));
      bContinue = false;
      break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint16_t OceanSnesTrack::ConvertToAPUAddress(uint16_t offset) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;
  return parentSeq->ConvertToAPUAddress(offset);
}

uint16_t OceanSnesTrack::GetShortAddress(uint32_t offset) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;
  return parentSeq->GetShortAddress(offset);
}

uint16_t OceanSnesTrack::GetShortAddressBE(uint32_t offset) {
  OceanSnesSeq* parentSeq = (OceanSnesSeq*)this->parentSeq;
  return parentSeq->GetShortAddressBE(offset);
}

void OceanSnesTrack::GetVolumeBalance(uint16_t pan, double &volumeLeft, double &volumeRight) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;

  uint8_t paOceandex = pan >> 8;
    uint8_t panMaxIndex = (uint8_t) (parentSeq->panTable.size() - 1);
    if (paOceandex > panMaxIndex) {
      // unexpected behavior
      pan = panMaxIndex << 8;
      paOceandex = panMaxIndex;
    }

    // actual engine divides pan by 256, though pan value is 7-bit
    // by the way, note that it is right-to-left pan
    volumeRight = ReadPanTable((panMaxIndex << 8) - pan) / 128.0;
    volumeLeft = ReadPanTable(pan) / 128.0;

}

uint8_t OceanSnesTrack::ReadPanTable(uint16_t pan) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;

  uint8_t paOceandex = pan >> 8;
  uint8_t panFraction = pan & 0xff;

  uint8_t panMaxIndex = (uint8_t) (parentSeq->panTable.size() - 1);
  if (paOceandex > panMaxIndex) {
    // unexpected behavior
    paOceandex = panMaxIndex;
    panFraction = 0; // floor(pan)
  }

  uint8_t volumeRate = parentSeq->panTable[paOceandex];

  // linear interpolation for pan fade
  uint8_t nextVolumeRate = (paOceandex < panMaxIndex) ? parentSeq->panTable[paOceandex + 1] : volumeRate;
  uint8_t volumeRateDelta = nextVolumeRate - volumeRate;
  volumeRate += (volumeRateDelta * panFraction) >> 8;

  return volumeRate;
}

int8_t OceanSnesTrack::CalcPanValue(uint8_t pan, double &volumeScale, bool &reverseLeft, bool &reverseRight) {
  OceanSnesSeq *parentSeq = (OceanSnesSeq *) this->parentSeq;

  uint8_t paOceandex;
    paOceandex = pan & 0x1f;
    reverseLeft = (pan & 0x80) != 0;
    reverseRight = (pan & 0x40) != 0;

  double volumeLeft;
  double volumeRight;
  GetVolumeBalance(paOceandex << 8, volumeLeft, volumeRight);

  // TODO: correct volume scale of TOSE sequence
  int8_t midiPan = ConvertVolumeBalanceToStdMidiPan(volumeLeft, volumeRight, &volumeScale);
  volumeScale = min(volumeScale, 1.0); // workaround

  return midiPan;
}

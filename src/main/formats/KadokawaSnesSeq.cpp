#include "pch.h"
#include "KadokawaSnesSeq.h"
#include "ScaleConversion.h"

DECLARE_FORMAT(KadokawaSnes);

//  ****************
//  KadokawaSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48


KadokawaSnesSeq::KadokawaSnesSeq(RawFile *file, KadokawaSnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
: VGMSeq(KadokawaSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
bLoadTickByTick = true;
bAllowDiscontinuousTrackData = true;
bUseLinearAmplitudeScale = true;

UseReverb();
AlwaysWriteInitialReverb(0);

LoadEventMap();
}

KadokawaSnesSeq::~KadokawaSnesSeq(void) {
}

void KadokawaSnesSeq::ResetVars(void) {
VGMSeq::ResetVars();
}

bool KadokawaSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  uint32_t curOffset = dwOffset;
  uint32_t beginOffset = curOffset;
  uint8_t statusByte = GetByte(curOffset++);
  uint32_t songOffset = dwOffset;
  uint8_t activateChannel;
  uint8_t activateChannelInNumber;
  activateChannel = GetByte(0x0);
  activateChannelInNumber = MAX_TRACKS;
  //for (int channelNo = 7; channelNo >= 0; channelNo--) {
  //  if ((activateChannel & (1 << channelNo)) != 0) {
  //    activateChannelInNumber++;
  //  }
  //}

  VGMHeader* header = AddHeader(dwOffset, 0);
  if (version == STINGSNES_SQUARE) {
    header->AddSimpleItem(dwOffset, 2, L"Echo FIR Table Pointer");
  }
    songOffset = GetShort(dwOffset + 0x02);
      header->AddSimpleItem(dwOffset + 0x02, 2, L"Song Pointer");
  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint16_t statusLocation = songOffset + (trackIndex * 2) * 2;
    //if (GetShort(statusLocation) == 0x0000) {
      //header->AddSimpleItem(statusLocation, 2, L"Header End");
      // trackIndex = 9;
    //}
    //else {
    uint16_t trackStatus = GetShort(statusLocation);
    if (trackStatus == 0x0000) {
      //KadokawaSnesTrack* track = new KadokawaSnesTrack(this, statusLocation);   //force stop
      //aTracks.push_back(track);
      break;
    }
    else {
      std::wstringstream statusName;
      statusName << L"Track " << (trackIndex + 1) << " Status / ID";
      header->AddSimpleItem(statusLocation, 2, statusName.str());
      uint16_t sectionListOffsetPtr = songOffset + (trackIndex * 2) + 0x02 + (trackIndex * 2);
      std::wstringstream playlistName;
      playlistName << L"Track " << (trackIndex + 1) << L" Pointer";
      header->AddSimpleItem(sectionListOffsetPtr, 2, playlistName.str());
      uint16_t sectionListAddress = GetShort(sectionListOffsetPtr);
      //assert(sectionListAddress >= dwOffset);
      if (sectionListAddress - dwOffset < nNumTracks * 2) {
        nNumTracks = (sectionListAddress - dwOffset) / 2;
      }
      KadokawaSnesTrack* track = new KadokawaSnesTrack(this, sectionListAddress);
      aTracks.push_back(track);
    }
    }
    /*if (GetShort(sectionListOffsetPtr += 0x02) == 0x0000) {
      header->AddSimpleItem(sectionListOffsetPtr, 2, L"Header End");
      break;
    }*/
    // Will out of range...
  //}
  return true;

}

bool KadokawaSnesSeq::GetTrackPointers(void) {
  return true;
}

void KadokawaSnesSeq::LoadEventMap() {
  const uint8_t NINSNES_VOL_TABLE_STANDARD[16] = {
      0x19, 0x33, 0x4c, 0x66, 0x72, 0x7f, 0x8c, 0x99,
      0xa5, 0xb2, 0xbf, 0xcc, 0xd8, 0xe5, 0xf2, 0xfc,
  };

  const uint8_t NINSNES_DUR_TABLE_STANDARD[8] = {
      0x33, 0x66, 0x7f, 0x99, 0xb2, 0xcc, 0xe5, 0xfc,
  };
  const uint8_t NINSNES_PAN_TABLE_STANDARD[21] = {
    0x00, 0x01, 0x03, 0x07, 0x0d, 0x15, 0x1e, 0x29,
    0x34, 0x42, 0x51, 0x5e, 0x67, 0x6e, 0x73, 0x77,
    0x7a, 0x7c, 0x7d, 0x7e, 0x7f,
  };
  const uint8_t NINSNES_VOL_TABLE_SQUARE[16] = {
    0x0a, 0x19, 0x28, 0x3c, 0x50, 0x64, 0x7d, 0x96,
    0xaa, 0xb9, 0xc8, 0xd4, 0xe1, 0xeb, 0xf5, 0xff,
  };
  const uint8_t NINSNES_DUR_TABLE_SQUARE[8] = {
      0x32, 0x65, 0x7f, 0x98, 0xb2, 0xcb, 0xe5, 0xfc,
  };
  const uint8_t NINSNES_PAN_TABLE_SQUARE[21] = {
  0x00, 0x01, 0x03, 0x07, 0x0d, 0x15, 0x1e, 0x35,
  0x34, 0x42, 0x51, 0x5e, 0x67, 0x6e, 0x73, 0x77,
  0x7a, 0x7c, 0x7d, 0x7e, 0x7f,
  };
  if (version != STINGSNES_SQUARE) {
    for (int statusByte = 0x01; statusByte <= 0x7f; statusByte++) {
      EventMap[statusByte] = EVENT_NOTEDUR;
    }
    for (int statusByte = 0x80; statusByte <= 0xc7; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    for (int statusByte = 0xca; statusByte <= 0xdf; statusByte++) {
      EventMap[statusByte] = EVENT_PERCUSSION_NOTE;
    }
    EventMap[0x00] = EVENT_STOP;
    EventMap[0xc9] = EVENT_REST;
    EventMap[0xc8] = EVENT_TIE;
    EventMap[0xe0] = EVENT_PROGCHANGE;
    EventMap[0xe1] = EVENT_PAN;
    EventMap[0xe2] = EVENT_PAN_FADE;
    EventMap[0xe3] = EVENT_VIBRATO_ON;
    EventMap[0xe4] = EVENT_VIBRATO_OFF;
    EventMap[0xe5] = EVENT_VIBRATO_FADE;
    EventMap[0xe6] = EVENT_DURATION_RATE;
    EventMap[0xe7] = EVENT_CHANNELTEMPO;
    EventMap[0xe8] = EVENT_TEMPO_FADE;
    EventMap[0xe9] = EVENT_GLOBAL_TRANSPOSE;
    EventMap[0xea] = EVENT_TRANSPOSE;
    EventMap[0xed] = EVENT_VOLUME;
    EventMap[0xee] = EVENT_VOLUME_FADE;
    EventMap[0xef] = EVENT_TREMOLO_ON;
    EventMap[0xf0] = EVENT_TREMOLO_OFF;
    EventMap[0xf1] = EVENT_PITCH_ENVELOPE_TO;
    EventMap[0xf2] = EVENT_PITCH_ENVELOPE_FROM;
    EventMap[0xf3] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xf4] = EVENT_TUNING;
    EventMap[0xf5] = EVENT_ECHOPARAM;
    EventMap[0xf7] = EVENT_ECHO_SET;
    EventMap[0xfa] = EVENT_JUMP;
    EventMap[0xfb] = EVENT_CALL;
    EventMap[0xfc] = EVENT_RETURN;
    EventMap[0xfd] = EVENT_LOOPSTART;
    EventMap[0xfe] = EVENT_LOOPEND;
  }
  else {
    EventMap[0x00] = EVENT_STOP;
    for (int statusByte = 0x01; statusByte <= 0xc7; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    EventMap[0xc8] = EVENT_DURATION;
    EventMap[0xca] = EVENT_REST;
    EventMap[0xc9] = EVENT_TIE;
    for (int statusByte = 0xcb; statusByte <= 0xcf; statusByte++) {
      EventMap[statusByte] = EVENT_NOTE;
    }
    EventMap[0xd0] = EVENT_PROGCHANGE;
    EventMap[0xd1] = EVENT_MASTER_VOLUME;
    EventMap[0xd2] = EVENT_ADSR1;
    EventMap[0xd3] = EVENT_ADSR2;
    EventMap[0xd4] = EVENT_RELEASE_GAIN;
    EventMap[0xd5] = EVENT_RESET_ADSR;
    EventMap[0xd6] = EVENT_DURATION_RATE;
    EventMap[0xd7] = EVENT_PAN;
    EventMap[0xd8] = EVENT_PAN_FADE;
    EventMap[0xd9] = EVENT_CHANNELTEMPO;
    EventMap[0xda] = EVENT_TEMPO_FADE;
    EventMap[0xdb] = EVENT_VOLUME;
    EventMap[0xdc] = EVENT_VOLUME_FADE;
    EventMap[0xdd] = EVENT_TREMOLO_ON;
    EventMap[0xde] = EVENT_PAN_LFO;
    EventMap[0xdf] = EVENT_PAN_LFO_OFF;
    EventMap[0xe0] = EVENT_TREMOLO_OFF;
    EventMap[0xe1] = EVENT_GLOBAL_TRANSPOSE;
    EventMap[0xe2] = EVENT_TRANSPOSE;
    EventMap[0xe3] = EVENT_TUNING;
    EventMap[0xe4] = EVENT_VIBRATO_ON;
    EventMap[0xe5] = EVENT_VIBRATO_OFF;
    EventMap[0xe6] = EVENT_PITCH_BASE;
    EventMap[0xe7] = EVENT_PITCH_ENVELOPE_TO;
    EventMap[0xe8] = EVENT_PITCH_ENVELOPE_FROM;
    EventMap[0xe9] = EVENT_PITCH_ENVELOPE_OFF;
    EventMap[0xea] = EVENT_ECHO_DELAY;
    EventMap[0xeb] = EVENT_ECHO_FEEDBACK;
    EventMap[0xec] = EVENT_ECHO_FIR;
    EventMap[0xed] = EVENT_ECHO_VOLUME;
    EventMap[0xee] = EVENT_ECHO_ON;
    EventMap[0xef] = EVENT_ECHO_OFF;
    EventMap[0xf0] = EVENT_NOISE_NOTE;
    EventMap[0xf1] = EVENT_NOISE_FREQ_FADE;
    EventMap[0xf2] = EVENT_NOISE_OFF;
    EventMap[0xf3] = EVENT_PMOD_ON;
    EventMap[0xf4] = EVENT_PMOD_OFF;
    EventMap[0xf5] = EVENT_JUMP;
    EventMap[0xf6] = EVENT_CALL;
    EventMap[0xf7] = EVENT_RETURN;
    EventMap[0xf8] = EVENT_LOOPSTART;
    EventMap[0xf9] = EVENT_LOOPEND;
  }
  if (volumeTable.empty()) {
    volumeTable.assign(std::begin(NINSNES_VOL_TABLE_SQUARE), std::end(NINSNES_VOL_TABLE_SQUARE));
  }

  if (durRateTable.empty()) {
    durRateTable.assign(std::begin(NINSNES_DUR_TABLE_SQUARE), std::end(NINSNES_DUR_TABLE_SQUARE));
  }

  if (panTable.empty()) {
    panTable.assign(std::begin(NINSNES_PAN_TABLE_SQUARE), std::end(NINSNES_PAN_TABLE_SQUARE));
  }
  // TODO: KadokawaSnesSeq::LoadEventMap
}

double KadokawaSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * GetByte(0xfa))) * (tempo / 256.0);
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

uint16_t KadokawaSnesSeq::ConvertToAPUAddress(uint16_t offset) {
  return offset;
}

uint16_t KadokawaSnesSeq::GetShortAddress(uint32_t offset) {
  return ConvertToAPUAddress(GetShort(offset));
}

//  ******************
//  KadokawaSnesTrack
//  ******************

KadokawaSnesTrack::KadokawaSnesTrack(KadokawaSnesSeq *parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void KadokawaSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();
  noteDur = 0;
  loopLevel = 0;
  isInSubroutine = false;
  callLevel = 0;
}

bool KadokawaSnesTrack::ReadEvent(void) {
  KadokawaSnesSeq *parentSeq = (KadokawaSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  KadokawaSnesSeqEventType eventType = (KadokawaSnesSeqEventType) 0;
  std::map<uint8_t, KadokawaSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

  case EVENT_PAN_LFO: {
    uint8_t lfoDepth = GetByte(curOffset++);
    uint8_t lfoRate = GetByte(curOffset++);
    desc << L"Depth: " << (int)lfoDepth << L"  Rate: " << (int)lfoRate;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Pan LFO",
      desc.str().c_str(),
      CLR_MODULATION,
      ICON_CONTROL);
    break;
  }

  case EVENT_PAN_LFO_OFF: {
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Pan LFO Off",
      desc.str().c_str(),
      CLR_MODULATION,
      ICON_CONTROL);
    break;
  }

  case EVENT_DURATION_RATE: {
  //OnEventNoteParam:
    // param #0: duration
   // shared->spcNoteDuration = statusByte;
   // desc << L"Duration: " << (int)shared->spcNoteDuration;

    // param #1: quantize and velocity (optional)
   // if (curOffset + 1 < 0x10000 && GetByte(curOffset) <= 0x7f) {
      uint8_t quantizeAndVelocity = GetByte(curOffset++);

      uint8_t durIndex = (quantizeAndVelocity >> 4) & 7;
      uint8_t velIndex = quantizeAndVelocity & 15;

      spcNoteDurRate = parentSeq->durRateTable[durIndex];
      spcNoteVolume = parentSeq->volumeTable[velIndex];

      desc << L"  Quantize: " << (int)durIndex << L" (" << (int)spcNoteDurRate << L"/256)"
        << L"  Velocity: " << (int)velIndex << L" (" << (int)spcNoteVolume << L"/256)";
   // }

    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Note Param",
      desc.str().c_str(),
      CLR_DURNOTE,
      ICON_CONTROL);
    break;
  }

  case EVENT_NOP: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"NOP (Working if placed after note)", desc.str().c_str(), CLR_MISC);
    break;
  }

  case EVENT_TEMPO_FADE: {
    uint8_t fadeLength = GetByte(curOffset++);
    uint8_t newTempo = GetByte(curOffset++);

    AddTempoSlide(beginOffset,
      curOffset - beginOffset,
      fadeLength,
      (int)(60000000 / parentSeq->GetTempoInBPM(newTempo)));
    break;
  }

  case EVENT_STOP: {
    AddEndOfTrack(beginOffset, 1, L"Stop");
    bContinue = false;
    break;
  }

  case EVENT_RETURN: {
    AddGenericEvent(beginOffset, 1, L"Return", desc.str().c_str(), CLR_TRACKEND, ICON_TRACKEND);
    if (isInSubroutine) {
      callLevel--;
      curOffset = loopReturnAddress[callLevel];
      if (callLevel == 0) {
        isInSubroutine = false;
      }
    }
    break;
  }

  case EVENT_RESET_ADSR: {
    AddGenericEvent(beginOffset, 1, L"Restore ADSR", desc.str().c_str(), CLR_ADSR);
    break;
  }

  case EVENT_CALL: {
    uint16_t dest = GetShortAddress(curOffset++);
    curOffset += 1;

    loopReturnAddress[callLevel] = beginOffset + 0x03;
    loopStartAddress[callLevel] = dest;

    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int) dest;
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Call (One-Time)",
      desc.str().c_str(),
      CLR_LOOP,
      ICON_STARTREP);
    curOffset = dest;
    isInSubroutine = true;
    callLevel++;
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

  case EVENT_GLOBAL_TRANSPOSE: {
    int8_t semitones = GetByte(curOffset++);
    AddGlobalTranspose(beginOffset, curOffset - beginOffset, semitones);
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

  case EVENT_NOISE_FREQ_FADE: {
    uint8_t fadeLength = GetByte(curOffset++);
    uint8_t newPan = GetByte(curOffset++);


    // TODO: fade in real curve
    // TODO: apply volume scale
    AddUnknown(beginOffset, curOffset - beginOffset, L"Noise Freq. Fade");
    break;
  }

  case EVENT_NOISE_OFF: {
    // TODO: fade in real curve
    // TODO: apply volume scale
    AddUnknown(beginOffset, curOffset - beginOffset, L"Noise Off (General)");
    break;
  }

  case EVENT_TRANSPOSE: {
    int8_t semitones = GetByte(curOffset++);
    AddTranspose(beginOffset, curOffset - beginOffset, semitones);
    break;
  }

  case EVENT_MASTER_VOLUME: {
    uint8_t newVol = GetByte(curOffset++);
    AddMasterVol(beginOffset, curOffset - beginOffset, newVol / 2);
    break;
  }

  case EVENT_NOTE_PARAM: {

      uint8_t quantizeAndVelocity = GetByte(curOffset++);

      uint8_t durIndex = (quantizeAndVelocity >> 4) & 7;
      uint8_t velIndex = quantizeAndVelocity & 15;

      //uint8_t spcNoteDurRate = parentSeq->durRateTable[durIndex];
      //uint8_t spcNoteVolume = parentSeq->volumeTable[velIndex];

      desc << L"Quantize: " << (int)durIndex //<< L" (" << (int) spcNoteDurRate << L"/256)"
        << L"  Velocity: " << (int)velIndex; // << L" (" << (int) spcNoteVolume << L"/256)";


    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Note Param",
      desc.str().c_str(),
      CLR_NOTEOFF,
      ICON_CONTROL);
    break;
  }

  case EVENT_VOLUME_FADE: {
    uint8_t fadeLength = GetByte(curOffset++);
    uint8_t newVol = GetByte(curOffset++);
    AddVolSlide(beginOffset, curOffset - beginOffset, fadeLength, newVol / 2);
    break;
  }

  case EVENT_PITCH_ENVELOPE_TO: {
    uint8_t pitchSlideDelay = GetByte(curOffset++);
    uint8_t pitchSlideLength = GetByte(curOffset++);
    uint8_t pitchSlideTargetNote = GetByte(curOffset++);

    desc << L"Delay: " << (int)pitchSlideDelay << L"  Length: " << (int)pitchSlideLength << L"  Note: "
      << (int)(pitchSlideTargetNote - 0x80);
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Pitch Slide",
      desc.str().c_str(),
      CLR_PITCHBEND,
      ICON_CONTROL);
    break;
  }

  case EVENT_PITCH_ENVELOPE_FROM: {
    //uint8_t pitchEnvLength = GetByte(curOffset++);
    //int8_t pitchEnvSemitones = (int8_t) GetByte(curOffset++);

    //desc << L"  Length: " << (int) pitchEnvLength << L"  Semitones: "
    //  << (int) pitchEnvSemitones;
    AddGenericEvent(beginOffset,
      1,
      L"Slur ON",
      desc.str().c_str(),
      CLR_MISC,
      ICON_CONTROL);
    break;
  }

  case EVENT_PITCH_ENVELOPE_OFF: {
    AddGenericEvent(beginOffset,
      curOffset - beginOffset,
      L"Slur OFF",
      desc.str().c_str(),
      CLR_MISC,
      ICON_CONTROL);
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

  case EVENT_ADSR1: {
    uint8_t spcEDL = GetByte(curOffset++);

    desc << L"Arg1: " << (int)spcEDL;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR(1)", desc.str().c_str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_ADSR2: {
    uint8_t spcEDL = GetByte(curOffset++);

    desc << L"Arg1: " << (int)spcEDL;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"ADSR(2)", desc.str().c_str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_RELEASE_GAIN: {
    uint8_t spcEDL = GetByte(curOffset++);

    desc << L"GAIN: " << (int)spcEDL;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Release Rate (GAIN)", desc.str().c_str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_SET: {
    uint8_t spcEDL = GetByte(curOffset++);
    uint8_t spcEFB = GetByte(curOffset++);
    uint8_t spcFIR = GetByte(curOffset++);

    desc << L"Delay: " << (int) spcEDL << L"  Feedback: " << (int) spcEFB << L"  FIR: " << (int) spcFIR;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Param", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
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

  case EVENT_PERCUSSION_NOTE: {
    uint8_t noteNumber = statusByte - 0xca;

    AddPercNoteByDur(beginOffset,
      curOffset - beginOffset,
      noteNumber,
      0x7f,
      noteDur, L"Percussion Note");
    AddTime(noteDur);
    break;
  }

  case EVENT_PITCH_BASE: {
    uint16_t dest = GetShortBE(curOffset);
    curOffset += 2;
    desc << L"Pitch Scale: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int)dest;
    uint32_t length = curOffset - beginOffset;
      AddGenericEvent(beginOffset, length, L"Pitch Scale (N-SPC)", desc.str().c_str(), CLR_PITCHBENDRANGE);
    break;
  }

  case EVENT_JUMP: {
    uint16_t dest = GetShort(curOffset);
    curOffset += 2;
    desc << L"Destination: $" << std::hex << std::setfill(L'0') << std::setw(4) << std::uppercase << (int) dest;
    uint32_t length = curOffset - beginOffset;

    curOffset = dest;
    if (!IsOffsetUsed(dest)) {
      AddGenericEvent(beginOffset, length, L"Goto", desc.str().c_str(), CLR_LOOPFOREVER);
    }
    else {
      bContinue = AddLoopForever(beginOffset, length, L"Loop");
    }
    break;
  }

  case EVENT_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    AddVol(beginOffset, curOffset - beginOffset, arg1 / 2);
    break;
  }

  case EVENT_NOTEDUR: {
noteDur = GetByte(beginOffset);
desc << L"Duration: " << (int)noteDur;
                    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Duration", desc.str(), CLR_DURNOTE, ICON_NOTE);
                    break;
  }

  case EVENT_DURATION: {
    noteDur = GetByte(curOffset++);
    desc << L"Duration: " << (int)noteDur;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Set Duration", desc.str(), CLR_DURNOTE, ICON_NOTE);
    break;
  }

  case EVENT_NOTE: {
    if (parentSeq->version == STINGSNES_SQUARE) {
      if (GetByte(curOffset) == 0xe7) {
        curOffset += 4;
      }
    }
    uint8_t noteID = GetByte(beginOffset);
    uint8_t noteNum = noteID & 0x7f;
    AddNoteByDur(beginOffset, curOffset - beginOffset, noteNum, spcNoteVolume / 2, noteDur, L"Note");
    AddTime(noteDur);
    break;
  }

  case EVENT_NOISE_NOTE: {
      if (GetByte(curOffset) == 0xf1) {
        curOffset += 3;
      }
    AddUnknown(beginOffset, curOffset - beginOffset, L"Noise Note");
    AddTime(noteDur);
    break;
  }

  case EVENT_TIE: {
    if (parentSeq->version == STINGSNES_SQUARE) {
      noteDur = GetByte(curOffset++);
    }
    desc << L"Duration: " << (int)noteDur;
    MakePrevDurNoteEnd(GetTime() + noteDur);
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tie", desc.str(), CLR_TIE, ICON_NOTE);
    AddTime(noteDur);
    break;
  }

  case EVENT_REST: {
    AddRest(beginOffset, curOffset - beginOffset, noteDur);
    break;
  }

  case EVENT_LOOPSTART: {
    uint8_t count = GetByte(curOffset++);
    if (count != 0) {
      count++;
    }
    desc << L"Loop Count: " << (int)count;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str().c_str(), CLR_LOOP, ICON_STARTREP);
    /*loopStart[loopLevel] = beginOffset + 0x02;
    loopIncCount[loopLevel] = 0;
    loopDecCount[loopLevel] = count;
    loopLevel = (loopLevel + 1) & KADOKAWASNES_LOOP_LEVEL_MAX;*/
    break;
  }

  case EVENT_LOOPEND: {
    uint8_t prevLoopLevel = (loopLevel != 0 ? loopLevel : KADOKAWASNES_LOOP_LEVEL_MAX) - 1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", desc.str().c_str(), CLR_LOOP, ICON_ENDREP);

    /*if (loopDecCount[prevLoopLevel] - 1 == 0) {
      // repeat end
      loopLevel = prevLoopLevel;
    }
    else {
      // repeat again
      loopDecCount[prevLoopLevel]--;
      curOffset = loopStart[prevLoopLevel];
    }*/
    break;
  }

  case EVENT_PAN: {
    uint8_t newPan = GetByte(curOffset++);

    double volumeScale;
    bool reverseLeft;
    bool reverseRight;
    int8_t midiPan = CalcPanValue(newPan, volumeScale, reverseLeft, reverseRight);
    AddPan(beginOffset, curOffset - beginOffset, midiPan);
    AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
    break;
  }

  case EVENT_PROGCHANGE: {
    uint8_t newProgNum = GetByte(curOffset++);


    AddProgramChange(beginOffset, curOffset - beginOffset, newProgNum, true);
    break;
  }

  case EVENT_CHANNELTEMPO: {
    uint8_t arg1 = GetByte(curOffset++);
    AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(arg1));
    break;
  }

  case EVENT_ECHO_DELAY: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Echo Delay: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_FEEDBACK: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Echo Feedback: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_FIR: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Echo FIR: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str(), CLR_REVERB, ICON_CONTROL);
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

  case EVENT_PMOD_ON: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation On", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_PMOD_OFF: {
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation Off (General)", desc.str(), CLR_CHANGESTATE, ICON_CONTROL);
    break;
  }

  case EVENT_ECHO_VOLUME: {
    uint8_t arg1 = GetByte(curOffset++);
    desc
      << L"Echo Volume: " << (int)arg1;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume L / R", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_ECHOPARAM: {
    uint8_t spcEON = GetByte(curOffset++);
    uint8_t arg2 = GetByte(curOffset++);
    uint8_t arg3 = GetByte(curOffset++);
    desc
      << L"Channels: ";
      for (int channelNo = MAX_TRACKS - 1; channelNo >= 0; channelNo--) {
      if ((spcEON & (1 << channelNo)) != 0) {
        desc << (int) channelNo;
        parentSeq->aTracks[channelNo]->AddReverbNoItem(40);
      }
      else {
        desc << L"-";
        parentSeq->aTracks[channelNo]->AddReverbNoItem(0);
      }
    } desc
      << L"  Echo Volume L: " << (int) arg2
      << L"  Echo Volume R: " << (int) arg3;
    AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo", desc.str(), CLR_REVERB, ICON_CONTROL);
    break;
  }

  case EVENT_TUNING: {
    uint8_t newTuning = GetByte(curOffset++);
    AddFineTuning(beginOffset, curOffset - beginOffset, (newTuning / 256.0) * 100.0, L"Tuning");
    break;
  }

  default:
    desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
    AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
    pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
      LOG_LEVEL_ERR,
      std::wstring(L"StingSnesSeq")));
    bContinue = false;
    break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint16_t KadokawaSnesTrack::ConvertToAPUAddress(uint16_t offset) {
  KadokawaSnesSeq *parentSeq = (KadokawaSnesSeq *) this->parentSeq;
  return parentSeq->ConvertToAPUAddress(offset);
}

uint16_t KadokawaSnesTrack::GetShortAddress(uint32_t offset) {
  KadokawaSnesSeq *parentSeq = (KadokawaSnesSeq *) this->parentSeq;
  return parentSeq->GetShortAddress(offset);
}


void KadokawaSnesTrack::GetVolumeBalance(uint16_t pan, double& volumeLeft, double& volumeRight) {
  KadokawaSnesSeq* parentSeq = (KadokawaSnesSeq*)this->parentSeq;

  uint8_t panIndex = pan >> 8;
    uint8_t panMaxIndex = (uint8_t)(parentSeq->panTable.size() - 1);
    if (panIndex > panMaxIndex) {
      // unexpected behavior
      pan = panMaxIndex << 8;
      panIndex = panMaxIndex;
    }

    // actual engine divides pan by 256, though pan value is 7-bit
    // by the way, note that it is right-to-left pan
    volumeRight = ReadPanTable((panMaxIndex << 8) - pan) / 128.0;
    volumeLeft = ReadPanTable(pan) / 128.0;
}

uint8_t KadokawaSnesTrack::ReadPanTable(uint16_t pan) {
  KadokawaSnesSeq* parentSeq = (KadokawaSnesSeq*)this->parentSeq;

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

int8_t KadokawaSnesTrack::CalcPanValue(uint8_t pan, double& volumeScale, bool& reverseLeft, bool& reverseRight) {
  KadokawaSnesSeq* parentSeq = (KadokawaSnesSeq*)this->parentSeq;

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

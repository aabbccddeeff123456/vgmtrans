#include "pch.h"
#include "NeverlandSnesSeq.h"

DECLARE_FORMAT(NeverlandSnes);

//  ****************
//  NeverlandSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

NeverlandSnesSeq::NeverlandSnesSeq(RawFile *file, NeverlandSnesVersion ver, uint32_t seqdataOffset)
    : VGMSeq(NeverlandSnesFormat::name, file, seqdataOffset), version(ver) {
  bLoadTickByTick = true;
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);

  LoadEventMap();
}

NeverlandSnesSeq::~NeverlandSnesSeq(void) {
}

void NeverlandSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool NeverlandSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader *header = AddHeader(dwOffset, 0);
  if (version == NEVERLANDSNES_SFC) {
    header->unLength = 0x40;
  }
  else if (version == NEVERLANDSNES_S2C) {
    header->unLength = 0x50;
  }

  if (dwOffset + header->unLength >= 0x10000) {
    return false;
  }

  header->AddSimpleItem(dwOffset, 3, L"Signature");
  header->AddTempo(dwOffset + 3, 1);
  tempoBPM = GetTempoInBPM(GetByte(dwOffset + 3));
  AlwaysWriteInitialTempo(tempoBPM);

  const size_t NAME_SIZE = 12;
  char rawName[NAME_SIZE + 1] = {0};
  GetBytes(dwOffset + 4, NAME_SIZE, rawName);
  header->AddSimpleItem(dwOffset + 4, 12, L"Song Name");

  if (version == NEVERLANDSNES_S2C) {
    header->AddSimpleItem(dwOffset + 0x19, 1, L"Echo Delay");
  }

  // trim name text
  for (int i = NAME_SIZE - 1; i >= 0; i--) {
    if (rawName[i] != ' ') {
      break;
    }
    rawName[i] = '\0';
  }
  // set name to the sequence
  if (rawName[0] != ('\0')) {
    std::string nameStr = std::string(rawName);
    name = string2wstring(nameStr);
  }
  else {
    name = L"NeverlandSnesSeq";
  }

  for (uint8_t trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
    uint16_t trackSignPtr = dwOffset + 0x10 + trackIndex;
    uint8_t trackSign = GetByte(trackSignPtr);
    if (trackSign >= 0x80) {
      percussion[trackIndex] = true;
    }

    std::wstringstream trackSignName;
    trackSignName << L"Track " << (trackIndex + 1) << L" Entry";
    header->AddSimpleItem(trackSignPtr, 1, trackSignName.str());

    uint16_t sectionListOffsetPtr = dwOffset + 0x20 + (trackIndex * 2);
    if (trackSign != 0xff) {
      uint16_t sectionListAddress = GetShortAddress(sectionListOffsetPtr);

      sectionPointer[trackIndex] = sectionListAddress;

      std::wstringstream playlistName;
      playlistName << L"Track " << (trackIndex + 1) << L" Playlist Pointer";
      header->AddSimpleItem(sectionListOffsetPtr, 2, playlistName.str());

      channelTranspose[trackIndex] = 0;

      while (GetByte(sectionPointer[trackIndex]) >= 0x80) {
        channelTranspose[trackIndex] = GetByte(sectionPointer[trackIndex]) - 0x80;
        // 00-3f incrase,40-7f decrase
        if (channelTranspose[trackIndex] >= 0x40) {
          channelTranspose[trackIndex] -= 0x80;
        }
        sectionPointer[trackIndex]++;
      }

      uint16_t currentTrackPointer = GetShortAddressBE(sectionPointer[trackIndex]);

      NeverlandSnesTrack *track = new NeverlandSnesTrack(this, currentTrackPointer);
      aTracks.push_back(track);
      track->spcTranspose = channelTranspose[trackIndex];
//      sectionPointer[trackIndex]++;
//      sectionPointer[trackIndex]++;
    }
    else {
      header->AddSimpleItem(sectionListOffsetPtr, 2, L"NULL");
    }
  }

  return true;
}

bool NeverlandSnesSeq::GetTrackPointers(void) {
  return true;
}

void NeverlandSnesSeq::LoadEventMap() {
  int statusByte;

  for (statusByte = 0x00; statusByte <= 0x7f; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE;
  }

  for (statusByte = 0x80; statusByte <= 0xef; statusByte++) {
    EventMap[statusByte] = EVENT_NOTE_PREV_PARAM;
  }

  EventMap[0xf0] = EVENT_VIBRATO;
  EventMap[0xf1] = EVENT_VOLUME;
  EventMap[0xf2] = EVENT_PAN;
  EventMap[0xf3] = EVENT_WAIT;
  EventMap[0xf4] = EVENT_TEMPO;
  EventMap[0xf5] = EVENT_UNKNOWN0;
  if (version == NEVERLANDSNES_S2C) {
    EventMap[0xf6] = EVENT_PITCH_BEND;
  }
  else {
    EventMap[0xf6] = EVENT_MISC_EVENT;
  }
  EventMap[0xf7] = EVENT_PROGCHANGE;
  EventMap[0xf8] = EVENT_UNKNOWN0;
  EventMap[0xf9] = EVENT_UNKNOWN0;
  EventMap[0xfa] = EVENT_UNKNOWN0;
  EventMap[0xfb] = EVENT_LOOP_START;
  EventMap[0xfc] = EVENT_LOOP_END;
  EventMap[0xfd] = EVENT_SECTION_END;
  EventMap[0xfe] = EVENT_END;
  EventMap[0xff] = EVENT_SUBEVENT;

  SubEventMap[0x00] = SUBEVENT_ECHO_DELAY;
  SubEventMap[0x01] = SUBEVENT_ECHO_FEEDBACK;
  SubEventMap[0x02] = SUBEVENT_ECHO_FIR;
  SubEventMap[0x03] = SUBEVENT_ECHO_ON;
  SubEventMap[0x04] = SUBEVENT_ECHO_OFF;
  SubEventMap[0x05] = SUBEVENT_UNKNOWN1;
  SubEventMap[0x06] = SUBEVENT_UNKNOWN1;
  SubEventMap[0x07] = SUBEVENT_UNKNOWN1;
  SubEventMap[0x08] = SUBEVENT_ADSR_AR;
  SubEventMap[0x09] = SUBEVENT_ADSR_DR;
  SubEventMap[0x0a] = SUBEVENT_ADSR_SL;
  SubEventMap[0x0b] = SUBEVENT_ADSR_SR;
  SubEventMap[0x0c] = SUBEVENT_UNKNOWN1;
  SubEventMap[0x0d] = SUBEVENT_ECHO_VOLUME_LR;
  SubEventMap[0x0e] = SUBEVENT_ECHO_VOLUME_L;
  SubEventMap[0x0f] = SUBEVENT_ECHO_VOLUME_R;
  SubEventMap[0x10] = SUBEVENT_NOISE_FREQ;
  SubEventMap[0x11] = SUBEVENT_NOISE_ON;
  SubEventMap[0x12] = SUBEVENT_NOISE_OFF;
  SubEventMap[0x13] = SUBEVENT_PMOD_ON;
  SubEventMap[0x14] = SUBEVENT_PMOD_OFF;
  SubEventMap[0x15] = SUBEVENT_TREMOLO_DEPTH;
  SubEventMap[0x16] = SUBEVENT_VIBRATO_DEPTH;
  SubEventMap[0x17] = SUBEVENT_TREMOLO_RATE;
  SubEventMap[0x18] = SUBEVENT_VIBRATO_RATE;
  SubEventMap[0x19] = SUBEVENT_SURROUND_L;
  SubEventMap[0x1a] = SUBEVENT_SURROUND_R;
  SubEventMap[0x1b] = SUBEVENT_SURROUND_OFF;
  SubEventMap[0x1c] = SUBEVENT_SURROUND_ECHO_VOLUME_L;
  SubEventMap[0x1d] = SUBEVENT_SURROUND_ECHO_VOLUME_R;
  SubEventMap[0x1e] = SUBEVENT_SURROUND_ECHO_VOLUME_OFF;
  SubEventMap[0x1f] = SUBEVENT_UNKNOWN1;

  // TODO: NeverlandSnesSeq::LoadEventMap
}

uint16_t NeverlandSnesSeq::ConvertToAPUAddress(uint16_t offset) {
  if (version == NEVERLANDSNES_S2C) {
    return dwOffset + offset;
  }
  else {
    return offset;
  }
}

uint16_t NeverlandSnesSeq::GetShortAddress(uint32_t offset) {
  return ConvertToAPUAddress(GetShort(offset));
}

uint16_t NeverlandSnesSeq::GetShortAddressBE(uint32_t offset) {
  return ConvertToAPUAddress(GetShortBE(offset));
}

double NeverlandSnesSeq::GetTempoInBPM(uint8_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * tempo));
  }
  else {
    // since tempo 0 cannot be expressed, this function returns a very small value.
    return 1.0;
  }
}

//  ******************
//  NeverlandSnesTrack
//  ******************

NeverlandSnesTrack::NeverlandSnesTrack(NeverlandSnesSeq *parentFile, long offset, long length)
    : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void NeverlandSnesTrack::ResetVars(void) {
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

  loopCount[0] = 0;
  loopCount[1] = 0;
  loopCount[2] = 0;
  loopCount[3] = 0;
  loopCount[4] = 0;
  loopCount[5] = 0;
  loopCount[6] = 0;
  loopCount[7] = 0;

  loopGroup[0] = 0;
  loopGroup[1] = 0;
  loopGroup[2] = 0;
  loopGroup[3] = 0;
  loopGroup[4] = 0;
  loopGroup[5] = 0;
  loopGroup[6] = 0;
  loopGroup[7] = 0;

  transpose = spcTranspose;

  NeverlandSnesSeq* parentSeq = (NeverlandSnesSeq*)this->parentSeq;

  currentSectionListPointer[0] = parentSeq->sectionPointer[0];
  currentSectionListPointer[1] = parentSeq->sectionPointer[1];
  currentSectionListPointer[2] = parentSeq->sectionPointer[2];
  currentSectionListPointer[3] = parentSeq->sectionPointer[3];
  currentSectionListPointer[4] = parentSeq->sectionPointer[4];
  currentSectionListPointer[5] = parentSeq->sectionPointer[5];
  currentSectionListPointer[6] = parentSeq->sectionPointer[6];
  currentSectionListPointer[7] = parentSeq->sectionPointer[7];

  //transpose = parentSeq->channelTranspose[channel];
}

bool NeverlandSnesTrack::ReadEvent(void) {
  NeverlandSnesSeq *parentSeq = (NeverlandSnesSeq *) this->parentSeq;

  uint32_t beginOffset = curOffset;
  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  std::wstringstream desc;

  NeverlandSnesSeqEventType eventType = (NeverlandSnesSeqEventType) 0;
  std::map<uint8_t, NeverlandSnesSeqEventType>::iterator pEventType = parentSeq->EventMap.find(statusByte);
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

    case EVENT_MISC_EVENT: {
      uint8_t len = GetByte(curOffset++);
      uint8_t newTempohi = GetByte(curOffset++);
      uint8_t newTempolo = GetByte(curOffset++);
      AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, newTempolo, newTempohi / 2);
      AddTime(len);
      break;
    }

    case EVENT_NOTE: {
      uint8_t arg = GetByte(beginOffset);
      prevDuration = GetByte(curOffset++);
      dur = GetByte(curOffset++);
      vel = GetByte(curOffset++);
      if (parentSeq->percussion[channel] == true) {
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, arg, vel, dur, L"Percussion Note with Duration");
      }
      else {
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, arg, vel, dur);
      }
      AddTime(prevDuration);
      break;
    }

    case EVENT_NOTE_PREV_PARAM: {
      uint8_t arg = GetByte(beginOffset) & 0x7f;
      if (parentSeq->percussion[channel] == true) {
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, arg, vel, dur, L"Percussion Note");
      }
      else {
        AddNoteByDur_Extend(beginOffset, curOffset - beginOffset, arg, vel, dur, L"Note");
      }
      AddTime(prevDuration);
      break;
    }

    case EVENT_VIBRATO: {
      uint8_t len = GetByte(curOffset++);
      uint8_t arg1 = GetByte(curOffset++);
      
      desc 
        << L"Arg1: " << (int)arg1;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato", desc.str(), CLR_MODULATION, ICON_CONTROL);
      AddTime(len);
      break;
    }

    case EVENT_VOLUME: {
      uint8_t len = GetByte(curOffset++);
      uint8_t arg1 = GetByte(curOffset++);
      
      AddVol(beginOffset, curOffset - beginOffset, arg1);
      AddTime(len);
      break;
    }

    case EVENT_PAN: {
      uint8_t len = GetByte(curOffset++);
      uint8_t arg1 = GetByte(curOffset++);
      
      AddPan(beginOffset, curOffset - beginOffset, arg1);
      AddTime(len);
      break;
    }

    case EVENT_WAIT: {
      uint8_t arg1 = GetByte(curOffset++);
      AddRest(beginOffset, curOffset - beginOffset, arg1);
      break;
    }

    case EVENT_TEMPO: {
      uint8_t len = GetByte(curOffset++);
      uint8_t newTempo = GetByte(curOffset++);
      
      AddTempoBPM(beginOffset, curOffset - beginOffset, parentSeq->GetTempoInBPM(newTempo));
      AddTime(len);
      break;
    }

    case EVENT_PITCH_BEND: {
      uint8_t len = GetByte(curOffset++);
      uint8_t newTempo = GetByte(curOffset++);
      AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, newTempo / 2);
      AddTime(len);
      break;
    }

    case EVENT_PROGCHANGE: {
      uint8_t len = GetByte(curOffset++);
      uint8_t newTempo = GetByte(curOffset++);
      
      AddProgramChange(beginOffset, curOffset - beginOffset, newTempo);
      AddTime(len);
      break;
    }

    case EVENT_LOOP_START: {
      loopReturnSectionNumber[loopGroup[channel]] = currentSection[channel];
      loopReturnPointer[loopGroup[channel]] = curOffset;
      loopReturnSectionPointer[loopGroup[channel]] = currentSectionListPointer[channel];
      transposeLoop[loopGroup[channel]] = transpose;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Start", desc.str(), CLR_LOOP, ICON_STARTREP);
      loopGroup[channel]++;
      break;
    }

    case EVENT_LOOP_END: {
      uint8_t prevLoopGroup = (loopGroup[channel]) - 1;
      uint8_t curLoopCount = GetByte(curOffset++);
      desc
        << L"Loop Count: " << (int)curLoopCount - 1;
      if (curLoopCount == 0) {
        bContinue = AddLoopForever(beginOffset, curOffset - beginOffset);
      }
      else if (curLoopCount == 1) {
        loopGroup[channel]--;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Marker End", desc.str(), CLR_MARKER, ICON_ENDREP);
      }
      else {
        if (loopCount[prevLoopGroup] == 0) {
          loopCount[prevLoopGroup] = curLoopCount - 1;
          currentSection[channel] = loopReturnSectionNumber[prevLoopGroup];
          curOffset = loopReturnPointer[prevLoopGroup];
          currentSectionListPointer[channel] = loopReturnSectionPointer[prevLoopGroup];
          transpose = transposeLoop[prevLoopGroup];
        }
        else {
          loopCount[prevLoopGroup]--;
          if (loopCount[prevLoopGroup] != 0) {
            currentSection[channel] = loopReturnSectionNumber[prevLoopGroup];
            curOffset = loopReturnPointer[prevLoopGroup];
            currentSectionListPointer[channel] = loopReturnSectionPointer[prevLoopGroup];
            transpose = transposeLoop[prevLoopGroup];
          }
          else {
            loopGroup[channel]--;
          }
        }
        AddGenericEvent(beginOffset, 2, L"Loop End", desc.str(), CLR_LOOP, ICON_ENDREP);
      }
      break;
    }

    case EVENT_SECTION_END: {
      currentSection[channel]++;
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Section End", desc.str().c_str(), CLR_TRACKEND, ICON_CONTROL);
      transpose = 0;
      while (GetByte(currentSectionListPointer[channel] + (currentSection[channel] * 2)) >= 0x80 && GetByte(currentSectionListPointer[channel] + (currentSection[channel] * 2)) != 0xff) {
        transpose = GetByte(currentSectionListPointer[channel] + (currentSection[channel] * 2)) & 0x7f;   // 00-3f incrase,40-7f decrase
        if (transpose >= 0x40) {
          transpose -= 0x80;
        }
        currentSectionListPointer[channel]++;     //??? doesn't work
      }
      if (GetByte(currentSectionListPointer[channel] + (currentSection[channel] * 2)) == 0xff) {
        bContinue = false;
        break;
      }
      else {
        /*desc
          << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)GetShortAddressBE(currentSectionListPointer[channel] + (currentSection[channel] * 2))
        << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)currentSectionListPointer[channel]+(currentSection[channel] * 2)
          << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)currentSection[channel];
        pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
          LOG_LEVEL_ERR,
          std::wstring(L"NeverlandSnesSeq")));*/
        uint16_t nextSectionPointer = (GetShortAddressBE(currentSectionListPointer[channel] + (currentSection[channel] * 2)));
        //while (nextSectionPointer == 0x3148) {
        //  while (GetByte(currentSectionListPointer[channel] + (currentSection[channel] * 2)) >= 0x80) {
        //    transpose = GetByte(currentSectionListPointer[channel]) & 0x80;
        //    currentSectionListPointer[channel]++;     //??? doesn't work
        //  }
        //  nextSectionPointer = (GetShortAddressBE(currentSectionListPointer[channel]));
        //}
        curOffset = nextSectionPointer;
      }
      break;
    }

    case EVENT_END: {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
      break;
    }

    case EVENT_SUBEVENT: {
      uint8_t subStatusByte = GetByte(curOffset++);
      NeverlandSnesSeqSubEventType subEventType = (NeverlandSnesSeqSubEventType)0;
      std::map<uint8_t, NeverlandSnesSeqSubEventType>::iterator
        pSubEventType = parentSeq->SubEventMap.find(subStatusByte);
      if (pSubEventType != parentSeq->SubEventMap.end()) {
        subEventType = pSubEventType->second;
      }

      switch (subEventType) {
      case SUBEVENT_UNKNOWN0:
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        break;

      case SUBEVENT_UNKNOWN1: {
        uint8_t arg1 = GetByte(curOffset++);
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int)arg1;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        break;
      }

      case SUBEVENT_UNKNOWN2: {
        uint8_t arg1 = GetByte(curOffset++);
        uint8_t arg2 = GetByte(curOffset++);
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int)arg1
          << L"  Arg2: " << (int)arg2;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        break;
      }

      case SUBEVENT_UNKNOWN3: {
        uint8_t arg1 = GetByte(curOffset++);
        uint8_t arg2 = GetByte(curOffset++);
        uint8_t arg3 = GetByte(curOffset++);
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int)arg1
          << L"  Arg2: " << (int)arg2
          << L"  Arg3: " << (int)arg3;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        break;
      }

      case SUBEVENT_UNKNOWN4: {
        uint8_t arg1 = GetByte(curOffset++);
        uint8_t arg2 = GetByte(curOffset++);
        uint8_t arg3 = GetByte(curOffset++);
        uint8_t arg4 = GetByte(curOffset++);
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte
          << std::dec << std::setfill(L' ') << std::setw(0)
          << L"  Arg1: " << (int)arg1
          << L"  Arg2: " << (int)arg2
          << L"  Arg3: " << (int)arg3
          << L"  Arg4: " << (int)arg4;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        break;
      }

      case SUBEVENT_ECHO_DELAY: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Delay: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_FEEDBACK: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Feedback: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_FIR: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"FIR: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_ON: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo On", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_OFF: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Off", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ADSR_AR: {
        uint8_t newAR = GetByte(curOffset++) & 15;
        desc << L"AR: " << (int)newAR;
        AddGenericEvent(beginOffset,
          curOffset - beginOffset,
          L"ADSR Attack Rate",
          desc.str().c_str(),
          CLR_ADSR,
          ICON_CONTROL);
        break;
      }

      case SUBEVENT_ADSR_DR: {
        uint8_t newDR = GetByte(curOffset++) & 7;
        desc << L"DR: " << (int)newDR;
        AddGenericEvent(beginOffset,
          curOffset - beginOffset,
          L"ADSR Decay Rate",
          desc.str().c_str(),
          CLR_ADSR,
          ICON_CONTROL);
        break;
      }

      case SUBEVENT_ADSR_SL: {
        uint8_t newSL = GetByte(curOffset++) & 7;
        desc << L"SL: " << (int)newSL;
        AddGenericEvent(beginOffset,
          curOffset - beginOffset,
          L"ADSR Sustain Level",
          desc.str().c_str(),
          CLR_ADSR,
          ICON_CONTROL);
        break;
      }

      case SUBEVENT_ADSR_SR: {
        uint8_t newSR = GetByte(curOffset++) & 15;
        desc << L"SR: " << (int)newSR;
        AddGenericEvent(beginOffset,
          curOffset - beginOffset,
          L"ADSR Sustain Rate",
          desc.str().c_str(),
          CLR_ADSR,
          ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_VOLUME_LR: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Volume L/R: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume L/R", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_VOLUME_L: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Volume Left: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume Left", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_ECHO_VOLUME_R: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Volume Right: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume Right", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_NOISE_FREQ: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Noise Clock: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Frequency", desc.str().c_str(), CLR_MISC, ICON_CONTROL);
        break;
      }

      case SUBEVENT_NOISE_ON: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise On", desc.str().c_str(), CLR_MISC, ICON_CONTROL);
        break;
      }

      case SUBEVENT_NOISE_OFF: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Off", desc.str().c_str(), CLR_MISC, ICON_CONTROL);
        break;
      }

      case SUBEVENT_PMOD_ON: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation On", desc.str().c_str(), CLR_MISC, ICON_CONTROL);
        break;
      }

      case SUBEVENT_PMOD_OFF: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Pitch Modulation Off", desc.str().c_str(), CLR_MISC, ICON_CONTROL);
        break;
      }

      case SUBEVENT_TREMOLO_DEPTH: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Rate: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tremolo Depth", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
        break;
      }

      case SUBEVENT_VIBRATO_DEPTH: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Depth: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Depth", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
        break;
      }

      case SUBEVENT_TREMOLO_RATE: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Rate: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Tremolo Rate", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
        break;
      }

      case SUBEVENT_VIBRATO_RATE: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Rate: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Vibrato Rate", desc.str().c_str(), CLR_MODULATION, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_L: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Left", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_R: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Right", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_OFF: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Off", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_ECHO_VOLUME_L: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Echo Volume Left", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_ECHO_VOLUME_R: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Echo Volume Right", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      case SUBEVENT_SURROUND_ECHO_VOLUME_OFF: {
        uint8_t arg1 = GetByte(curOffset++);
        desc
          << L"Arg1: " << (int)arg1;
        AddGenericEvent(beginOffset, curOffset - beginOffset, L"Surround Echo Volume Off", desc.str().c_str(), CLR_REVERB, ICON_CONTROL);
        break;
      }

      default:
        desc << L"Subevent: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase
          << (int)subStatusByte;
        AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str().c_str());
        pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event - ") + desc.str()).c_str(),
          LOG_LEVEL_ERR,
          L"NeverlandSnesSeq"));
        bContinue = false;
        break;
      }
      break;
    }

    default:
      desc << L"Event: 0x" << std::hex << std::setfill(L'0') << std::setw(2) << std::uppercase << (int) statusByte;
      AddUnknown(beginOffset, curOffset - beginOffset, L"Unknown Event", desc.str());
      pRoot->AddLogItem(new LogItem(std::wstring(L"Unknown Event - ") + desc.str(),
                                    LOG_LEVEL_ERR,
                                    std::wstring(L"NeverlandSnesSeq")));
      bContinue = false;
      break;
  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

uint16_t NeverlandSnesTrack::ConvertToAPUAddress(uint16_t offset) {
  NeverlandSnesSeq *parentSeq = (NeverlandSnesSeq *) this->parentSeq;
  return parentSeq->ConvertToAPUAddress(offset);
}

uint16_t NeverlandSnesTrack::GetShortAddress(uint32_t offset) {
  NeverlandSnesSeq *parentSeq = (NeverlandSnesSeq *) this->parentSeq;
  return parentSeq->GetShortAddress(offset);
}

uint16_t NeverlandSnesTrack::GetShortAddressBE(uint32_t offset) {
  NeverlandSnesSeq* parentSeq = (NeverlandSnesSeq*)this->parentSeq;
  return parentSeq->GetShortAddressBE(offset);
}

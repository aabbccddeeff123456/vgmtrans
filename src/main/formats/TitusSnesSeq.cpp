#include "pch.h"
#include "TitusSnesSeq.h"

DECLARE_FORMAT(TitusSnes);

// NOT WORKING NOW
// DeltaTime == nullptr for some reason

//  ****************
//  TitusSnesSeq
//  ****************
#define MAX_TRACKS  8
#define SEQ_PPQN    48

TitusSnesSeq::TitusSnesSeq(RawFile* file,
  TitusSnesVersion ver,
  uint32_t seqdataOffset,
  std::wstring newName)
  : VGMSeq(TitusSnesFormat::name, file, seqdataOffset, 0, newName), version(ver) {
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  UseReverb();
  AlwaysWriteInitialReverb(0);
}

TitusSnesSeq::~TitusSnesSeq(void) {
}

void TitusSnesSeq::ResetVars(void) {
  VGMSeq::ResetVars();
}

bool TitusSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);

  VGMHeader* header = AddHeader(dwOffset, 0);

  header->AddTempo(dwOffset, 1);
  tempoBPM = GetTempoInBPM(GetByte(dwOffset));
  AlwaysWriteInitialTempo(tempoBPM);
  header->AddSimpleItem(dwOffset + 1, 1, L"Number of Tracks");
  uint8_t maxTracksAllow = GetByte(dwOffset + 1);

  if (version == TITUSSNES_V3_FLAIR) {
    uint16_t programTablePointer = dwOffset + 2 + maxTracksAllow * 2 + 2;
    for (uint8_t trackIndex = 0; trackIndex < maxTracksAllow; trackIndex++) {

      uint32_t addrTrackLowPtr = dwOffset + 2 + (trackIndex * 2);

      std::wstringstream trackName;
      trackName << L"Track Pointer " << (trackIndex + 1);
      header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());

      uint16_t addrTrackStart = GetShort(addrTrackLowPtr);
      if (addrTrackStart != 0x0000) {
        TitusSnesTrack* track = new TitusSnesTrack(this, addrTrackStart);
        AddSimpleItem(programTablePointer, 12, L"Program Table");
        track->spcSRCN = GetByte(programTablePointer++);
        programTablePointer += 2;
        track->initVol = GetByte(programTablePointer++);
        if (track->initVol <= 0x80) {
          track->initVol = 0xff;
        }
        else {
          uint8_t backupItem23 = track->initVol;
          track->initVol = backupItem23 -= 0x81 * 2;
        }
        programTablePointer += 2;
        track->initPan = GetByte(programTablePointer++);
        programTablePointer += 3;
        while (GetByte(programTablePointer) != 0x80) {
          programTablePointer++;
        }
        programTablePointer += 2;
        aTracks.push_back(track);
      }
    }
  }
  else if (version == TITUSSNES_V3) {
    uint16_t programTablePointer = dwOffset + 2 + maxTracksAllow * 4 + 2;
    for (uint8_t trackIndex = 0; trackIndex < maxTracksAllow; trackIndex++) {

      uint32_t addrTrackLowPtr = dwOffset + 2 + (trackIndex * 4);

      std::wstringstream trackName;
      trackName << L"Track Pointer " << (trackIndex + 1);
      header->AddSimpleItem(addrTrackLowPtr, 2, trackName.str());
      std::wstringstream progtblName;
      progtblName << L"Unknown Pointer " << (trackIndex + 1);
      header->AddSimpleItem(addrTrackLowPtr + 2, 2, progtblName.str());

      uint16_t addrTrackStart = GetShort(addrTrackLowPtr);
      if (addrTrackStart != 0x0000) {
        TitusSnesTrack* track = new TitusSnesTrack(this, addrTrackStart);
        AddSimpleItem(programTablePointer, 12, L"Program Table");
        track->spcSRCN = GetByte(programTablePointer++);
        programTablePointer += 2;
        track->initVol = GetByte(programTablePointer++);
        if (track->initVol <= 0x80) {
          track->initVol = 0xff;
        }
        else {
          uint8_t backupItem23 = track->initVol;
          track->initVol = backupItem23 -= 0x81 * 2;
        }
        programTablePointer += 2;
        track->initPan = GetByte(programTablePointer++);
        programTablePointer += 3;
        while (GetByte(programTablePointer) != 0x80) {
          programTablePointer++;
        }
        programTablePointer += 2;
        aTracks.push_back(track);
      }
    }
  }

  return true;
}

bool TitusSnesSeq::GetTrackPointers(void) {

  return true;
}

double TitusSnesSeq::GetTempoInBPM(uint16_t tempo) {
  if (tempo != 0) {
    return 60000000.0 / (SEQ_PPQN * (125 * tempo));
  }
  else {
    return 1.0; // since tempo 0 cannot be expressed, this function returns a very small value.
  }
}

//  ******************
//  TitusSnesTrack
//  ******************

TitusSnesTrack::TitusSnesTrack(TitusSnesSeq* parentFile, long offset, long length)
  : SeqTrack(parentFile, offset, length) {
  ResetVars();
  bDetermineTrackLengthEventByEvent = true;
  bWriteGenericEventAsTextEvent = false;
}

void TitusSnesTrack::ResetVars(void) {
  SeqTrack::ResetVars();

}

void TitusSnesTrack::AddInitialMidiEvents(int trackNum) {
  SeqTrack::AddInitialMidiEvents(trackNum);

 // double volumeScale;
  AddProgramChangeNoItem(spcSRCN, true);
  AddVolNoItem(initVol);
 // AddPanNoItem(Convert7bitLinearPercentPanValToStdMidiVal((uint8_t)(spcPan + 0x80) / 2, &volumeScale));
 // AddExpressionNoItem(ConvertPercentAmpToStdMidiVal(volumeScale));
 // AddReverbNoItem(0);
}

bool TitusSnesTrack::ReadEvent(void) {
  TitusSnesSeq* parentSeq = (TitusSnesSeq*)this->parentSeq;

  uint32_t beginOffset = curOffset;
  AddTime(ReadVarLen(curOffset));

  if (curOffset >= 0x10000) {
    return false;
  }

  uint8_t statusByte = GetByte(curOffset++);
  bool bContinue = true;

  if (statusByte <= 0xfe) {
      uint8_t noteOctave = ((statusByte >> 4) & 0x0f);
      key = (statusByte & 0x0f) + (noteOctave * 0x0c);
      vel = GetByte(curOffset++);
      if (vel > 0) {
        AddNoteOn(beginOffset, curOffset - beginOffset, key, vel);
      }
      else {
        AddNoteOff(beginOffset, curOffset - beginOffset, key);
      }
  }
  else {

    switch (statusByte) {

    case 0xff: {
      bContinue = AddEndOfTrack(beginOffset, curOffset - beginOffset);
      break;
    }

    default:
      pRoot->AddLogItem(new LogItem((std::wstring(L"Unknown Event")),
        LOG_LEVEL_ERR,
        L"TitusSnesSeq"));
      bContinue = false;
      break;
    }

  }

  //std::wostringstream ssTrace;
  //ssTrace << L"" << std::hex << std::setfill(L'0') << std::setw(8) << std::uppercase << beginOffset << L": " << std::setw(2) << (int)statusByte  << L" -> " << std::setw(8) << curOffset << std::endl;
  //OutputDebugString(ssTrace.str().c_str());

  return bContinue;
}

#include "pch.h"
#include "AccoladeSnesSeq.h"
#include "ScaleConversion.h"

DECLARE_FORMAT(AccoladeSnes);

//  ************
//  AccoladeSnesSeq
//  ************
#define MAX_TRACKS  16
#define SEQ_PPQN    48

AccoladeSnesSeq::AccoladeSnesSeq(RawFile* file, AccoladeSnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
  : VGMSeqNoTrks(AccoladeSnesFormat::name, file, seqdataOffset, newName),
  version(ver) {
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  //AlwaysWriteInitialTempo(60000000.0 / (SEQ_PPQN * (125 * 0x85)));
  //AddTimeSigNoItem(4,4,16);
  UseReverb();

}

AccoladeSnesSeq::~AccoladeSnesSeq(void) {
}

void AccoladeSnesSeq::ResetVars(void) {
  VGMSeqNoTrks::ResetVars();
  //Beginning = true;
  bSkipDeltaTime = true;
}

bool AccoladeSnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  nNumTracks = MAX_TRACKS;

  SetEventsOffset(VGMSeq::dwOffset);
  return true;
}

bool AccoladeSnesSeq::ReadEvent(void) {
  uint32_t beginOffset = curOffset;
  uint32_t deltaTime;
  if (bSkipDeltaTime)
    deltaTime = 0;
  else
    deltaTime = ReadVarLen(curOffset);
  AddTime(deltaTime);
  if (curOffset >= rawfile->size())
    return false;

  bSkipDeltaTime = false;
  //}
  //else {
  //  AddTime(0);
  //  Beginning = false;    //Seens missing beginning delta time?
  //}
  uint8_t status_byte = GetByte(curOffset++);
  // Running Status
  if (status_byte <= 0x7F) {
    // some games were ripped to PSF with the EndTrack event missing, so
    //if we read a sequence of four 0 bytes, then just treat that as the end of the track
    if (status_byte == 0 && GetWord(curOffset) == 0) {
      return false;
    }
    status_byte += 0x80;
    //curOffset--;
    bSkipDeltaTime = true;
  }
  else {
    runningStatus = status_byte;
    bSkipDeltaTime = false;
  }
  channel = status_byte & 0x0F;
  SetCurTrack(channel);

  switch (status_byte & 0xF0) {
  case 0x80:
    key = GetByte(curOffset++);
     // curOffset++;
    //vel = GetByte(curOffset++);
    AddNoteOff(beginOffset, curOffset - beginOffset, key);
    break;
  case 0x90: {
    key = GetByte(curOffset++);
    vel = GetByte(curOffset++);
    AddNoteOn(beginOffset, curOffset - beginOffset, key, vel);
    break;
  }

  case 0xA0: {
    uint8_t arg1 = GetByte(curOffset++);
    AddUnknown(beginOffset, curOffset - beginOffset);
    //return false;
    break;
  }

  case 0xB0: {
    uint8_t controlNum = GetByte(curOffset++);
    AddUnknown(beginOffset, curOffset - beginOffset);
      break;
  }
           break;

  case 0xC0: {
    uint8_t progNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, progNum);
  }
           break;

  case 0xE0: {
    uint8_t lo = GetByte(curOffset++);
    AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, lo / 2);
  }
           break;

  case 0xF0: {
        AddEndOfTrack(beginOffset, curOffset - beginOffset);
        return false;
    break;
  }


           //  break;
  default:
    AddUnknown(beginOffset, curOffset - beginOffset);
    return false;
  }
  return true;
}

uint8_t AccoladeSnesSeq::GetDataByte(uint32_t offset) {
  uint8_t dataByte = GetByte(offset);
  if (dataByte & 0x80) {
    bSkipDeltaTime = true;
    dataByte &= 0x7F;
  }
  else
    bSkipDeltaTime = false;
  return dataByte;
}

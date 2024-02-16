#include "pch.h"
#include "RealtimeASnesSeq.h"
#include "ScaleConversion.h"

DECLARE_FORMAT(RealtimeASnes);

//  ************
//  RealtimeASnesSeq
//  ************
#define MAX_TRACKS  16
#define SEQ_PPQN    48

RealtimeASnesSeq::RealtimeASnesSeq(RawFile* file, RealtimeASnesVersion ver, uint32_t seqdataOffset, std::wstring newName)
  : VGMSeqNoTrks(RealtimeASnesFormat::name, file, seqdataOffset, newName),
  version(ver) {
  bAllowDiscontinuousTrackData = true;
  bUseLinearAmplitudeScale = true;

  //AlwaysWriteInitialTempo(60000000.0 / (SEQ_PPQN * (125 * 0x85)));
  //AddTimeSigNoItem(4,4,16);
  UseReverb();

}

RealtimeASnesSeq::~RealtimeASnesSeq(void) {
}

void RealtimeASnesSeq::ResetVars(void) {
  VGMSeqNoTrks::ResetVars();
  Beginning = true;
}

bool RealtimeASnesSeq::GetHeaderInfo(void) {
  SetPPQN(SEQ_PPQN);
  nNumTracks = MAX_TRACKS;

  SetEventsOffset(VGMSeq::dwOffset);
  return true;
}

bool RealtimeASnesSeq::ReadEvent(void) {
  uint32_t beginOffset = curOffset;
  //if (Beginning != true) {
  if (GetByte(beginOffset) == 0xff) {
    AddTime(0);
  }
  else {
    uint32_t delta = ReadVarLen(curOffset);
    AddTime(delta);
  }
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
    status_byte = runningStatus;
    curOffset--;
  }
  else
    runningStatus = status_byte;
  channel = status_byte & 0x0F;
  SetCurTrack(channel);

  switch (status_byte & 0xF0) {
  case 0x80:
    key = GetByte(curOffset++);
    if (version == REALTIMEASNES_C1992_WORDTRIS || version == REALTIMEASNES_ACIDV) {
      curOffset++;
    }
    //vel = GetByte(curOffset++);
    AddNoteOff(beginOffset, curOffset - beginOffset, key);
    break;
  case 0x90: {
    key = GetByte(curOffset++);
    vel = GetByte(curOffset++);
    AddNoteOn(beginOffset, curOffset - beginOffset, key, vel);
  }
           break;

  case 0xB0: {
    uint8_t controlNum = GetByte(curOffset++);
    uint8_t value = GetByte(curOffset++);
    if (controlNum <= 0x1f && controlNum >= 0x18) {
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo FIR", L"", CLR_REVERB);
      break;
    }
    switch (controlNum)
    {

      //master volume
    case 0:
      AddModulationDepthRange(beginOffset, curOffset - beginOffset, value);
      break;

      //vibrato
    case 1:
      AddModulation(beginOffset, curOffset - beginOffset, value);
      break;

    case 0x02:
      AddBreath(beginOffset, curOffset - beginOffset, value);
      break;

    case 0x03:
      AddPortamentoTime(beginOffset, curOffset - beginOffset, value);
      break;

    case 0x04:
      AddUnknown(beginOffset, curOffset - beginOffset);
      break;

    case 0x05:
      AddUnknown(beginOffset, curOffset - beginOffset);
      break;

      //volume
    case 7:
      AddVol(beginOffset, curOffset - beginOffset, value);
      break;

    case 8:
      AddPan(beginOffset, curOffset - beginOffset, value);
      break;

    case 0x09:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Switch", L"", CLR_REVERB);
      break;

      //pan
    case 10:
      AddPan(beginOffset, curOffset - beginOffset, value);
      break;

    case 11:
      AddFineTuning(beginOffset, curOffset - beginOffset, value);
      break;

      //echo volume
    case 0x0c:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Master Volume L", L"", CLR_VOLUME);
      break;

      //echo volume pan
    case 0x0d:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Master Volume R", L"", CLR_VOLUME);
      break;

      //effect
    case 0x0e:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume L", L"", CLR_REVERB);
      break;

    case 0x0f:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Volume R", L"", CLR_REVERB);
      break;

      //feedback
    case 0x11:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Feedback", L"", CLR_REVERB);
      break;

      //echo delay
    case 0x10:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Echo Delay", L"", CLR_REVERB);
      break;

    case 0x12:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Noise Clock", L"", CLR_MISC);
      break;

    case 0x13:
      AddUnknown(beginOffset, curOffset - beginOffset);
      break;

      //infinite loop point
    case 0x14:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop Begin", L"", CLR_LOOP);
      if (VGMSeq::readMode == READMODE_CONVERT_TO_MIDI) {
        //pMidiTrack->AddControllerEvent(channel, controlNum, value);
        pMidiTrack->AddMetaMarker(L"[");
      }
      break;

      //loop forever
    case 0x15:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Loop End", L"", CLR_LOOPFOREVER);
      if (VGMSeq::readMode == READMODE_CONVERT_TO_MIDI) {
        //pMidiTrack->AddControllerEvent(channel, controlNum, value);
        pMidiTrack->AddMetaMarker(L"]");
      }
      break;

    default:
      AddGenericEvent(beginOffset, curOffset - beginOffset, L"Control Event", L"", CLR_UNKNOWN);
      if (VGMSeq::readMode == READMODE_CONVERT_TO_MIDI) {
        pMidiTrack->AddControllerEvent(channel, controlNum, value);
      }
      break;
    }
  }
           break;

  case 0xC0: {
    uint8_t progNum = GetByte(curOffset++);
    AddProgramChange(beginOffset, curOffset - beginOffset, progNum);
  }
           break;

  case 0xE0: {
    uint8_t lo = GetByte(curOffset++);
    AddPitchBendMidiFormat(beginOffset, curOffset - beginOffset, 0, lo);
  }
           break;

  case 0xF0: {
    if (status_byte == 0xFF) {
      switch (GetByte(curOffset++)) {
        //tempo. identical to SMF
      case 0x51: {
        uint32_t microsPerQuarter = GetWordBE(curOffset) & 0x00FFFFFF;    //mask out the hi byte 0x03
        AddTempo(beginOffset, curOffset + 4 - beginOffset, microsPerQuarter / 2.41);
        curOffset += 4;
        break;
      }

      case 0x2F:
        AddEndOfTrack(beginOffset, curOffset - beginOffset);
        return false;

      default:
        AddEndOfTrack(beginOffset, curOffset - beginOffset - 1);
        return false;
      }
    }
    break;
  }

         //  break;
  default:
    AddUnknown(beginOffset, curOffset - beginOffset);
    return false;
  }
  return true;
}

#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "ShuichiSnesFormat.h"

#define SHUICHISNES_LOOP_LEVEL_MAX 16
#define SHUICHISNES_SUBROUTINE_LEVEL_MAX 16

enum ShuichiSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_SEND_TO_APU2,
  EVENT_INFINITE_LOOP_START,
  EVENT_LOOP_FOREVER,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_SUBROUTINE_START,
  EVENT_SUBROUTINE_END,
  EVENT_TRACK_END,
  EVENT_PROGCHANGE,
  EVENT_RELEASE_RATE,
  EVENT_TEMPO,
  EVENT_TRANSPOSE,
  EVENT_TRANSPOSE_REL,
  EVENT_TUNING,
  EVENT_PITCH_FADE,
  EVENT_DURATION_RATE,
  EVENT_VOLUME_PAN,
  EVENT_VOLUME,
  EVENT_VOLUME_REL,
  EVENT_VOLUME_REL_ONE_TIME,
  EVENT_PAN,
  EVENT_PAN_FADE,
  EVENT_PAN_LFO,
  EVENT_ECHO_VOLUME,
  EVENT_ECHO_PARAM,
  EVENT_ECHO_ON,
  EVENT_VIBRATO,
  EVENT_VIBRATO_OFF,
  EVENT_REST,
  EVENT_MUITI_FLAG,
  EVENT_NOTE,
  EVENT_TIE,
  EVENT_GAIN_DURATION_RATE,
};

class ShuichiSnesSeq
  : public VGMSeq {
public:
  ShuichiSnesSeq(RawFile* file,
    ShuichiSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"ASCII / Shuichi SNES Seq");
  virtual ~ShuichiSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);
  std::vector<uint8_t> panTable;

  ShuichiSnesVersion version;
  std::map<uint8_t, ShuichiSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class ShuichiSnesTrack
  : public SeqTrack {
public:
  ShuichiSnesTrack(ShuichiSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t loopGroup;
  uint8_t loopCount[SHUICHISNES_LOOP_LEVEL_MAX];
  uint16_t loopReturnAddress[SHUICHISNES_LOOP_LEVEL_MAX];
  uint8_t subroutineGroup;
  uint16_t subroutineReturnAddress[SHUICHISNES_SUBROUTINE_LEVEL_MAX];
  void GetVolumeBalance(uint16_t pan, double& volumeLeft, double& volumeRight);
  uint8_t ReadPanTable(uint16_t pan);
  int8_t CalcPanValue(uint8_t pan, double& volumeScale, bool& reverseLeft, bool& reverseRight);

  uint8_t prevVolume;
  uint8_t duration;
  bool onetimeVolumeRel;
  bool tieState;
};

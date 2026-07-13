#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "HiroakiYagishitaSnesFormat.h"

enum HiroakiYagishitaSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_TEMPO,
  EVENT_PROGCHANGE,
  EVENT_SRCNCHANGE,
  EVENT_ADSR1CHANGE,
  EVENT_ADSR2CHANGE,
  EVENT_PAN,
  EVENT_PAN_LFO,
  EVENT_VOLUME,
  EVENT_TREMOLO,
  EVENT_ECHO_ON,
  EVENT_ECHO_OFF,
  EVENT_NOISE_ON,
  EVENT_NOISE_OFF,
  EVENT_TUNING,
  EVENT_TIE_STATE,
  EVENT_TIE_STATE_OFF,
  EVENT_VIBRATO,
  EVENT_VIBRATO_OFF,
  EVENT_PITCH_ENVELOPE,
  EVENT_PITCH_ENVELOPE_OFF,
  EVENT_DURATION,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_SUBROUTINE,
  EVENT_SUBROUTINE_END,
  EVENT_SUBROUTINE_2,
  EVENT_SUBROUTINE_END_2,
  EVENT_JUMP,
  EVENT_SFXTRACK_END,
  EVENT_TRACK_END,

  EVENT_TUNING_OFF,

  EVENT_DRUM_KIT_ON,
  EVENT_DRUM_KIT_OFF,
  EVENT_TRANSPOSE,
  EVENT_DURATION_RATE,
  EVENT_PITCH_MODULATION_ON,
  EVENT_PITCH_MODULATION_OFF,
  EVENT_NOISE_ON_NOTSET_NCK,
  EVENT_ECHO_FIR,
  EVENT_ECHO_PARAM,
  EVENT_OCTAVE,
  EVENT_OCTAVE_INSTANT,
  EVENT_LOOP,
  EVENT_SUBROUTINE_START_END,
  EVENT_NOISE_CLOCK,
  EVENT_EXPRESSION_BASE,
  EVENT_EXPRESSION,
  EVENT_PITCH_ENVELOPE_VER2,
  EVENT_VIBRATO_VER2,
};

class HiroakiYagishitaSnesSeq
  : public VGMSeq {
public:
  HiroakiYagishitaSnesSeq(RawFile* file,
    HiroakiYagishitaSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Hiroaki Yagishita SNES Seq");
  virtual ~HiroakiYagishitaSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);

  HiroakiYagishitaSnesVersion version;
  std::map<uint8_t, HiroakiYagishitaSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class HiroakiYagishitaSnesTrack
  : public SeqTrack {
public:
  HiroakiYagishitaSnesTrack(HiroakiYagishitaSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  bool tieState;
  bool tieStateOff;
  uint8_t duration;
  uint8_t noteBase;
  uint8_t durRate;
  uint8_t expressionBase;

  uint8_t loopCount;
  uint16_t loopReturnAddress;
  uint16_t subroutineReturnAddress;
  uint16_t subroutineReturnAddress2;
};

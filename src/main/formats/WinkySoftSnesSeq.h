#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "WinkySoftSnesFormat.h"

#define WINKYSOFTSNES_LOOP_LEVEL_MAX 8

enum WinkySoftSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_PAN_ENVELOPE,
  EVENT_DSP_WRITE,
  EVENT_VIBRATO_RATE,
  EVENT_NOISE_CONFIG,
  EVENT_PITCH_BEND,
  EVENT_ECHO_PARAM,
  EVENT_OPEN_TRACK,
  EVENT_PERCUSSION,
  EVENT_DETUNE,
  EVENT_VIBRATO_DEPTH,
  EVENT_VOLUME,
  EVENT_PAN,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_CALL,
  EVENT_PATTERN_END,
  EVENT_END,
  EVENT_TEMPO,
  EVENT_TRANSPOSE,
  EVENT_PROGCHANGE,
  EVENT_REST,
  EVENT_VETOCITY,
  EVENT_LENGTH,
  EVENT_DURATION,
};

class WinkySoftSnesSeq
  : public VGMSeq {
public:
  WinkySoftSnesSeq(RawFile* file,
    WinkySoftSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Winkysoft SNES Seq");
  virtual ~WinkySoftSnesSeq(void);

  uint16_t tempo;

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  WinkySoftSnesVersion version;
  std::map<uint8_t, WinkySoftSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

};


class WinkySoftSnesTrack
  : public SeqTrack {
public:
  WinkySoftSnesTrack(WinkySoftSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t detuneenvduration;
  uint8_t prevdetuneenvduration;
  uint8_t loopLevel;
  uint8_t loopCount[WINKYSOFTSNES_LOOP_LEVEL_MAX];
  uint16_t loopReturnAddress[WINKYSOFTSNES_LOOP_LEVEL_MAX];
  uint16_t patternReturnAddress;
  uint8_t perviousKey;
  uint8_t noteLength;
  bool percussion;
};

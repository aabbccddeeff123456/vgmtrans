#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "OpusSnesFormat.h"

#define OPUSSNES_LOOP_LEVEL_MAX 8

enum OpusSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_PLAY,
  EVENT_PLAY_RELATIVE,
  EVENT_PAUSE,
  EVENT_END,
  EVENT_PROGCHANGE,
  EVENT_STOP,
  EVENT_STOP_ABSOLUTE,
  EVENT_STOP_PAUSE,
  EVENT_BEND,
  EVENT_BEND_PAUSE,
  EVENT_VOLUME_RANGE,

  EVENT_VIBRATO,
  EVENT_SONG,
  EVENT_VOLUME,
  EVENT_PAN,
  EVENT_ECHO,
  EVENT_PRIORITY,
  EVENT_RESET_STATE,
  EVENT_LEGATO,
  EVENT_TEMPO,
  EVENT_NOP,
  EVENT_HALT,
  EVENT_TRACK,
  EVENT_PLAY_VCMD,
  EVENT_STOP_VCMD,
  EVENT_SET_NOTE,
  EVENT_BEND_VCMD,
  EVENT_FINE_TUNING,
  EVENT_PAUSE_255,
  EVENT_VCMDC1FLAG,
};

class OpusSnesSeq
  : public VGMSeq {
public:
  OpusSnesSeq(RawFile* file,
    OpusSnesVersion ver,
    uint32_t seqdata_offset,
    uint32_t seqpointerdata_offset,
    uint8_t songIndex,
    std::wstring newName = L"SQ & DBOOT SNES Seq");
  virtual ~OpusSnesSeq(void);

  uint16_t tempo;

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  uint8_t currentSong;
  uint32_t songPointer;

  OpusSnesVersion version;
  std::map<uint8_t, OpusSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  double GetTempoInBPM(uint8_t tempo, bool fastTempo);

};


class OpusSnesTrack
  : public SeqTrack {
public:
  OpusSnesTrack(OpusSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t detuneenvduration;
  uint8_t prevdetuneenvduration;
  uint8_t loopLevel;
  uint8_t loopCount[OPUSSNES_LOOP_LEVEL_MAX];
  uint16_t loopReturnAddress[OPUSSNES_LOOP_LEVEL_MAX];
  uint16_t patternReturnAddress;
  uint8_t perviousKey;
  uint8_t noteLength;
  uint8_t prevDur;
  uint8_t vcmdC1_Flag;
  uint8_t preservedCmd;
  bool percussion;
};

#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "NinRD1SnesFormat.h"

#define STATE_NOTE 0
#define STATE_TIE 1
#define STATE_TIE_END 2
#define STATE_VOL 3
#define STATE_PAN 4
#define STATE_PITCHBEND 5
#define STATE_MODULATION 6
#define STATE_MODULATION_RATE 7
#define STATE_MODULATION_DELAY 8
#define STATE_PAN_LFO_SPEED 9
#define STATE_LFO_SPEED 10
#define STATE_LFO_RATE 11
#define STATE_TUNING 12


enum NinRD1SnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_REST,
  EVENT_NOTE_PARAM,
  EVENT_END,
  EVENT_JUMP,
  EVENT_GOTO,
  EVENT_RETURN,
  EVENT_LOOP_UNTIL,
  EVENT_TEMPO,
  EVENT_TRANSPOSE,
  EVENT_PROGCHANGE,
  EVENT_VELOCITY,
  EVENT_PAN,
  EVENT_VIBRATO_DEPTH,
  EVENT_VIBRATO_RATE,
  EVENT_LFO_SPEED,
  EVENT_VIBRATO_DELAY,
  EVENT_LFO_RATE,
  EVENT_PAN_LFO_SPEED,
  EVENT_TUNING,
  EVENT_NOP1,
  EVENT_SUBCOMMAND,
  EVENT_CHORDNOTE,
  EVENT_FINE_TUNING,
};

class NinRD1SnesSeq
  : public VGMSeq {
public:
  NinRD1SnesSeq(RawFile* file,
    NinRD1SnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Nintendo RD1 (Sappy?) SNES Seq");
  virtual ~NinRD1SnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  NinRD1SnesVersion version;
  std::map<uint8_t, NinRD1SnesSeqEventType> EventMap;
  std::vector<uint8_t> NOTE_DUR_TABLE;

  double GetTempoInBPM(uint8_t tempo);

private:
  void LoadEventMap(void);

};


class NinRD1SnesTrack
  : public SeqTrack {
public:
  NinRD1SnesTrack(NinRD1SnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  bool inSubroutine;

  uint8_t len;
  uint16_t returnAddr;
  uint8_t loopCount;
  uint8_t curDuration;
  uint8_t eventState;
  uint8_t state;
};

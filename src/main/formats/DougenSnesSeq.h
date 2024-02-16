#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "DougenSnesFormat.h"

#define DOUGENSNES_LOOP_LEVEL_MAX 2

enum DougenSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_END,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_PROGCHANGE,
  EVENT_TEMPO,
  EVENT_TUNING,
  EVENT_PAN,
  EVENT_ECHO_VOLUME_LR,
};

class DougenSnesSeq
  : public VGMSeq {
public:
  DougenSnesSeq(RawFile* file,
    DougenSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Cube / Dougen SNES Seq");
  virtual ~DougenSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);

  DougenSnesVersion version;
  std::map<uint8_t, DougenSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class DougenSnesTrack
  : public SeqTrack {
public:
  DougenSnesTrack(DougenSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint16_t loopReturnAddress[DOUGENSNES_LOOP_LEVEL_MAX];
  uint8_t loopCount[DOUGENSNES_LOOP_LEVEL_MAX];

  uint8_t loopGroup;
  uint8_t noteDeltaTime;

  bool percussion;
  bool tieStatus;
};

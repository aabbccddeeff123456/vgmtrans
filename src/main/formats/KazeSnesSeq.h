#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "KazeSnesFormat.h"

enum KazeSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_TEMPO,
  EVENT_MASTER_VOLUME_PAN,
  EVENT_MASTER_VOLUME,
  EVENT_REST,
  EVENT_ECHO_DELAY,
  EVENT_ECHO_FEEDBACK,
  EVENT_ECHO_FIR,
  EVENT_ADSR_MODE,
  EVENT_GAIN_ENV_MODE,
  EVENT_TIE_STATE,
  EVENT_NOISE_ON,
  EVENT_NOISE_OFF,
  EVENT_PROGCHANGE_FROMTABLE,
  EVENT_PROGTABLE_WRITE_2,
  EVENT_PROGTABLE_WRITE_3,
  EVENT_ECHO_VOLUME_PAN,
  EVENT_ECHO_VOLUME,
  EVENT_ECHO_ON,
  EVENT_ECHO_OFF,
  EVENT_PAN,
  EVENT_VOLUME,
  EVENT_NOTE_ON_DELAY,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_TRANSPOSE,
  EVENT_SLUR_OFF,
  EVENT_SLUR_ON,
  EVENT_MASTER_VOLUME_FADE,
  EVENT_JUMP,
  EVENT_END,
  EVENT_NOTE,
  EVENT_PATTERN,
  EVENT_PATTERN_END,
};

class KazeSnesSeq
  : public VGMSeq {
public:
  KazeSnesSeq(RawFile* file,
    KazeSnesVersion ver,
    uint32_t seqdata_offset,
    uint16_t header_offset,
    uint16_t songBaseOffset,
    std::wstring newName = L"KaZe SNES Seq");
  virtual ~KazeSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);

  KazeSnesVersion version;
  std::map<uint8_t, KazeSnesSeqEventType> EventMap;

  uint16_t baseAddress;
  uint16_t headerAddr;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class KazeSnesTrack
  : public SeqTrack {
public:
  KazeSnesTrack(KazeSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  bool isTieState;

  uint8_t patternGroup;
  uint16_t patternReturnAddr[4];
  uint8_t loopGroup;
  uint16_t loopReturnAddr[4];
  uint8_t loopCount[4];
};

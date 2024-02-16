#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "GremlinSnesFormat.h"

enum GremlinSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_REST,
  EVENT_PAN,
  EVENT_END,
};

class GremlinSnesSeq
  : public VGMSeq {
public:
  GremlinSnesSeq(RawFile* file,
    GremlinSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Gremlin SNES Seq");
  virtual ~GremlinSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  uint16_t nNumSections;
  uint16_t currentSectionPointer[8];

  GremlinSnesVersion version;
  std::map<uint8_t, GremlinSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class GremlinSnesTrack
  : public SeqTrack {
public:
  GremlinSnesTrack(GremlinSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t currentSection;
};

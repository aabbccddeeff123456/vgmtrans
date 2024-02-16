#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "MasumiSnesFormat.h"

enum MasumiSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_TEMPO,
  EVENT_PROGCHANGE,
};

class MasumiSnesSeq
  : public VGMSeq {
public:
  MasumiSnesSeq(RawFile* file,
    MasumiSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Cube / Masumi SNES Seq");
  virtual ~MasumiSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);

  MasumiSnesVersion version;
  std::map<uint8_t, MasumiSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class MasumiSnesTrack
  : public SeqTrack {
public:
  MasumiSnesTrack(MasumiSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);
};

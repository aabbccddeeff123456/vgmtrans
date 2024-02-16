#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "RainbowArtSnesFormat.h"

enum RainbowArtSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_PITCH_BEND,
  EVENT_END,
  EVENT_PITCH_BEND_INSTANT,
  EVENT_PAN,
  EVENT_VOLUME,
  EVENT_PROGCHANGE,
  EVENT_REST,
  EVENT_TIE,
  EVENT_NOTE,
};

class RainbowArtSnesSeq
  : public VGMSeq {
public:
  RainbowArtSnesSeq(RawFile* file,
    RainbowArtSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Rainbow Art SNES Seq");
  virtual ~RainbowArtSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  RainbowArtSnesVersion version;
  std::map<uint8_t, RainbowArtSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
  uint16_t baseAddress;
  uint16_t instrumentTablePointer[8];
  uint16_t programTablePointer[8];
  uint16_t sectionTableAddress[8];
};


class RainbowArtSnesTrack
  : public SeqTrack {
public:
  RainbowArtSnesTrack(RainbowArtSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t prevDuration;
};

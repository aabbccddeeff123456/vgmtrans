#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "DaviddeSnesFormat.h"

enum DaviddeSnesSeqEventType {
  // start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get
  // confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_REST,
  EVENT_PAN,
  EVENT_END,
};

class DaviddeSnesSeq : public VGMSeq {
public:
  DaviddeSnesSeq(RawFile* file, DaviddeSnesVersion ver, uint32_t seqdata_offset,
                 std::wstring newName = L"David de Gruttola SNES Seq");
  virtual ~DaviddeSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  uint16_t nNumSections;
  uint16_t currentSectionPointer[8];

  DaviddeSnesVersion version;
  std::map<uint8_t, DaviddeSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};

class DaviddeSnesTrack : public SeqTrack {
public:
  DaviddeSnesTrack(DaviddeSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t currentSection;
  bool restIsOn;
  uint32_t delta;
  uint8_t noteNumber;
};

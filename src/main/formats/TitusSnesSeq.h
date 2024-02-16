#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "TitusSnesFormat.h"

enum TitusSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,

  EVENT_NOTE,
  EVENT_NOTE_OFF,
  EVENT_SILENCE,
  EVENT_LOOP,
};

class TitusSnesSeq
  : public VGMSeq {
public:
  TitusSnesSeq(RawFile* file,
    TitusSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Titus SNES Seq");
  virtual ~TitusSnesSeq(void);

  uint16_t tempo;

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  TitusSnesVersion version;
  std::map<uint8_t, TitusSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

  double GetTempoInBPM(uint8_t tempo, bool fastTempo);

};


class TitusSnesTrack
  : public SeqTrack {
public:
  TitusSnesTrack(TitusSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual void AddInitialMidiEvents(int trackNum);
  virtual bool ReadEvent(void);

  uint8_t spcSRCN;
  uint8_t initVol;
  uint8_t initPan;
};

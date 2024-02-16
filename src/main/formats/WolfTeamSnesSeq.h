#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "WolfTeamSnesFormat.h"

class TriAcePS1ScorePattern;

enum WolfTeamSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_END,
  EVENT_DELAY,
  EVENT_PROGCHANGE,
  EVENT_NOTE,
  EVENT_RETURN,
  EVENT_MOD,
  EVENT_REVERB,
  EVENT_VOLUME,
  EVENT_VOLUME2,
  EVENT_PAN,
  EVENT_LOOPSTART,
  EVENT_ECHOSTATUS,
  EVENT_LOOPEND,
  EVENT_VIBRATO,
  EVENT_TEMPO,
  EVENT_PITCHBEND,
  EVENT_ADSR,
  EVENT_VIBRATO_STATUS,
  EVENT_DETUNE,
};

class WolfTeamSnesSeq
  : public VGMSeq {
public:
  typedef struct _TrkInfo {
    uint16_t unknown1;
    uint16_t unknown2;
    uint16_t trkOffset;
  } TrkInfo;
  WolfTeamSnesSeq(RawFile *file, WolfTeamSnesVersion ver, uint32_t seqdataOffset, std::wstring newName = L"Tri-Ace(Wolf Team) SNES Seq");
  virtual ~WolfTeamSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  WolfTeamSnesVersion version;
  std::map<uint8_t, WolfTeamSnesSeqEventType> EventMap;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
  double GetTempoInBPM(uint16_t tempo);

  VGMHeader* header;
  TrkInfo TrkInfos[32];
  uint16_t trackPointerAddrLynar2[8];
  uint16_t musicBaseAddr;
  uint16_t trackSectionPointer[16];
  uint8_t initialTempoBPM;

private:
  void LoadEventMap(void);
};


class WolfTeamSnesTrack
  : public SeqTrack {
public:
  WolfTeamSnesTrack(WolfTeamSnesSeq *parentFile, long offset = 0, long length = 0);
  //virtual void AddEvent(SeqEvent* pSeqEvent);
  virtual bool ReadEvent(void);
  virtual void ResetVars(void);
  //virtual void LoadTrackMainLoop(uint32_t stopOffset, int32_t stopTime);
  //uint32_t ReadScorePattern(uint32_t offset);
  virtual bool IsOffsetUsed(uint32_t offset);

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);

  uint16_t currentPattern;
  uint8_t loopGroup;
  uint8_t loopCount[2];
  uint16_t loopReturnAddr[2];
  uint16_t loopReturnPattern[2];
  uint8_t currentChannel;
  uint8_t impliedNoteDur;
  uint8_t impliedVelocity;
};


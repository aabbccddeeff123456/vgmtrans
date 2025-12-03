#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "DSEFormat.h"

enum DSESeqEventType {
  // start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get
  // confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_UNKNOWN5,
  EVENT_NOTE,
  EVENT_PAUSE,
  EVENT_REPEAT_LAST_PAUSE,
  EVENT_ADD_TO_LAST_PAUSE,
  EVENT_PAUSE8,
  EVENT_PAUSE16,
  EVENT_PAUSE24,
  EVENT_PAUSE_UNTIL_REL,
  EVENT_VOLUME,
  EVENT_PAN,
  EVENT_TEMPO,
  EVENT_END,
  EVENT_LOOP_POINT,
  EVENT_OCTAVE,
  EVENT_OCTAVE_REL,
  EVENT_NOP,
  EVENT_NOP2,
  EVENT_PITCH_BEND,
  EVENT_EXPRESSION,
  EVENT_PROGRAM_CHANGE,
  EVENT_MODULATION,
  EVENT_SLUR,
  EVENT_PITCH_SLIDE,
  EVENT_VOLUME_FADE,
  EVENT_PAN_FADE,
};

class DSESeq : public VGMSeq {
public:
  DSESeq(RawFile *file, DSEVersion ver, uint32_t seqdataOffset,
              std::wstring newName = L"DSE Seq");
  virtual ~DSESeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM(uint8_t tempo);

  DSEVersion version;
  std::map<uint8_t, DSESeqEventType> EventMap;

  uint16_t TrackStartAddress[10];
  std::vector<uint16_t> InstrumentAddresses;
  std::vector<uint8_t> NOTE_DUR_TABLE;

  uint8_t spcTempo;
  bool fastTempo;

private:
  void LoadEventMap(void);
};

class DSETrack : public SeqTrack {
public:
  DSETrack(DSESeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  void ParseInstrument(uint16_t instrAddress, uint8_t instrNum);
  void ParseInstrumentEvents(uint16_t offset, uint8_t instrNum, bool percussion = false,
                             uint8_t percNoteKey = 0);

private:
  std::list<int8_t> tiedNoteKeys;

  uint8_t spcDeltaTime;
  int8_t spcNoteNumberBase;
  uint32_t spcNoteDuration; // there no uint24_t
  uint8_t spcNoteVelocity;
  uint8_t spcVolume;
  int8_t spcTranspose;
  uint8_t spcTuning;

};

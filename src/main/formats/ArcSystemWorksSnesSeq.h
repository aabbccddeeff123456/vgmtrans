#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "ArcSystemWorksSnesFormat.h"

#define ARCSYSTEMWORKS_SNES_REPEAT_SLOT_MAX 4

enum ArcSystemWorkSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_PERCUSSION_NOTE,
  EVENT_NOTE_PARAM,
  EVENT_TEMPO,
  EVENT_PITCH_LFO,
  EVENT_TUNING,
  EVENT_ADSR,
  EVENT_JUMP,
  EVENT_TIE,
  EVENT_SUBROUTINE_END,
  EVENT_SUBROUTINE,
  EVENT_PROGRAM_CHANGE,
  EVENT_END,
  EVENT_VOLUME,
  EVENT_PROFESSIONAL_LOOP,
  EVENT_TRANSPOSE,
  EVENT_ECHO_ON,
  EVENT_ECHO_PARAM,
  EVENT_ECHO_OFF,
  EVENT_NOISE_ADSR,
  EVENT_NOISE_OFF,
  EVENT_PROGADD,
  EVENT_VIBRATO_OFF,
  EVENT_MASTER_VOLUME,
  EVENT_NOTE_SHINGFX,
  EVENT_PERCUSSION_NOTE_SHINGFX,
  EVENT_NOTE_PARAM_SHINGFX,
  EVENT_PITCH_SLIDE,
  EVENT_NOTE_WITHOUT_DUR,
  EVENT_VOLENV,
};

class ArcSystemWorkSnesSeq
  : public VGMSeq {
public:
  ArcSystemWorkSnesSeq(RawFile* file, ArcSystemWorkSnesVersion ver, uint32_t seqdataOffset, std::wstring newName = L"ArcSystemWorks SNES Seq");
  virtual ~ArcSystemWorkSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM(uint8_t tempo);
  uint8_t TIMER0_FREQUENCY;

  ArcSystemWorkSnesVersion version;
  std::map<uint8_t, ArcSystemWorkSnesSeqEventType> EventMap;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);

private:
  void LoadEventMap(void);
};


class ArcSystemWorkSnesTrack
  : public SeqTrack {
public:
  ArcSystemWorkSnesTrack(ArcSystemWorkSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t repeatCount[ARCSYSTEMWORKS_SNES_REPEAT_SLOT_MAX];
  uint16_t noteDeltaTime;
  uint16_t noteDuration;
  uint16_t returnAddress;
  uint8_t currentProgram;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
};



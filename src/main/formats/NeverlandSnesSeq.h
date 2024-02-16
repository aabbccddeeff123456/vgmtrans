#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "NeverlandSnesFormat.h"

enum NeverlandSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_NOTE_PREV_PARAM,
  EVENT_VIBRATO,
  EVENT_VOLUME,
  EVENT_PAN,
  EVENT_WAIT,
  EVENT_TEMPO,
  EVENT_PITCH_BEND,
  EVENT_PROGCHANGE,
  EVENT_LOOP_START,
  EVENT_LOOP_END,
  EVENT_SECTION_END,
  EVENT_END,
  EVENT_SUBEVENT,

  EVENT_MISC_EVENT,
};

enum NeverlandSnesSeqSubEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  SUBEVENT_UNKNOWN0 = 1,
  SUBEVENT_UNKNOWN1,
  SUBEVENT_UNKNOWN2,
  SUBEVENT_UNKNOWN3,
  SUBEVENT_UNKNOWN4,
  SUBEVENT_ECHO_DELAY,
  SUBEVENT_ECHO_FEEDBACK,
  SUBEVENT_ECHO_FIR,
  SUBEVENT_ECHO_ON,
  SUBEVENT_ECHO_OFF,
  SUBEVENT_ADSR_AR,
  SUBEVENT_ADSR_DR,
  SUBEVENT_ADSR_SL,
  SUBEVENT_ADSR_RR,
  SUBEVENT_ADSR_SR,
  SUBEVENT_ECHO_VOLUME_LR,
  SUBEVENT_ECHO_VOLUME_L,
  SUBEVENT_ECHO_VOLUME_R,
  SUBEVENT_NOISE_FREQ,
  SUBEVENT_NOISE_ON,
  SUBEVENT_NOISE_OFF,
  SUBEVENT_PMOD_ON,
  SUBEVENT_PMOD_OFF,
  SUBEVENT_TREMOLO_DEPTH,
  SUBEVENT_VIBRATO_DEPTH,
  SUBEVENT_TREMOLO_RATE,
  SUBEVENT_VIBRATO_RATE,
  SUBEVENT_SURROUND_L,
  SUBEVENT_SURROUND_R,
  SUBEVENT_SURROUND_OFF,
  SUBEVENT_SURROUND_ECHO_VOLUME_L,
  SUBEVENT_SURROUND_ECHO_VOLUME_R,
  SUBEVENT_SURROUND_ECHO_VOLUME_OFF,
};

class NeverlandSnesSeq
    : public VGMSeq {
 public:
  NeverlandSnesSeq(RawFile *file, NeverlandSnesVersion ver, uint32_t seqdataOffset);
  virtual ~NeverlandSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  NeverlandSnesVersion version;
  std::map<uint8_t, NeverlandSnesSeqEventType> EventMap;
  std::map<uint8_t, NeverlandSnesSeqSubEventType> SubEventMap;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
  uint16_t GetShortAddressBE(uint32_t offset);
  double GetTempoInBPM(uint8_t tempo);

  bool percussion[8];
  uint16_t sectionPointer[8];
  uint8_t channelTranspose[8];

 private:
  void LoadEventMap(void);
};


class NeverlandSnesTrack
    : public SeqTrack {
 public:
  NeverlandSnesTrack(NeverlandSnesSeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
  uint16_t GetShortAddressBE(uint32_t offset);
  uint16_t GetShortAddressBEParentSeq(uint16_t offset);

  uint8_t spcTranspose;

  uint16_t currentSection[8];
  uint8_t prevDuration;
  uint16_t currentSectionListPointer[8];

  uint16_t loopReturnSectionNumber[8];
  uint16_t loopReturnSectionPointer[8];
  uint16_t loopReturnPointer[8];
  uint8_t loopCount[8];
  uint8_t loopGroup[8];
  uint8_t transposeLoop[8];
};

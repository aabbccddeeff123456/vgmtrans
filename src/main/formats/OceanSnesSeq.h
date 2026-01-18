#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "OceanSnesFormat.h"

enum OceanSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  // shared or N-SPC
  EVENT_NOP,
  EVENT_NOP1,
  EVENT_END,
  EVENT_NOTE_PARAM,
  EVENT_NOTE,
  EVENT_TIE,
  EVENT_REST,
  EVENT_PERCUSSION_NOTE,
  EVENT_PROGCHANGE,
  EVENT_PAN,
  EVENT_PAN_FADE,
  EVENT_VIBRATO_ON,
  EVENT_VIBRATO_OFF,
  EVENT_MASTER_VOLUME,
  EVENT_MASTER_VOLUME_FADE,
  EVENT_TEMPO,
  EVENT_TEMPO_FADE,
  EVENT_GLOBAL_TRANSPOSE,
  EVENT_TRANSPOSE,
  EVENT_TREMOLO_ON,
  EVENT_TREMOLO_OFF,
  EVENT_VOLUME,
  EVENT_VOLUME_FADE,
  EVENT_CALL,
  EVENT_VIBRATO_FADE,
  EVENT_PITCH_ENVELOPE_TO,
  EVENT_PITCH_ENVELOPE_FROM,
  EVENT_PITCH_ENVELOPE_OFF,
  EVENT_TUNING,
  EVENT_ECHO_ON,
  EVENT_ECHO_OFF,
  EVENT_ECHO_PARAM,
  EVENT_ECHO_VOLUME_FADE,
  EVENT_PITCH_SLIDE,
  EVENT_PERCCUSION_PATCH_BASE,
  EVENT_NOP2,
  EVENT_MUTE,
  EVENT_FASTFORWARD_ON,
  EVENT_FASTFORWARD_OFF,
  EVENT_ADSR_GAIN,
  EVENT_INSTRUMENT_EFFECT_ON,
  EVENT_INSTRUMENT_EFFECT_OFF,

  // STANDALONE

  EVENT_WAIT,
  EVENT_VOL,
  EVENT_CHANGE_EFFECT,
  EVENT_KEY_OFF_TRIGGER,
  EVENT_EXPRESSION,
  EVENT_PORTAMENTO,
  EVENT_PROGCHANGE_ONEBYTE,
  EVENT_TRACKEND,
  EVENT_EFFECT_SPEED,
  EVENT_EFFECT_DEPTH,
  EVENT_EFFECT_RESET,
  EVENT_KEY_OFF_TRIGGER_ONOFF,
  EVENT_ECHO_FIR,
  EVENT_ECHO_VOLUME_L,
  EVENT_ECHO_VOLUME_R,
  EVENT_ECHO_FEEDBACK,
  EVENT_ECHO_VOLUME,
  EVENT_JUMP,
  EVENT_SLUR,
  EVENT_ECHO_DELAY,
  EVENT_PITCH_FADE,
  EVENT_TICK,
  EVENT_VOLUME_REL,
  EVENT_WAITMODE_ON,
  EVENT_WAITMODE_OFF,
  EVENT_NOTE_ON,
  EVENT_RELEASE_NOTE,
};


class OceanSnesSeq
    : public VGMSeq {
 public:
  OceanSnesSeq(RawFile *file, OceanSnesVersion ver, uint32_t seqdataOffset, std::wstring newName = L"Ocean Jonathan Dunn SNES Seq");
  virtual ~OceanSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  OceanSnesVersion version;
  std::map<uint8_t, OceanSnesSeqEventType> EventMap;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
  uint16_t GetShortAddressBE(uint32_t offset);
  double GetTempoInBPM(uint8_t tempo);

    std::vector<uint8_t> volumeTable;
  std::vector<uint8_t> durRateTable;
  std::vector<uint8_t> panTable;

  bool percussion[8];
  uint16_t sectionPointer[8];
  uint8_t channelTranspose[8];
  uint16_t sectionPointerLater;

 private:
  void LoadEventMap(void);
};


class OceanSnesTrack
    : public SeqTrack {
 public:
  OceanSnesTrack(OceanSnesSeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
  uint16_t GetShortAddressBE(uint32_t offset);
  uint16_t GetShortAddressBEParentSeq(uint16_t offset);
  void GetVolumeBalance(uint16_t pan, double &volumeLeft, double &volumeRight);
  uint8_t ReadPanTable(uint16_t pan);
  int8_t CalcPanValue(uint8_t pan, double &volumeScale, bool &reverseLeft, bool &reverseRight);

  uint8_t spcTranspose;
  uint8_t spcNoteDuration;
  uint8_t spcNoteDurRate;
  uint8_t spcNoteVolume;

  uint16_t loopReturnAddress;
  uint16_t loopStartAddress;
  uint8_t loopCount;

  uint16_t currentSection[8];
  uint8_t prevDuration;
  uint16_t currentSectionListPointer[8];

  uint8_t tick;

  bool keyOff;

};

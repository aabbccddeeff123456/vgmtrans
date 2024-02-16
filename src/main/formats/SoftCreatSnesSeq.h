#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "SoftCreatSnesFormat.h"

#define SOFTCREATSNES_LOOP_LEVEL_MAX 16
#define SOFTCREATSNES_SUBROUTINE_LEVEL_MAX 16

enum SoftCreatSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_END,
  EVENT_PROGCHANGE,
  EVENT_GOTO,
  EVENT_CALL_SUBROUTINE,
  EVENT_SUBROUTINE_END,
  EVENT_REPEAT_START,
  EVENT_REPEAT_END,
  EVENT_DEFAULT_NOTE_LENGTH,
  EVENT_DEFAULT_NOTE_LENGTH_CLEAR,
  EVENT_TRANSPOSE,
  EVENT_VOLUME,
  EVENT_VOLUME_L,
  EVENT_VOLUME_R,
  EVENT_GAIN_FADE_ENVELOPE,
  EVENT_TUNING,
  EVENT_VIBRATO,
  EVENT_VIBRATO_FADE,
  EVENT_VIBRATO_OFF,
  EVENT_DURATION_DIRECT,
  EVENT_DURATION_SUBTRACT,
  EVENT_PITCH_FADE,
  EVENT_PITCH_FADE_OFF,
  EVENT_PITCH_FADE_ENVELOPE,
  EVENT_GAIN_PRESET,
  EVENT_NOISE_TOGGLE,
  EVENT_NOISE_FREQ,
  EVENT_SLUR_ON,
  EVENT_SLUR_OFF,
  EVENT_SLUR_OFF_2,
  EVENT_ECHO_ON,
  EVENT_ECHO_VOLUME_LEFT,
  EVENT_ECHO_VOLUME_RIGHT,
  EVENT_ECHO_FEEDBACK,
  EVENT_ECHO_FIR,
  EVENT_LOAD_SONG,
  EVENT_GAIN_FADE_MANUALLY,
  EVENT_JUMP,
  EVENT_JUMP_A4,
  EVENT_MUTE,
  EVENT_VOLUME_PAN,
  EVENT_PAN_LFO,
  EVENT_TEMPO,
  EVENT_DISABLE_VOL,
  EVENT_APU_PORTS,
  EVENT_PERCESSION,
  EVENT_PERCESSION_OFF,
  EVENT_RESET,
  EVENT_VETOCITY_ON,
  EVENT_VETOCITY_OFF,
  EVENT_NOTE,
};

class SoftCreatSnesSeq
    : public VGMSeq {
 public:
  SoftCreatSnesSeq(RawFile *file,
                   SoftCreatSnesVersion ver,
                   uint32_t seqdata_offset,
                   uint8_t headerAlignSize,
                   std::wstring newName = L"SoftCreat SNES Seq");
  virtual ~SoftCreatSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  SoftCreatSnesVersion version;
  std::map<uint8_t, SoftCreatSnesSeqEventType> EventMap;

  double GetTempoInBPM(uint8_t tempo);

  uint8_t TIMER2_FREQUENCY;

 private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class SoftCreatSnesTrack
    : public SeqTrack {
 public:
  SoftCreatSnesTrack(SoftCreatSnesSeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint16_t subroutineReturnAddress[SOFTCREATSNES_SUBROUTINE_LEVEL_MAX];
  uint16_t loopReturnAddress[SOFTCREATSNES_LOOP_LEVEL_MAX];
  uint8_t loopCount[SOFTCREATSNES_LOOP_LEVEL_MAX];
  uint8_t defaultNoteLength;
  uint8_t duration;
  uint8_t durationSubtract;
  uint8_t lastDuration;
  uint8_t loopStack;
  uint8_t subroutineStack;
  uint16_t percessionPointer;

  bool percession;
  bool vetocitySwitch;
};

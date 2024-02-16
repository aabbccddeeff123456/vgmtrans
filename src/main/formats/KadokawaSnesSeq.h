#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "KadokawaSnesFormat.h"

#define KADOKAWASNES_LOOP_LEVEL_MAX 4

enum KadokawaSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_CHANNELTEMPO,
  EVENT_ECHOPARAM,
  EVENT_PAN,
  EVENT_PROGCHANGE,
  EVENT_LOOPSTART,
  EVENT_LOOPEND,
  EVENT_NOTEDUR,
  EVENT_NOTE,
  EVENT_TIE,
  EVENT_REST,
  EVENT_VOLUME,
  EVENT_JUMP,
  EVENT_PERCUSSION_NOTE,
  EVENT_TUNING,
  EVENT_VIBRATO_ON,
  EVENT_VIBRATO_OFF,
  EVENT_VIBRATO_FADE,
  EVENT_PITCH_ENVELOPE_TO,
  EVENT_PITCH_ENVELOPE_FROM,
  EVENT_PITCH_ENVELOPE_OFF,
  EVENT_VOLUME_FADE,
  EVENT_MASTER_VOLUME,
  EVENT_ECHO_SET,
  EVENT_TRANSPOSE,
  EVENT_PAN_FADE,
  EVENT_GLOBAL_TRANSPOSE,
  EVENT_TREMOLO_ON,
  EVENT_TREMOLO_OFF,
  EVENT_CALL,
  EVENT_RETURN,
  EVENT_STOP,
  EVENT_TEMPO_FADE,
  EVENT_ECHO_DELAY,
  EVENT_ECHO_FEEDBACK,
  EVENT_ECHO_FIR,
  EVENT_ECHO_ON,
  EVENT_ECHO_OFF,
  EVENT_ECHO_VOLUME,
  EVENT_DURATION,
  EVENT_NOISE_NOTE,
  EVENT_NOISE_FREQ_FADE,
  EVENT_NOISE_OFF,
  EVENT_PMOD_ON,
  EVENT_PMOD_OFF,
  EVENT_ADSR1,
  EVENT_ADSR2,
  EVENT_RELEASE_GAIN,
  EVENT_RESET_ADSR,
  EVENT_DURATION_RATE,
  EVENT_PAN_LFO,
  EVENT_PAN_LFO_OFF,
  EVENT_PITCH_BASE,
  EVENT_NOP,
  EVENT_NOTE_PARAM,
};

class KadokawaSnesSeq
  : public VGMSeq {
public:
  KadokawaSnesSeq(RawFile *file, KadokawaSnesVersion ver, uint32_t seqdataOffset, std::wstring newName = L"Sting SNES Seq");
  virtual ~KadokawaSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  double GetTempoInBPM();
  double GetTempoInBPM(uint8_t tempo);

  KadokawaSnesVersion version;
  std::map<uint8_t, KadokawaSnesSeqEventType> EventMap;

  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);

  std::vector<uint8_t> volumeTable;
  std::vector<uint8_t> durRateTable;
  std::vector<uint8_t> panTable;

private:
  void LoadEventMap(void);
};


class KadokawaSnesTrack
  : public SeqTrack {
public:
  KadokawaSnesTrack(KadokawaSnesSeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);
  void GetVolumeBalance(uint16_t pan, double& volumeLeft, double& volumeRight);
  uint8_t ReadPanTable(uint16_t pan);
  int8_t CalcPanValue(uint8_t pan, double& volumeScale, bool& reverseLeft, bool& reverseRight);
  uint16_t loopReturnAddress[KADOKAWASNES_LOOP_LEVEL_MAX];
  uint16_t loopStartAddress[KADOKAWASNES_LOOP_LEVEL_MAX];
  uint8_t loopCount;
  uint8_t noteDur;
  uint8_t loopLevel;
  uint8_t callLevel;
  uint8_t spcNoteDurRate;
  uint8_t spcNoteVolume;
  bool isInSubroutine;
  uint8_t loopIncCount[KADOKAWASNES_LOOP_LEVEL_MAX];
  uint8_t loopDecCount[KADOKAWASNES_LOOP_LEVEL_MAX];
  uint16_t loopStart[KADOKAWASNES_LOOP_LEVEL_MAX];
  uint16_t ConvertToAPUAddress(uint16_t offset);
  uint16_t GetShortAddress(uint32_t offset);
};



#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "GameFreakSnesFormat.h"

enum GameFreakSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_END,
  EVENT_REST,
  EVENT_NOTE_PARAM,
  EVENT_ECHO_PARAM,
  EVENT_PROGCHANGE,
  EVENT_ECHO_OFF,
  EVENT_VOLUME,
  EVENT_ADSR_FROM_TABLE,
  EVENT_TIE_STATE,
  EVENT_NOTE,
  EVENT_VOLUME_REL,
  EVENT_VOLUME_REL_FOREVER,
  EVENT_TRANSPOSE,
  EVENT_CALL,
  EVENT_LOOP_UNITL,
  EVENT_TEMPO,
  EVENT_VIBRATO,
  EVENT_NOTE_LENGTH,
  EVENT_VIBRATO2,
  EVENT_VIBRATO_OFF,
  EVENT_PLAY_SONG,
  EVENT_GLOBAL_TRANSPOSE,
  EVENT_ECHO_FIR,
  EVENT_NOISE,
  EVENT_NOISE_OFF,
  EVENT_JUMP,
};

class GameFreakSnesSeq
  : public VGMSeq {
public:
  GameFreakSnesSeq(RawFile* file,
    GameFreakSnesVersion ver,
    uint32_t seqdata_offset,
    uint16_t instrumentPointer,
    std::wstring newName = L"GameFreak SNES Seq");
  virtual ~GameFreakSnesSeq(void);

  uint16_t tempo;
  uint16_t instrumentPointer;

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);
  double GetTempoInBPM();
  double GetTempoInBPM(uint16_t tempo);

  GameFreakSnesVersion version;
  std::map<uint8_t, GameFreakSnesSeqEventType> EventMap;

private:
  void LoadEventMap(void);

};


class GameFreakSnesTrack
  : public SeqTrack {
public:
  GameFreakSnesTrack(GameFreakSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);
  void AddVolLR(uint32_t offset,
    uint32_t length,
    int8_t spcVolL,
    int8_t spcVolR,
    const std::wstring& sEventName = L"Volume L/R");
  void AddVolLRNoItem(int8_t spcVolL, int8_t spcVolR);

  int8_t spcVolL, spcVolR;                        // SPC700 left/right volume
  bool tieState;
  uint8_t loopCount;
  uint16_t subroutineReturnAddress;
  int previousKey;
  uint8_t volumerelativeforever;

  void CalcVolPanFromVolLR(int8_t volL, int8_t volR, uint8_t& midiVol, uint8_t& midiPan);
};

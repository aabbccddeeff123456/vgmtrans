#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "SensibleSnesFormat.h"

#define SENSIBLESNES_LOOP_LEVEL_MAX 4
#define SENSIBLESNES_SUBROUTINE_LEVEL_MAX 4

enum SensibleSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_PROGCHANGE,
  EVENT_PAN,
  EVENT_VOLENV,
  EVENT_NOISE,
  EVENT_TEMPO,
  EVENT_PITCHENV,
  EVENT_JUMP,
  EVENT_LOOPSTART,
  EVENT_LOOPEND,
  EVENT_TRACKEND,
  EVENT_REST,
  EVENT_TRANSPOSE,
  EVENT_GOTOSUBROUTINE,
  EVENT_SUBROUTINEEND,
  EVENT_PANFADE,
  EVENT_MASTERVOL,
  EVENT_GLOBALTRANSPOSE,
  EVENT_NOISENOTE,
  EVENT_TUNING,
  EVENT_PITCHFADE,
  EVENT_ADSR,
  EVENT_ECHO_VOLUME,
  EVENT_ECHO_PARAM,
  EVENT_ECHO_FIR,
  EVENT_ECHO_ON,
};

class SensibleSnesSeq
  : public VGMSeq {
public:
  SensibleSnesSeq(RawFile* file,
    SensibleSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Beat Maniac SNES Seq");
  virtual ~SensibleSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  SensibleSnesVersion version;
  std::map<uint8_t, SensibleSnesSeqEventType> EventMap;

  double GetTempoInBPM(uint8_t tempo);

  uint8_t T0FREQ = GetByte(0xfa);

private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class SensibleSnesTrack
  : public SeqTrack {
public:
  SensibleSnesTrack(SensibleSnesSeq* parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t loopGroup;
  uint8_t loopCount[SENSIBLESNES_LOOP_LEVEL_MAX];
  uint8_t subroutineGroup;
  uint16_t subroutineReturnAddress[SENSIBLESNES_SUBROUTINE_LEVEL_MAX];
};

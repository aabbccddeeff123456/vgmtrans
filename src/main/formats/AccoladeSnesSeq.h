#pragma once
#include "VGMSeqNoTrks.h"
#include "AccoladeSnesFormat.h"

class AccoladeSnesSeq
  : public VGMSeqNoTrks {
public:
  AccoladeSnesSeq(RawFile* file,
    AccoladeSnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"Accolade SNES Midi Seq");
  virtual ~AccoladeSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);
  uint8_t GetDataByte(uint32_t offset);

  AccoladeSnesVersion version;
  bool Beginning;
  bool bSkipDeltaTime;

protected:
  uint8_t runningStatus;
};

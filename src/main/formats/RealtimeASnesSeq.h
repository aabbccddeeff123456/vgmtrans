#pragma once
#include "VGMSeqNoTrks.h"
#include "RealtimeASnesFormat.h"

class RealtimeASnesSeq
  : public VGMSeqNoTrks {
public:
  RealtimeASnesSeq(RawFile* file,
    RealtimeASnesVersion ver,
    uint32_t seqdata_offset,
    std::wstring newName = L"David Warhol SNES Seq");
  virtual ~RealtimeASnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  RealtimeASnesVersion version;
  bool Beginning;

protected:
  uint8_t runningStatus;
};

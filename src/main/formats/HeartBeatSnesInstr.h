#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"
#include "HeartBeatSnesFormat.h"

// *********************
// HeartBeatSnesInstrSet
// *********************

class HeartBeatSnesInstrSet:
    public VGMInstrSet {
 public:
  HeartBeatSnesInstrSet(RawFile *file,
                        HeartBeatSnesVersion ver,
                        uint32_t offset,
                        uint32_t length,
                        uint16_t addrSRCNTable,
                        uint8_t songIndex,
                        uint32_t spcDirAddr,
                        const std::wstring &name = L"NinSnesInstrSet");
  virtual ~HeartBeatSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

  HeartBeatSnesVersion version;

 protected:
  uint16_t addrSRCNTable;
  uint8_t songIndex;
  uint32_t spcDirAddr;
  std::vector<uint8_t> usedSRCNs;
};

// ******************
// HeartBeatSnesInstr
// ******************

class HeartBeatSnesInstr
    : public VGMInstr {
 public:
  HeartBeatSnesInstr(VGMInstrSet *instrSet,
                     HeartBeatSnesVersion ver,
                     uint32_t offset,
                     uint32_t theBank,
                     uint32_t theInstrNum,
                     uint16_t addrSRCNTable,
                     uint8_t songIndex,
                     uint32_t spcDirAddr,
                     const std::wstring &name = L"NinSnesInstr");
  virtual ~HeartBeatSnesInstr(void);

  virtual bool LoadInstr();

  HeartBeatSnesVersion version;

 protected:
  uint16_t addrSRCNTable;
  uint8_t songIndex;
  uint32_t spcDirAddr;
};

// ****************
// HeartBeatSnesRgn
// ****************

class HeartBeatSnesRgn
    : public VGMRgn {
 public:
  HeartBeatSnesRgn(HeartBeatSnesInstr *instr, HeartBeatSnesVersion ver, uint32_t offset);
  virtual ~HeartBeatSnesRgn(void);

  virtual bool LoadRgn();

  HeartBeatSnesVersion version;
};

#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"

// ****************
// WinkySoftSnesInstrSet
// ****************

class WinkySoftSnesInstrSet :
  public VGMInstrSet {
public:
  WinkySoftSnesInstrSet
  (RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name = L"WinkySoftSnesInstrSet");
  virtual ~WinkySoftSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

protected:
  uint32_t spcDirAddr;
  std::vector<uint8_t> usedSRCNs;
};

// *************
// WinkySoftSnesInstr
// *************

class WinkySoftSnesInstr
  : public VGMInstr {
public:
  WinkySoftSnesInstr(VGMInstrSet* instrSet,
    uint32_t offset,
    uint32_t theBank,
    uint32_t theInstrNum,
    uint32_t spcDirAddr,
    const std::wstring& name = L"WinkySoftSnesInstr");
  virtual ~WinkySoftSnesInstr(void);

  virtual bool LoadInstr();

  static bool IsValidHeader(RawFile* file, uint32_t addrInstrHeader, uint32_t spcDirAddr, bool validateSample);

protected:
  uint32_t spcDirAddr;
};

// ***********
// WinkySoftSnesRgn
// ***********

class WinkySoftSnesRgn
  : public VGMRgn {
public:
  WinkySoftSnesRgn(WinkySoftSnesInstr* instr, uint32_t offset);
  virtual ~WinkySoftSnesRgn(void);

  virtual bool LoadRgn();
};

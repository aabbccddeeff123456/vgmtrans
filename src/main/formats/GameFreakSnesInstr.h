#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"

// ****************
// GameFreakSnesInstrSet
// ****************

class GameFreakSnesInstrSet :
  public VGMInstrSet {
public:
  GameFreakSnesInstrSet
  (RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name = L"GameFreakSnesInstrSet");
  virtual ~GameFreakSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

protected:
  uint32_t spcDirAddr;
  std::vector<uint8_t> usedSRCNs;
};

// *************
// GameFreakSnesInstr
// *************

class GameFreakSnesInstr
  : public VGMInstr {
public:
  GameFreakSnesInstr(VGMInstrSet* instrSet,
    uint32_t offset,
    uint32_t theBank,
    uint32_t theInstrNum,
    uint8_t srcn,
    uint32_t spcDirAddr,
    const std::wstring& name = L"GameFreakSnesInstr");
  virtual ~GameFreakSnesInstr(void);

  virtual bool LoadInstr();

  static bool IsValidHeader(RawFile* file, uint32_t addrInstrHeader, uint32_t spcDirAddr, bool validateSample);

protected:
  uint8_t srcn;
  uint32_t spcDirAddr;
};

// ***********
// GameFreakSnesRgn
// ***********

class GameFreakSnesRgn
  : public VGMRgn {
public:
  GameFreakSnesRgn(GameFreakSnesInstr* instr, uint32_t offset, uint8_t srcn);
  virtual ~GameFreakSnesRgn(void);

  virtual bool LoadRgn();
};


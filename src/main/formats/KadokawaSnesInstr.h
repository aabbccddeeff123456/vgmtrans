#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"

// ****************
// KadokawaSnesInstrSet
// ****************

class KadokawaSnesInstrSet :
  public VGMInstrSet {
public:
  KadokawaSnesInstrSet
  (RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name = L"StingSnesInstrSet");
  virtual ~KadokawaSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

protected:
  uint32_t spcDirAddr;
  std::vector<uint8_t> usedSRCNs;
};

// *************
// KadokawaSnesInstr
// *************

class KadokawaSnesInstr
  : public VGMInstr {
public:
  KadokawaSnesInstr(VGMInstrSet* instrSet,
    uint32_t offset,
    uint32_t theBank,
    uint32_t theInstrNum,
    uint32_t spcDirAddr,
    const std::wstring& name = L"StingSnesInstr");
  virtual ~KadokawaSnesInstr(void);

  virtual bool LoadInstr();

  static bool IsValidHeader(RawFile* file, uint32_t addrInstrHeader, uint32_t spcDirAddr, bool validateSample);

protected:
  uint32_t spcDirAddr;
};

// ***********
// KadokawaSnesRgn
// ***********

class KadokawaSnesRgn
  : public VGMRgn {
public:
  KadokawaSnesRgn(KadokawaSnesInstr* instr, uint32_t offset);
  virtual ~KadokawaSnesRgn(void);

  virtual bool LoadRgn();
};


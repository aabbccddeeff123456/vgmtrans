#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"
#include "NeverlandSnesFormat.h"

// ****************
// NeverlandSnesInstrSet
// ****************

class NeverlandSnesInstrSet :
  public VGMInstrSet {
public:
  static const uint32_t DRUMKIT_PROGRAM = (0x7F << 14);

  NeverlandSnesInstrSet(RawFile* file,
    NeverlandSnesVersion ver,
    uint32_t spcDirAddr,
    uint16_t addrTuningTable,
    const std::wstring& name = L"NeverlandSnesInstrSet");
  virtual ~NeverlandSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

  NeverlandSnesVersion version;

protected:
  uint32_t spcDirAddr;
  uint16_t addrTuningTable;
  uint16_t addrADSRTable;
  std::vector<uint8_t> usedSRCNs;
};

// *************
// NeverlandSnesInstr
// *************

class NeverlandSnesInstr
  : public VGMInstr {
public:
  NeverlandSnesInstr(VGMInstrSet* instrSet,
    NeverlandSnesVersion ver,
    uint8_t srcn,
    uint32_t spcDirAddr,
    uint16_t addrTuningTable,
    const std::wstring& name = L"NeverlandSnesInstr");
  virtual ~NeverlandSnesInstr(void);

  virtual bool LoadInstr();

  NeverlandSnesVersion version;

protected:
  uint32_t spcDirAddr;
  uint16_t addrTuningTable;
  uint16_t addrADSRTable;
};

class NeverlandSnesRgn
  : public VGMRgn {
public:
  NeverlandSnesRgn(VGMInstr* instr,
    NeverlandSnesVersion ver,
    uint16_t addrTuningTable);
  virtual ~NeverlandSnesRgn(void);

  bool InitializeRegion(uint8_t srcn,
    uint32_t spcDirAddr,
    uint16_t addrADSRTable);
  virtual bool LoadRgn();

  NeverlandSnesVersion version;
};

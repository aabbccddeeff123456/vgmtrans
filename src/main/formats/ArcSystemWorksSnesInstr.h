#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"
#include "ArcSystemWorksSnesFormat.h"

// ****************
// ArcSystemWorkSnesInstrSet
// ****************

class ArcSystemWorkSnesInstrSet :
  public VGMInstrSet {
public:
  static const uint32_t DRUMKIT_PROGRAM = (0x7F << 14);

  ArcSystemWorkSnesInstrSet(RawFile* file,
    ArcSystemWorkSnesVersion ver,
    uint32_t spcDirAddr,
    uint16_t addrTuningTable,
    const std::wstring& name = L"ArcSystemWorksSnesInstrSet");
  virtual ~ArcSystemWorkSnesInstrSet(void);

  virtual bool GetHeaderInfo();
  virtual bool GetInstrPointers();

  ArcSystemWorkSnesVersion version;

protected:
  uint32_t spcDirAddr;
  uint16_t addrTuningTable;
  uint16_t addrADSRTable;
  std::vector<uint8_t> usedSRCNs;
};

// *************
// ArcSystemWorkSnesInstr
// *************

class ArcSystemWorkSnesInstr
  : public VGMInstr {
public:
  ArcSystemWorkSnesInstr(VGMInstrSet* instrSet,
    ArcSystemWorkSnesVersion ver,
    uint8_t srcn,
    uint32_t spcDirAddr,
    uint16_t addrTuningTable,
    const std::wstring& name = L"ArcSystemWorksSnesInstr");
  virtual ~ArcSystemWorkSnesInstr(void);

  virtual bool LoadInstr();

  ArcSystemWorkSnesVersion version;

protected:
  uint32_t spcDirAddr;
  uint16_t addrTuningTable;
  uint16_t addrADSRTable;
};

class ArcSystemWorkSnesRgn
  : public VGMRgn {
public:
  ArcSystemWorkSnesRgn(VGMInstr* instr,
    ArcSystemWorkSnesVersion ver,
    uint16_t addrTuningTable);
  virtual ~ArcSystemWorkSnesRgn(void);

  bool InitializeRegion(uint8_t srcn,
    uint32_t spcDirAddr,
    uint16_t addrADSRTable);
  virtual bool LoadRgn();

  ArcSystemWorkSnesVersion version;
};

#include "pch.h"
#include "NeverlandSnesInstr.h"
#include "NeverlandSnesFormat.h"
#include "SNESDSP.h"

// ****************
// NeverlandSnesInstrSet
// ****************

NeverlandSnesInstrSet::NeverlandSnesInstrSet(RawFile* file,
  NeverlandSnesVersion ver,
  uint32_t spcDirAddr,
  uint16_t addrTuningTable,
  const std::wstring& name) :
  VGMInstrSet(NeverlandSnesFormat::name, file, addrTuningTable, 0, name), version(ver),
  spcDirAddr(spcDirAddr),
  addrTuningTable(addrTuningTable) {
}

NeverlandSnesInstrSet::~NeverlandSnesInstrSet() {
}

bool NeverlandSnesInstrSet::GetHeaderInfo() {
  return true;
}

bool NeverlandSnesInstrSet::GetInstrPointers() {
  usedSRCNs.clear();
  uint8_t srcn_max = 0x3f;
  for (uint8_t srcn = 0; srcn <= srcn_max; srcn++) {
    uint32_t addrDIRentry = spcDirAddr + (srcn * 4);
    if (!SNESSampColl::IsValidSampleDir(rawfile, addrDIRentry, true)) {
      continue;
    }

    uint16_t addrSampStart = GetShort(addrDIRentry);
    uint16_t instrumentMinOffset;

    // Some instruments can be placed before the DIR table. At least a few
    // songs from Chrono Trigger ("World Revolution", "Last Battle") will do
    // this.
    instrumentMinOffset = 0x200;


    if (addrSampStart < instrumentMinOffset) {
      continue;
    }

    uint32_t ofsTuningEntry;
    ofsTuningEntry = addrTuningTable + srcn * 4;

    if (ofsTuningEntry + 2 > 0x10000) {
      break;
    }



    usedSRCNs.push_back(srcn);

    std::wostringstream instrName;
    instrName << L"Instrument " << srcn;
    NeverlandSnesInstr* newInstr = new NeverlandSnesInstr(this, version, srcn, spcDirAddr, ofsTuningEntry, instrName.str());
    aInstrs.push_back(newInstr);
  }
  if (aInstrs.size() == 0) {
    return false;
  }


  std::sort(usedSRCNs.begin(), usedSRCNs.end());
  SNESSampColl* newSampColl = new SNESSampColl(NeverlandSnesFormat::name, this->rawfile, spcDirAddr, usedSRCNs);
  if (!newSampColl->LoadVGMFile()) {
    delete newSampColl;
    return false;
  }

  return true;
}

// *************
// NeverlandSnesInstr
// *************

NeverlandSnesInstr::NeverlandSnesInstr(VGMInstrSet* instrSet,
  NeverlandSnesVersion ver,
  uint8_t srcn,
  uint32_t spcDirAddr,
  uint16_t addrTuningTable,
  const std::wstring& name) :
  VGMInstr(instrSet, addrTuningTable, 0, 0, srcn, name), version(ver),
  spcDirAddr(spcDirAddr),
  addrTuningTable(addrTuningTable) {
}

NeverlandSnesInstr::~NeverlandSnesInstr() {
}

bool NeverlandSnesInstr::LoadInstr() {
  uint32_t offDirEnt = spcDirAddr + (instrNum * 4);
  if (offDirEnt + 4 > 0x10000) {
    return false;
  }

  uint16_t addrSampStart = GetShort(offDirEnt);

  NeverlandSnesRgn* rgn = new NeverlandSnesRgn(this, version, addrTuningTable);
  rgn->sampOffset = addrSampStart - spcDirAddr;
  if (!rgn->InitializeRegion(instrNum, spcDirAddr, addrADSRTable) || !rgn->LoadRgn()) {
    delete rgn;
    return false;
  }
  aRgns.push_back(rgn);

  SetGuessedLength();
  return true;
}

// ***********
// NeverlandSnesRgn
// ***********

NeverlandSnesRgn::NeverlandSnesRgn(VGMInstr* instr,
  NeverlandSnesVersion ver,
  uint16_t addrTuningTable) :
  VGMRgn(instr, addrTuningTable, 0),
  version(ver) {
}

bool NeverlandSnesRgn::InitializeRegion(uint8_t srcn,
  uint32_t spcDirAddr,
  uint16_t addrADSRTable)
{
  uint16_t addrTuningTable = dwOffset;
  uint8_t adsr1;
  uint8_t adsr2;
  int16_t pitch_scale;
  adsr1 = GetByte(dwOffset);
  adsr2 = GetByte(dwOffset + 1);
  pitch_scale = GetShortBE(dwOffset + 2);
  AddSimpleItem(dwOffset, 2, L"ADSR");

  const double pitch_fixer = 4286.0 / 4096.0;
  double fine_tuning;
  double coarse_tuning;
  fine_tuning = modf((log(pitch_scale * pitch_fixer / 256.0) / log(2.0)) * 12.0, &coarse_tuning);
     AddUnityKey(96 - (int) (coarse_tuning), dwOffset + 2, 1);
    AddFineTune((int16_t) (fine_tuning * 100.0), dwOffset + 3, 1);

  // normalize
  if (fine_tuning >= 0.5) {
    coarse_tuning += 1.0;
    fine_tuning -= 1.0;
  }
  else if (fine_tuning <= -0.5) {
    coarse_tuning -= 1.0;
    fine_tuning += 1.0;
  }


  sampNum = srcn;
  SNESConvADSR<VGMRgn>(this, adsr1, adsr2, 0xa0);

  return true;
}

NeverlandSnesRgn::~NeverlandSnesRgn() {
}

bool NeverlandSnesRgn::LoadRgn() {
  SetGuessedLength();

  return true;
}

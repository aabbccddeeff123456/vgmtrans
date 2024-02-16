#include "pch.h"
#include "ArcSystemWorksSnesInstr.h"
#include "ArcSystemWorksSnesFormat.h"
#include "SNESDSP.h"

// ****************
// ArcSystemWorkSnesInstrSet
// ****************

ArcSystemWorkSnesInstrSet::ArcSystemWorkSnesInstrSet(RawFile* file,
  ArcSystemWorkSnesVersion ver,
  uint32_t spcDirAddr,
  uint16_t addrTuningTable,
  const std::wstring& name) :
  VGMInstrSet(ArcSystemWorkSnesFormat::name, file, addrTuningTable, 0, name), version(ver),
  spcDirAddr(spcDirAddr),
  addrTuningTable(addrTuningTable) {
}

ArcSystemWorkSnesInstrSet::~ArcSystemWorkSnesInstrSet() {
}

bool ArcSystemWorkSnesInstrSet::GetHeaderInfo() {
  return true;
}

bool ArcSystemWorkSnesInstrSet::GetInstrPointers() {
  usedSRCNs.clear();
  uint8_t srcn_max = 0x7f;
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
    ofsTuningEntry = addrTuningTable + srcn * 2;

    if (ofsTuningEntry + 2 > 0x10000) {
      break;
    }



    usedSRCNs.push_back(srcn);

    std::wostringstream instrName;
    instrName << L"Instrument " << srcn;
    ArcSystemWorkSnesInstr* newInstr = new ArcSystemWorkSnesInstr(this, version, srcn, spcDirAddr, addrTuningTable, instrName.str());
    aInstrs.push_back(newInstr);
  }
  if (aInstrs.size() == 0) {
    return false;
  }


  std::sort(usedSRCNs.begin(), usedSRCNs.end());
  SNESSampColl* newSampColl = new SNESSampColl(ArcSystemWorkSnesFormat::name, this->rawfile, spcDirAddr, usedSRCNs);
  if (!newSampColl->LoadVGMFile()) {
    delete newSampColl;
    return false;
  }

  return true;
}

// *************
// ArcSystemWorkSnesInstr
// *************

ArcSystemWorkSnesInstr::ArcSystemWorkSnesInstr(VGMInstrSet* instrSet,
  ArcSystemWorkSnesVersion ver,
  uint8_t srcn,
  uint32_t spcDirAddr,
  uint16_t addrTuningTable,
  const std::wstring& name) :
  VGMInstr(instrSet, addrTuningTable, 0, 0, srcn, name), version(ver),
  spcDirAddr(spcDirAddr),
  addrTuningTable(addrTuningTable) {
}

ArcSystemWorkSnesInstr::~ArcSystemWorkSnesInstr() {
}

bool ArcSystemWorkSnesInstr::LoadInstr() {
  uint32_t offDirEnt = spcDirAddr + (instrNum * 4);
  if (offDirEnt + 4 > 0x10000) {
    return false;
  }

  uint16_t addrSampStart = GetShort(offDirEnt);

  ArcSystemWorkSnesRgn* rgn = new ArcSystemWorkSnesRgn(this, version, addrTuningTable);
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
// ArcSystemWorkSnesRgn
// ***********

ArcSystemWorkSnesRgn::ArcSystemWorkSnesRgn(VGMInstr* instr,
  ArcSystemWorkSnesVersion ver,
  uint16_t addrTuningTable) :
  VGMRgn(instr, addrTuningTable, 0),
  version(ver) {
}

bool ArcSystemWorkSnesRgn::InitializeRegion(uint8_t srcn,
  uint32_t spcDirAddr,
  uint16_t addrADSRTable)
{
  uint16_t addrTuningTable = dwOffset;
  uint8_t adsr1;
  uint8_t adsr2;
  adsr1 = GetByte(dwOffset + srcn * 4 + 1);
  adsr2 = GetByte(dwOffset + srcn * 4 + 2);
  AddSimpleItem(dwOffset + srcn * 4, 1, L"SRCN");
  AddSimpleItem(dwOffset + srcn * 4 + 1, 1, L"ADSR1");
  AddSimpleItem(dwOffset + srcn * 4 + 2, 1, L"ADSR2");

  uint8_t tuning1;

  tuning1 = GetByte(dwOffset + srcn * 4 + 3);

  AddSimpleItem(dwOffset + srcn * 4 + 3, 1, L"Tuning");


  double pitch_scale;
  if (tuning1 <= 0x7f) {
    pitch_scale = 1.0 + (tuning1 / 256.0);
  }
  else {
    pitch_scale = tuning1 / 256.0;
  }

  double fine_tuning;
  double coarse_tuning;
  fine_tuning = modf((log(pitch_scale) / log(2.0)) * 12.0, &coarse_tuning);

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
  unityKey = 69 - (int)(coarse_tuning);
  fineTune = (int16_t)(fine_tuning * 100.0);
  SNESConvADSR<VGMRgn>(this, adsr1, adsr2, 0xa0);

  return true;
}

ArcSystemWorkSnesRgn::~ArcSystemWorkSnesRgn() {
}

bool ArcSystemWorkSnesRgn::LoadRgn() {
  SetGuessedLength();

  return true;
}

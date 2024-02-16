#include "pch.h"
#include "WinkySoftSnesInstr.h"
#include "Format.h"
#include "SNESDSP.h"
#include "WinkySoftSnesFormat.h"

// ****************
// WinkySoftSnesInstrSet
// ****************

WinkySoftSnesInstrSet::WinkySoftSnesInstrSet(RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name) :
  VGMInstrSet(WinkySoftSnesFormat::name, file, offset, 0, name),
  spcDirAddr(spcDirAddr) {
}

WinkySoftSnesInstrSet::~WinkySoftSnesInstrSet() {
}

bool WinkySoftSnesInstrSet::GetHeaderInfo() {
  return true;
}

bool WinkySoftSnesInstrSet::GetInstrPointers() {
  usedSRCNs.clear();
  for (int instr = 0; instr <= 0x7f; instr++) {
    uint32_t addrInstrHeader = dwOffset + (8 * instr);
    if (addrInstrHeader + 7 > 0x10000) {
      return false;
    }

    // skip blank slot
    if (GetByte(addrInstrHeader) == 0xff
      && GetByte(addrInstrHeader + 1) == 0xff
      && GetByte(addrInstrHeader + 2) == 0xff
      && GetByte(addrInstrHeader + 3) == 0xff
      && GetByte(addrInstrHeader + 4) == 0xff
      && GetByte(addrInstrHeader + 5) == 0xff
      && GetByte(addrInstrHeader + 6) == 0xff
      && GetByte(addrInstrHeader + 7) == 0xff) {
      continue;
    }

    if (GetByte(addrInstrHeader) == 0x00
      && GetByte(addrInstrHeader + 1) == 0x00
      && GetByte(addrInstrHeader + 2) == 0x00
      && GetByte(addrInstrHeader + 3) == 0x00
      && GetByte(addrInstrHeader + 4) == 0x00
      && GetByte(addrInstrHeader + 5) == 0x00
      && GetByte(addrInstrHeader + 6) == 0x00
      && GetByte(addrInstrHeader + 7) == 0x00) {
      continue;
    }

   // if (!WinkySoftSnesInstr::IsValidHeader(this->rawfile, addrInstrHeader, spcDirAddr, false)) {
   //   break;
   // }
    if (!WinkySoftSnesInstr::IsValidHeader(this->rawfile, addrInstrHeader, spcDirAddr, true)) {
      continue;
    }

    uint8_t srcn = GetByte(addrInstrHeader);
    std::vector<uint8_t>::iterator itrSRCN = find(usedSRCNs.begin(), usedSRCNs.end(), srcn);
    if (itrSRCN == usedSRCNs.end()) {
      usedSRCNs.push_back(srcn);
    }

    std::wostringstream instrName;
    instrName << L"Instrument " << instr;
    WinkySoftSnesInstr
      * newInstr = new WinkySoftSnesInstr(this, addrInstrHeader, instr >> 7, instr & 0x7f, spcDirAddr, instrName.str());
    aInstrs.push_back(newInstr);
  }
  if (aInstrs.size() == 0) {
    return false;
  }

  std::sort(usedSRCNs.begin(), usedSRCNs.end());
  SNESSampColl* newSampColl = new SNESSampColl(WinkySoftSnesFormat::name, this->rawfile, spcDirAddr, usedSRCNs);
  if (!newSampColl->LoadVGMFile()) {
    delete newSampColl;
    return false;
  }

  return true;
}

// *************
// WinkySoftSnesInstr
// *************

WinkySoftSnesInstr::WinkySoftSnesInstr(VGMInstrSet* instrSet,
  uint32_t offset,
  uint32_t theBank,
  uint32_t theInstrNum,
  uint32_t spcDirAddr,
  const std::wstring& name) :
  VGMInstr(instrSet, offset, 8, theBank, theInstrNum, name),
  spcDirAddr(spcDirAddr) {
}

WinkySoftSnesInstr::~WinkySoftSnesInstr() {
}

bool WinkySoftSnesInstr::LoadInstr() {
  uint8_t srcn = GetByte(dwOffset);
  uint32_t offDirEnt = spcDirAddr + (srcn * 4);
  if (offDirEnt + 4 > 0x10000) {
    return false;
  }

  uint16_t addrSampStart = GetShort(offDirEnt);

  WinkySoftSnesRgn* rgn = new WinkySoftSnesRgn(this, dwOffset);
  rgn->sampOffset = addrSampStart - spcDirAddr;
  aRgns.push_back(rgn);

  return true;
}

bool WinkySoftSnesInstr::IsValidHeader(RawFile* file, uint32_t addrInstrHeader, uint32_t spcDirAddr, bool validateSample) {
  if (addrInstrHeader + 6 > 0x10000) {
    return false;
  }

  uint8_t srcn = file->GetByte(addrInstrHeader);
  uint8_t adsr1 = file->GetByte(addrInstrHeader + 1);
  uint8_t adsr2 = file->GetByte(addrInstrHeader + 2);
  int16_t pitch_scale = file->GetShortBE(addrInstrHeader + 3);
  uint8_t defaultVolume = file->GetByte(addrInstrHeader + 5);
  uint8_t defaultPan = file->GetByte(addrInstrHeader + 6);
  uint8_t defaultTranspose = file->GetByte(addrInstrHeader + 7);

  uint32_t addrDIRentry = spcDirAddr + (srcn * 4);
  if (!SNESSampColl::IsValidSampleDir(file, addrDIRentry, validateSample)) {
    return false;
  }

  uint16_t srcAddr = file->GetShort(addrDIRentry);
  uint16_t loopStartAddr = file->GetShort(addrDIRentry + 2);
  if (srcAddr > loopStartAddr || (loopStartAddr - srcAddr) % 9 != 0) {
    return false;
  }

  return true;
}

// ***********
// WinkySoftSnesRgn
// ***********

WinkySoftSnesRgn::WinkySoftSnesRgn(WinkySoftSnesInstr* instr, uint32_t offset) :
  VGMRgn(instr, offset, 8) {
  uint8_t srcn = GetByte(offset);
  uint8_t adsr1 = GetByte(offset + 1);
  uint8_t adsr2 = GetByte(offset + 2);
  int16_t pitch_scale = GetShortBE(offset + 3);
  uint8_t vol = GetByte(offset + 5);
  double pan = GetByte(offset + 6);
  uint8_t defaultTranspose = GetByte(offset + 7);

  const double pitch_fixer = 4286.0 / 4096.0;
  double fine_tuning;
  double coarse_tuning;
  fine_tuning = modf((log(pitch_scale * pitch_fixer / 256.0) / log(2.0)) * 12.0, &coarse_tuning);

  // normalize
  if (fine_tuning >= 0.5) {
    coarse_tuning += 1.0;
    fine_tuning -= 1.0;
  }
  else if (fine_tuning <= -0.5) {
    coarse_tuning -= 1.0;
    fine_tuning += 1.0;
  }

  AddSampNum(srcn, offset, 1);
  AddSimpleItem(offset + 1, 1, L"ADSR1");
  AddSimpleItem(offset + 2, 1, L"ADSR2");
  AddUnityKey(96 - (int)(coarse_tuning), offset + 3, 1);
  AddFineTune((int16_t)(fine_tuning * 100.0), offset + 4, 1);
  AddVolume(vol / 256.0, offset + 5, 1);
  AddPan(pan / 256.0, offset + 6, 1);
  AddSimpleItem(offset + 7, 1, L"Transpose");
  SNESConvADSR<VGMRgn>(this, adsr1, adsr2, 0x00);

  uint8_t sl = (adsr2 >> 5);
  //EmulateSDSPGAIN(gain, (sl << 8) | 0xff, 0, NULL, &release_time);
}

WinkySoftSnesRgn::~WinkySoftSnesRgn() {
}

bool WinkySoftSnesRgn::LoadRgn() {
  return true;
}

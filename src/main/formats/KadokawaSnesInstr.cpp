#include "pch.h"
#include "KadokawaSnesInstr.h"
#include "Format.h"
#include "SNESDSP.h"
#include "KadokawaSnesFormat.h"

// ****************
// KadokawaSnesInstrSet
// ****************

KadokawaSnesInstrSet::KadokawaSnesInstrSet(RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name) :
  VGMInstrSet(KadokawaSnesFormat::name, file, offset, 0, name),
  spcDirAddr(spcDirAddr) {
}

KadokawaSnesInstrSet::~KadokawaSnesInstrSet() {
}

bool KadokawaSnesInstrSet::GetHeaderInfo() {
  return true;
}

bool KadokawaSnesInstrSet::GetInstrPointers() {
  usedSRCNs.clear();
  for (int instr = 0; instr <= 0xff; instr++) {
    uint32_t addrInstrHeader = dwOffset + (6 * instr);
    if (addrInstrHeader + 6 > 0x10000) {
      return false;
    }

    if (GetByte(addrInstrHeader) == 0xff
      && GetByte(addrInstrHeader + 1) == 0xff
      && GetByte(addrInstrHeader + 2) == 0xff
      && GetByte(addrInstrHeader + 3) == 0xff
      && GetByte(addrInstrHeader + 4) == 0xff
      && GetByte(addrInstrHeader + 5) == 0xff) {
      continue;
    }

    if (!KadokawaSnesInstr::IsValidHeader(this->rawfile, addrInstrHeader, spcDirAddr, false)) {
      break;
    }
    if (!KadokawaSnesInstr::IsValidHeader(this->rawfile, addrInstrHeader, spcDirAddr, true)) {
      continue;
    }

    uint8_t srcn = GetByte(addrInstrHeader);
    std::vector<uint8_t>::iterator itrSRCN = find(usedSRCNs.begin(), usedSRCNs.end(), srcn);
    if (itrSRCN == usedSRCNs.end()) {
      usedSRCNs.push_back(srcn);
    }

    std::wostringstream instrName;
    instrName << L"Instrument " << instr;
    KadokawaSnesInstr
      * newInstr = new KadokawaSnesInstr(this, addrInstrHeader, instr >> 7, instr & 0x7f, spcDirAddr, instrName.str());
    aInstrs.push_back(newInstr);
  }
  if (aInstrs.size() == 0) {
    return false;
  }

  std::sort(usedSRCNs.begin(), usedSRCNs.end());
  SNESSampColl* newSampColl = new SNESSampColl(KadokawaSnesFormat::name, this->rawfile, spcDirAddr, usedSRCNs);
  if (!newSampColl->LoadVGMFile()) {
    delete newSampColl;
    return false;
  }

  return true;
}

// *************
// KadokawaSnesInstr
// *************

KadokawaSnesInstr::KadokawaSnesInstr(VGMInstrSet* instrSet,
  uint32_t offset,
  uint32_t theBank,
  uint32_t theInstrNum,
  uint32_t spcDirAddr,
  const std::wstring& name) :
  VGMInstr(instrSet, offset, 6, theBank, theInstrNum, name),
  spcDirAddr(spcDirAddr) {
}

KadokawaSnesInstr::~KadokawaSnesInstr() {
}

bool KadokawaSnesInstr::LoadInstr() {
  uint8_t srcn = GetByte(dwOffset);
  uint32_t offDirEnt = spcDirAddr + (srcn * 4);
  if (offDirEnt + 4 > 0x10000) {
    return false;
  }

  uint16_t addrSampStart = GetShort(offDirEnt);

  KadokawaSnesRgn* rgn = new KadokawaSnesRgn(this, dwOffset);
  rgn->sampOffset = addrSampStart - spcDirAddr;
  aRgns.push_back(rgn);

  return true;
}

bool KadokawaSnesInstr::IsValidHeader(RawFile* file, uint32_t addrInstrHeader, uint32_t spcDirAddr, bool validateSample) {
  if (addrInstrHeader + 6 > 0x10000) {
    return false;
  }

  uint8_t srcn = file->GetByte(addrInstrHeader);
  uint8_t adsr1 = file->GetByte(addrInstrHeader + 1);
  uint8_t adsr2 = file->GetByte(addrInstrHeader + 2);
  uint8_t gain = file->GetByte(addrInstrHeader + 3);
  int16_t pitch_scale = file->GetShortBE(addrInstrHeader + 4);

  if (srcn >= 0x80 || (adsr1 == 0 && gain == 0)) {
    return false;
  }

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
// KadokawaSnesRgn
// ***********

KadokawaSnesRgn::KadokawaSnesRgn(KadokawaSnesInstr* instr, uint32_t offset) :
  VGMRgn(instr, offset, 6) {
  uint8_t srcn = GetByte(offset);
  uint8_t adsr1 = GetByte(offset + 1);
  uint8_t adsr2 = GetByte(offset + 2);
  uint8_t gain = GetByte(offset + 3);
  int16_t pitch_scale = GetShortBE(offset + 4);

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
  AddSimpleItem(offset + 3, 1, L"GAIN");
  AddUnityKey(96 - (int)(coarse_tuning), offset + 4, 1);
  AddFineTune((int16_t)(fine_tuning * 100.0), offset + 5, 1);
  SNESConvADSR<VGMRgn>(this, adsr1, adsr2, gain);

  uint8_t sl = (adsr2 >> 5);
  EmulateSDSPGAIN(gain, (sl << 8) | 0xff, 0, NULL, &release_time);
}

KadokawaSnesRgn::~KadokawaSnesRgn() {
}

bool KadokawaSnesRgn::LoadRgn() {
  return true;
}

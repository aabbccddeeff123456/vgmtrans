#include "pch.h"
#include "GameFreakSnesInstr.h"
#include "Format.h"
#include "SNESDSP.h"
#include "GameFreakSnesFormat.h"

// ****************
// GameFreakSnesInstrSet
// ****************

GameFreakSnesInstrSet::GameFreakSnesInstrSet(RawFile* file, uint32_t offset, uint32_t spcDirAddr, const std::wstring& name) :
  VGMInstrSet(GameFreakSnesFormat::name, file, offset, 0, name),
  spcDirAddr(spcDirAddr) {
}

GameFreakSnesInstrSet::~GameFreakSnesInstrSet() {
}

bool GameFreakSnesInstrSet::GetHeaderInfo() {
  return true;
}

bool GameFreakSnesInstrSet::GetInstrPointers() {
  usedSRCNs.clear();
  for (int instr = 0; instr <= 0x10; instr++) {
    uint32_t addrInstrHeader = GetShort(dwOffset) + (instr * 4);
    if (addrInstrHeader + 7 > 0x10000) {
      return false;
    }

    // if (!GameFreakSnesInstr::IsValidHeader(this->rawfile, addrInstrHeader, spcDirAddr, false)) {
    //   break;
    // }

    uint8_t srcn = instr;
    std::vector<uint8_t>::iterator itrSRCN = find(usedSRCNs.begin(), usedSRCNs.end(), srcn);
    if (itrSRCN == usedSRCNs.end()) {
      usedSRCNs.push_back(srcn);
    }

    std::wostringstream instrName;
    instrName << L"Instrument " << instr;

    GameFreakSnesInstr
      * newInstr = new GameFreakSnesInstr(this, addrInstrHeader, instr >> 7, instr & 0x10, srcn, spcDirAddr, instrName.str());
    aInstrs.push_back(newInstr);
  }
  if (aInstrs.size() == 0) {
    return false;
  }

  std::sort(usedSRCNs.begin(), usedSRCNs.end());
  SNESSampColl* newSampColl = new SNESSampColl(GameFreakSnesFormat::name, this->rawfile, spcDirAddr, usedSRCNs);
  if (!newSampColl->LoadVGMFile()) {
    delete newSampColl;
    return false;
  }

  return true;
}

// *************
// GameFreakSnesInstr
// *************

GameFreakSnesInstr::GameFreakSnesInstr(VGMInstrSet* instrSet,
  uint32_t offset,
  uint32_t theBank,
  uint32_t theInstrNum,
  uint8_t srcn,
  uint32_t spcDirAddr,
  const std::wstring& name) :
  VGMInstr(instrSet, offset, 4, theBank, theInstrNum, name), srcn(srcn),
  spcDirAddr(spcDirAddr) {
}

GameFreakSnesInstr::~GameFreakSnesInstr() {
}

bool GameFreakSnesInstr::LoadInstr() {
  uint32_t offDirEnt = spcDirAddr + (srcn * 4);
  if (offDirEnt + 4 > 0x10000) {
    return false;
  }

  uint16_t addrSampStart = GetShort(offDirEnt);

  GameFreakSnesRgn* rgn = new GameFreakSnesRgn(this, dwOffset, srcn);
  rgn->sampOffset = addrSampStart - spcDirAddr;
  aRgns.push_back(rgn);

  return true;
}

// ***********
// GameFreakSnesRgn
// ***********

GameFreakSnesRgn::GameFreakSnesRgn(GameFreakSnesInstr* instr, uint32_t offset, uint8_t srcn) :
  VGMRgn(instr, offset, 4) {
  uint8_t tuning = GetByte(offset + 1);
  uint8_t volLR = GetByte(offset + 5);
  uint8_t defaultTranspose = GetByte(offset + 7);

  AddSimpleItem(offset, 1, L"Transpose");
  AddFineTune(tuning, offset + 1, 1);
  AddVolume(volLR, offset + 2, 1);
  AddSimpleItem(offset + 3, 1, L"ADSR");

  //EmulateSDSPGAIN(gain, (sl << 8) | 0xff, 0, NULL, &release_time);
}

GameFreakSnesRgn::~GameFreakSnesRgn() {
}

bool GameFreakSnesRgn::LoadRgn() {
  return true;
}

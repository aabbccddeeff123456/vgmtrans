#include "pch.h"
#include "GameFreakSnesScanner.h"
#include "GameFreakSnesSeq.h"
#include "GameFreakSnesInstr.h"

//; Bushi Seiryuuden - Futari no Yuusha SPC
//03bd: 8d 00     mov   y, #$00
//03bf: f7 00     mov   a, ($00)+y
//03c1 : c4 22     mov   $22, a
//03c3 : fc        inc   y
//03c4 : f7 00     mov   a, ($00)+y
//03c6 : c4 23     mov   $23, a
//03c8 : ba 22     movw  ya, $22
//03ca : 7a 02     addw  ya, $02
//03cc : da 22     movw  $22, ya
//03ce : 8d 00     mov   y, #$00
//03d0: f7 22     mov   a, ($22)+y
//03d2 : 10 07     bpl   $03db
//03d4 : 8f 01 7c  mov   $7c, #$01
//03d7: 28 7f and a, #$7f
//03d9: 2f 03     bra   $03de
BytePattern GameFreakSnesScanner::ptnLoadSong(
  "\x8d\x00\xf7\x00\xc4\x22\xfc\xf7"
  "\x00\xc4\x23\xba\x22\x7a\x02\xda"
  "\x22\x8d\x00\xf7\x22\x10\x07\x8f"
  "\x01\x7c\x28\x7f\x2f\x03"
  ,
  "x?x?x?xx"
  "?x?x?x?x"
  "?x?x?x?x"
  "??x?x?"
  ,
  30);

//0a10: 8f 02 00  mov   $00, #$02
//0a13: 8f 10 01  mov   $01, #$10
BytePattern GameFreakSnesScanner::ptnLoadInstrSet(
  "\x8f\x02\x00\x8f\x10\x01"
  ,
  "xxxxxx"
  ,
  6);

void GameFreakSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForGameFreakSnesFromARAM(file);
  }
  else {
    SearchForGameFreakSnesFromROM(file);
  }
  return;
}

void GameFreakSnesScanner::SearchForGameFreakSnesFromARAM(RawFile* file) {
  GameFreakSnesVersion version = GAMEFREAKSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  std::wstringstream desc;
  if (file->SearchBytePattern(ptnLoadSong, ofsLoadSeq)) {
    addrSeqList = file->GetByte(ofsLoadSeq + 5);

    version = GAMEFREAKSNES_MAIN;
  }
  else {
    return;
  }

  uint32_t addrLoadInstr;
  uint16_t addrInstrTable;
  uint32_t ofsSetDIR;
  uint16_t spcDirAddr;

  if (file->SearchBytePattern(ptnLoadInstrSet, addrLoadInstr)) {
    addrInstrTable =
      file->GetByte(addrLoadInstr + 1) | (file->GetByte(addrLoadInstr + 4) << 8);
  }
  else {
    return;
  }

  spcDirAddr = 0x2000;

  uint32_t addrSeqHeader = file->GetShort(addrSeqList);
  GameFreakSnesSeq* newSeq = new GameFreakSnesSeq(file, version, addrSeqHeader, addrInstrTable, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }

  GameFreakSnesInstrSet* newInstrSet = new GameFreakSnesInstrSet(file, addrInstrTable, spcDirAddr);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }
}

void GameFreakSnesScanner::SearchForGameFreakSnesFromROM(RawFile* file) {
}

#include "pch.h"
#include "NinRD1SnesScanner.h"
#include "NinRD1SnesSeq.h"

//; Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo
//0c41: 8d 18     mov   y, #$18
//0c43: e8 7c     mov   a, #$7c
//0c45: 7a 10     addw  ya, $10
//0c47 : 2f 18     bra   $0c61
//0c49 : ad 20     cmp   y, #$20
//0c4b: b0 06     bcs   $0c53
//0c4d : 8d 30     mov   y, #$30; music location - 0x3091
//0c4f: e8 91     mov   a, #$91
//0c51: 2f 0e     bra   $0c61
//0c53 : ad 30     cmp   y, #$30
//0c55: b0 06     bcs   $0c5d
//0c57 : 8d 43     mov   y, #$43
//0c59: e8 b3     mov   a, #$b3
//0c5b: 2f 04     bra   $0c61
//0c5d : 8d 30     mov   y, #$30
//0c5f: e8 91     mov   a, #$91
//0c61: da 10     movw  $10, ya
BytePattern NinRD1SnesScanner::ptnLoadSeq(
  "\x8d\x18\xe8\x7c\x7a\x10\x2f\x18"
  "\xad\x20\xb0\x06\x8d\x30\xe8\x91"
  "\x2f\x0e\xad\x30\xb0\x06\x8d\x43"
  "\xe8\xb3\x2f\x04\x8d\x30\xe8\x91"
  "\xda\x10"
  ,
  "x?x?x?x?"
  "x?x?x?x?"
  "x?x?x?x?"
  "x?x?x?x?"
  "x?"
  ,
  34);

void NinRD1SnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForNinRD1SnesFromARAM(file);
  }
  else {
    SearchForNinRD1SnesFromROM(file);
  }
  return;
}

void NinRD1SnesScanner::SearchForNinRD1SnesFromARAM(RawFile* file) {
  NinRD1SnesVersion version = NINRD1SNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqList = file->GetByte(ofsLoadSeq + 31) | (file->GetByte(ofsLoadSeq + 29) << 8);
    version = NINRD1SNES_MAIN;
  }
  else {
    return;
  }

  NinRD1SnesSeq* newSeq = new NinRD1SnesSeq(file, version, addrSeqList, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void NinRD1SnesScanner::SearchForNinRD1SnesFromROM(RawFile* file) {
}

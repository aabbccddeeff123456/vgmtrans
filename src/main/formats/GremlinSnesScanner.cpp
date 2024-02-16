#include "pch.h"
#include "GremlinSnesScanner.h"
#include "GremlinSnesSeq.h"

// Top Gear 2 SPC
//0BDF  E5 F2 02       mov a, !$02f2
//0BE2  68 FF          cmp a, #$ff
//0BE4  F0 78          beq $0c5e
//0BE6  C4 04          mov $04, a
//0BE8  E8 00          mov a, #$00
//0BEA  C4 00          mov $00, a
//0BEC  E8 15          mov a, #$15
//0BEE  C4 01          mov $01, a
//0BF0  8D 00          mov y, #$00
//0BF2  F7 00          mov a, [$00] + y
//0BF4  C4 06          mov $06, a
//0BF6  FC             inc y
//0BF7  F7 00          mov a, [$00] + y
//0BF9  C4 07          mov $07, a
//0BFB  3A 06          incw $06
//0BFD  1A 06          decw $06
//0BFF  F0 5D          beq $0c5e

BytePattern GremlinSnesScanner::ptnLoadSeq(
  "\xe5\xf2\x02\x68\xff\xf0\x78\xc4"
  "\x04\xe8\x00\xc4\x00\xe8\x15\xc4"
  "\x01\x8d\x00\xf7\x00\xc4\x06\xfc"
  "\xf7\x00\xc4\x07\x3a\x06\x1a\x06"
  "\xf0\x5d"
  ,
  "x??x?x?x"
  "?x?x?x?x"
  "?x?x?x?x"
  "x?x?x?x?"
  "x?"
  ,
  34);

void GremlinSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForGremlinSnesFromARAM(file);
  }
  else {
    SearchForGremlinSnesFromROM(file);
  }
  return;
}

void GremlinSnesScanner::SearchForGremlinSnesFromARAM(RawFile* file) {
  GremlinSnesVersion version = GREMLINSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqList = file->GetShort(ofsLoadSeq + 16);


    uint16_t addrStartLow = file->GetByte(ofsLoadSeq + 10);
    uint16_t addrStartHigh = file->GetByte(ofsLoadSeq + 14);
    uint16_t addrTrackStart = addrStartLow | addrStartHigh << 8;

    addrSeqList = file->GetShort(addrTrackStart) + addrTrackStart;
    version = GREMLINSNES_MAIN;
  }
  else {
    return;
  }

  // TODO: guess song index
  int8_t songIndex = 1;

  uint32_t addrSeqHeader = addrSeqList + songIndex;
  GremlinSnesSeq* newSeq = new GremlinSnesSeq(file, version, addrSeqList, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void GremlinSnesScanner::SearchForGremlinSnesFromROM(RawFile* file) {
}

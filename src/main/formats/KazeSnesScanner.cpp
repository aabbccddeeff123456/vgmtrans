#include "pch.h"
#include "KazeSnesScanner.h"
#include "KazeSnesSeq.h"

//; Dark Law
//0db4: f8 07     mov   x, $07
//0db6 : e4 05     mov   a, $05
//0db8 : 60        clrc
//0db9 : 88 00     adc   a, #$00
//0dbb: c4 05     mov   $05, a
//0dbd : e4 03     mov   a, $03
//0dbf : d5 0d 04  mov   $040d + x, a   ; set track pointer lo to 0x040d
//0dc2 : e4 06     mov   a, $06
//0dc4 : 88 20     adc   a, #$20
//0dc6: c4 06     mov   $06, a
//0dc8 : e4 04     mov   a, $04
//0dca : d5 05 04  mov   $0405 + x, a   ; set track pointer hi to 0x0405
BytePattern KazeSnesScanner::ptnLoadSeq(
  "\xf8\x07\xe4\x05\x60\x88\x00\xc4"
  "\x05\xe4\x03\xd5\x0d\x04\xe4\x06"
  "\x88\x20\xc4\x06\xe4\x04\xd5\x05"
  "\x04"
  ,
  "x?x?xx?x"
  "?x?x??x?"
  "x?x?x?x?"
  "?"
  ,
  25);

void KazeSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForKazeSnesFromARAM(file);
  }
  else {
    SearchForKazeSnesFromROM(file);
  }
  return;
}

void KazeSnesScanner::SearchForKazeSnesFromARAM(RawFile* file) {
  KazeSnesVersion version = KAZESNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqListLo;
  uint16_t addrSeqListHi;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  uint16_t baseAddr;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqListLo = file->GetShort(ofsLoadSeq + 12);
    addrSeqListHi = file->GetShort(ofsLoadSeq + 23);

    uint16_t baseAddrLo = file->GetByte(ofsLoadSeq + 6);
    uint16_t baseAddrHi = file->GetByte(ofsLoadSeq + 17);

    uint16_t addrStartLow = file->GetByte(addrSeqListLo);
    uint16_t addrStartHigh = file->GetByte(addrSeqListHi);    //best solve now
    version = KAZESNES_MAIN;
    addrSeqHeader = addrStartLow | (addrStartHigh << 8);
    baseAddr = baseAddrLo | (baseAddrHi << 8);
  }
  else {
    return;
  }

  KazeSnesSeq* newSeq = new KazeSnesSeq(file, version, 0x2000, addrSeqHeader, baseAddr, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void KazeSnesScanner::SearchForKazeSnesFromROM(RawFile* file) {
}

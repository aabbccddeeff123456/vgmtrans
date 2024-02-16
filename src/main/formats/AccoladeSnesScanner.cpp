#include "pch.h"
#include "AccoladeSnesScanner.h"
#include "AccoladeSnesSeq.h"

// 8d 00 f7 12 28 f0 5c 5c 5c 5d 1f 36 19
BytePattern AccoladeSnesScanner::ptnLoadSeqListPointer(
  //8d 04 f7 41 8d 00 7a 41 d5 28 0d d5 08 0d dd d5
  //29 0d d5 09 0d
  "\x8d\x04\xf7\x41\x8d\x00\x7a\x41"
  "\xd5\x28\x0d\xd5\x08\x0d\xdd\xd5"
  "\x29\x0d\xd5\x09\x0d"
  ,
  "x?x?x?x?"
  "x??x??xx"
  "??x??"
  ,
  21);

void AccoladeSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForAccoladeSnesFromARAM(file);
  }
  else {
    SearchForAccoladeSnesFromROM(file);
  }
  return;
}

void AccoladeSnesScanner::SearchForAccoladeSnesFromARAM(RawFile* file) {
  AccoladeSnesVersion version = ACCOLADESNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint32_t ofsLoadSeqTable;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeqListPointer, ofsLoadSeqTable)) {
    addrSeqList = file->GetShort(ofsLoadSeqTable + 9);
    songIndexMax = 0x10;
    version = ACCOLADESNES_MAIN;
  }
  if (version != ACCOLADESNES_NONE) {
    for (songIndexMax != 0; songIndexMax -= 0x02;) {
      addrSeqHeader = file->GetShort(addrSeqList + songIndexMax);
      if (addrSeqHeader != 0x0000 && addrSeqHeader != 0xffff) {
        AccoladeSnesSeq* newSeq = new AccoladeSnesSeq(file, version, addrSeqHeader, name);
        if (!newSeq->LoadVGMFile()) {
          delete newSeq;
          return;
        }
      }
    }
    addrSeqHeader = file->GetShort(addrSeqList + songIndexMax);
    if (addrSeqHeader != 0x0000 && addrSeqHeader != 0xffff) {
      AccoladeSnesSeq* newSeq = new AccoladeSnesSeq(file, version, addrSeqHeader, name);
      if (!newSeq->LoadVGMFile()) {
        delete newSeq;
        return;
      }
    }
  }

  // TODO: guess song index
  int8_t songIndex = 1;

}

void AccoladeSnesScanner::SearchForAccoladeSnesFromROM(RawFile* file) {
}

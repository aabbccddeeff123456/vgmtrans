#include "pch.h"
#include "SensibleSnesScanner.h"
#include "SensibleSnesSeq.h"

//; Mega-lo-Mania
//0e59: f6 57 13  mov   a, $1357 + y
//0e5c : d4 70     mov   $70 + x, a
//0e5e : f4 90     mov   a, $90 + x
//0e60 : 1c        asl   a
//0e61 : fd        mov   y, a
//0e62 : f6 00 11  mov   a, $1100 + y
//0e65 : c4 0c     mov   $0c, a
//0e67 : f6 01 11  mov   a, $1101 + y
//0e6a : c4 0d     mov   $0d, a
//0e6c : fb 70     mov   y, $70 + x
//0e6e : f7 0c     mov   a, ($0c)+y
//0e70 : c4 09     mov   $09, a
//0e72: 3f ae 0e  call  $0eae
//0e75 : 09 02 04 or ($04), ($02)
//0e78: 6f        ret
BytePattern SensibleSnesScanner::ptnLoadSeq(
  "\xf6\x57\x13\xd4\x70\xf4\x90\x1c"
  "\xfd\xf6\x00\x11\xc4\x0c\xf6\x01"
  "\x11\xc4\x0d\xfb\x70\xf7\x0c\xc4"
  "\x09\x3f\xae\x0e\x09\x02\x04\x6f"
  ,
  "x??x?x?x"
  "xx??x?x?"
  "?x?x?x?x"
  "?x??x??x"
  ,
  32);

//070b: f6 7e 13  mov   a, $137e + y
//070e : c4 05     mov   $05, a
//0710 : f6 7f 13  mov   a, $137f + y
BytePattern SensibleSnesScanner::ptnLoadSeq2(
  "\xf6\x7e\x13\xc4\x05\xf6\x7f\x13"
  ,
  "x??x?x??"
  ,
  8);

void SensibleSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForSensibleSnesFromARAM(file);
  }
  else {
    SearchForSensibleSnesFromROM(file);
  }
  return;
}

void SensibleSnesScanner::SearchForSensibleSnesFromARAM(RawFile* file) {
  SensibleSnesVersion version = SENSIBLESNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSeq;
  uint16_t addrSeqHeader;
  uint32_t addrSeqList;
  int8_t songIndex;
  int8_t songIndexFun;
  int8_t songIndexSec;
  std::wstringstream desc;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    songIndex = file->GetByte(0xf4);
    songIndexFun = songIndex / 2;
    songIndexSec = songIndexFun * 2;
    uint16_t addrStart = 0;
    if (file->SearchBytePattern(ptnLoadSeq2, addrSeqList)) {
      addrStart = file->GetShort(addrSeqList + 1);
    }
    addrSeqHeader = file->GetShort(addrStart + songIndexSec);
  }
  else {
    return;
  }

  version = SENSIBLESNES_MAIN;
  SensibleSnesSeq* newSeq = new SensibleSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void SensibleSnesScanner::SearchForSensibleSnesFromROM(RawFile* file) {
}

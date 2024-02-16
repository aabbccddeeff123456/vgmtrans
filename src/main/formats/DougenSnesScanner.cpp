#include "pch.h"
#include "DougenSnesScanner.h"
#include "DougenSnesSeq.h"

//; Der Langrisser
//0427: 8f 00 e0  mov   $e0,#$00
//042a: 8f 55 e1  mov   $e1,#$55
//042d: fa e0 de  mov   ($de),($e0)
//0430: fa e1 df  mov   ($df),($e1)
//0433: 8d 00     mov   y,#$00
//0435: f7 e0     mov   a,($e0)+y
//0437: c4 ef     mov   $ef,a
//0439: 3a e0     incw  $e0
BytePattern DougenSnesScanner::ptnLoadSeq(
  "\x8f\x00\xe0\x8f\x55\xe1\xfa\xe0"
  "\xde\xfa\xe1\xdf\x8d\x00\xf7\xe0"
  "\xc4\xef\x3a\xe0"
  ,
  "x??x??x?"
  "?x??x?x?"
  "x?x?"
  ,
  20);

void DougenSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForDougenSnesFromARAM(file);
  }
  else {
    SearchForDougenSnesFromROM(file);
  }
  return;
}

void DougenSnesScanner::SearchForDougenSnesFromARAM(RawFile* file) {
  DougenSnesVersion version = DOUGENSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {

    uint16_t addrStartLow = file->GetByte(ofsLoadSeq + 1);
    uint16_t addrStartHigh = file->GetByte(ofsLoadSeq + 4);
    addrSeqList = addrStartLow | addrStartHigh << 8;

    version = DOUGENSNES_MAIN;
  }
  else {
    return;
  }

  uint32_t addrSeqHeader = addrSeqList;
  DougenSnesSeq* newSeq = new DougenSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void DougenSnesScanner::SearchForDougenSnesFromROM(RawFile* file) {
}

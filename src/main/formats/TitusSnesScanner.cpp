#include "pch.h"
#include "TitusSnesScanner.h"
#include "TitusSnesSeq.h"

//; Realm SPC
//; read song header
//05c1: 8d 00     mov   y, #$00
//05c3: cb 84     mov   $84, y
//05c5 : cb 83     mov   $83, y
//05c7 : cb 8a     mov   $8a, y
//05c9 : cb f1     mov   $f1, y
//05cb : f7 e8     mov   a, ($e8)+y; first byte - tempo
//05cd: c4 fa     mov   $fa, a
//05cf : c4 82     mov   $82, a
//05d1 : c4 81     mov   $81, a
//05d3 : e8 03     mov   a, #$03
//05d5: c4 f1     mov   $f1, a
//05d7 : fc        inc   y
//05d8 : f7 e8     mov   a, ($e8)+y; byte 2 - total tracks
//05da: c4 85     mov   $85, a
//05dc : fc        inc   y
//05dd : f7 e8     mov   a, ($e8)+y; read track pointer
//05df: fc        inc   y
//05e0 : 17 e8 or a, ($e8)+y
//05e2 : f0 12     beq   $05f6; 0000 - header end
BytePattern TitusSnesScanner::ptnLoadSong(
  "\x8d\x00\xcb\x84\xcb\x83\xcb\x8a"
  "\xcb\xf1\xf7\xe8\xc4\xfa\xc4\x82"
  "\xc4\x81\xe8\x03\xc4\xf1\xfc\xf7"
  "\xe8\xc4\x85\xfc\xf7\xe8\xfc\x17"
  "\xe8\xf0\x12"
  ,
  "x?x?x?x?"
  "x?x?x?x?"
  "x?x?x?xx"
  "?x?xx?xx"
  "?x?"
  ,
  35);

//8D 00 CB 7A CB F1 F7 E8 C4 FA C4 72 C4 71 E8 03
//C4 F1 FC F7 E8 C4 75 FC F7 E8 FC 17 E8 F0 12

BytePattern TitusSnesScanner::ptnLoadSongVer3(
  "\x8d\x00\xcb\x7a\xcb\xf1\xf7\xe8"
  "\xc4\xfa\xc4\xe2\xc4\x71\xe8\x03"
  "\xc4\xf1\xfc\xf7\xe8\xc4\x75\xfc"
  "\xf7\xe8\xfc\x17\xe8\xf0\x12"
  ,
  "x?x?x?x?"
  "x?x?x?x?"
  "x?xx?x?x"
  "x?xx?x?"
  ,
  31);

void TitusSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForTitusSnesFromARAM(file);
  }
  else {
    SearchForTitusSnesFromROM(file);
  }
  return;
}

void TitusSnesScanner::SearchForTitusSnesFromARAM(RawFile* file) {
  TitusSnesVersion version = TITUSSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  if (file->SearchBytePattern(ptnLoadSong, ofsLoadSeq)) {
    addrSeqList = file->GetByte(ofsLoadSeq + 11);
    version = TITUSSNES_V3_FLAIR;
  } else if (file->SearchBytePattern(ptnLoadSongVer3, ofsLoadSeq)) {
    addrSeqList = file->GetByte(ofsLoadSeq + 7);
    version = TITUSSNES_V3;
  }
  else {
    return;
  }

  uint32_t addrSeqHeader = file->GetShort(addrSeqList);
  TitusSnesSeq* newSeq = new TitusSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void TitusSnesScanner::SearchForTitusSnesFromROM(RawFile* file) {
}

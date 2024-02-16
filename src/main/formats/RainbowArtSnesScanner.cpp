#include "pch.h"
#include "RainbowArtSnesScanner.h"
#include "RainbowArtSnesSeq.h"

//; Rendering Ranger R2
//07ed: 8d 00     mov   y, #$00
//07ef: f7 40     mov   a, ($40)+y
//07f1 : 68 ff     cmp   a, #$ff
//07f3: d0 03     bne   $07f8
//07f5 : 5f 98 08  jmp   $0898
//07f8: 5d        mov   x, a
//07f9 : fc        inc   y
//07fa : f7 40     mov   a, ($40)+y
//07fc : 75 d4 0a  cmp   a, $0ad4 + x
//07ff : b0 08     bcs   $0809
//0801: dd        mov   a, y
//0802 : 60        clrc
//0803 : 88 07     adc   a, #$07
//0805: fd        mov   y, a
//0806 : 5f ef 07  jmp   $07ef
//0809 : d5 d4 0a  mov   $0ad4 + x, a
//080c : 3f 99 08  call  $0899
//080f: e4 50     mov   a, $50
//0811 : d5 cc 09  mov   $09cc + x, a
//0814: e4 51     mov   a, $51
//0816 : d5 d4 09  mov   $09d4 + x, a
BytePattern RainbowArtSnesScanner::ptnLoadSeq(
  "\x8d\x00\xf7\x40\x68\xff\xd0\x03"
  "\x5f\x98\x08\x5d\xfc\xf7\x40\x75"
  "\xd4\x0a\xb0\x08\xdd\x60\x88\x07"
  "\xfd\x5f\xef\x07\xd5\xd4\x0a\x3f"
  "\x99\x08\xe4\x50\xd5\xcc\x09\xe4"
  "\x51\xd5\xd4\x09"
  ,
  "x?x?x?x?"
  "x??xxx?x"
  "??x?xxx?"
  "xx??x??x"
  "??x?x??x"
  "?x??"
  ,
  44);

void RainbowArtSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForRainbowArtSnesFromARAM(file);
  }
  else {
    SearchForRainbowArtSnesFromROM(file);
  }
  return;
}

void RainbowArtSnesScanner::SearchForRainbowArtSnesFromARAM(RawFile* file) {
  RainbowArtSnesVersion version = RAINBOWARTSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    songIndexMax = file->GetByte(ofsLoadSeq + 3);
    addrSeqHeader = file->GetShort(songIndexMax);
    version = RAINBOWARTSNES_MAIN;
  }
  else {
    return;
  }

  // TODO: guess song index
  int8_t songIndex = 1;

  
  RainbowArtSnesSeq* newSeq = new RainbowArtSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void RainbowArtSnesScanner::SearchForRainbowArtSnesFromROM(RawFile* file) {
}

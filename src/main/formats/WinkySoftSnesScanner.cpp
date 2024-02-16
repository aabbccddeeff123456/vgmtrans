#include "pch.h"
#include "WinkySoftSnesScanner.h"
#include "WinkySoftSnesSeq.h"
#include "WinkySoftSnesInstr.h"

//Jojo no Kimyou na Bouken SPC
//f688: a8 08     sbc   a, #$08
//f68a : 65 00 1d  cmp   a, $1d00
//f68d : b0 b1     bcs   $f640
//f68f : 8d 06     mov   y, #$06
//f691 : cf        mul   ya
//f692 : 7a 0c     addw  ya, $0c
//f694 : da 08     movw  $08, ya
BytePattern WinkySoftSnesScanner::ptnLoadSongOLD(
  "\xa8\x08\x65\x00\x1d\xb0\xb1\x8d"
  "\x06\xcf\x7a\x0c\xda\x08"
  ,
  "x?x??x?x"
  "?xx?x?"
  ,
  14);

//Dai-3-Ji Super Robot Taisen SPC
//fbe5: 8d 06     mov   y,#$06
//fbe7: e4 18     mov   a, $18
//fbe9 : cf        mul   ya
//fbea : 7a 0c     addw  ya, $0c
//fbec : da 08     movw  $08, ya
//fbee : 8d 00     mov   y, #$00
//fbf0 : cb 0f     mov   $0f, y
//fbf2 : cb 0e     mov   $0e, y
//fbf4 : f7 08     mov   a, ($08)+y
//fbf6 : f0 21     beq   $fc19
//fbf8 : 64 1d     cmp   a, $1d
//fbfa : 90 63     bcc   $fc5f
//fbfc : c4 1d     mov   $1d, a
BytePattern WinkySoftSnesScanner::ptnLoadSongSRT3(
  "\x8d\x06\xe4\x18\xcf\x7a\x0c\xda"
  "\x08\x8d\x00\xcb\x0f\xcb\x0e\xf7"
  "\x08\xf0\x21\x64\x1d\x90\x63\xc4"
  "\x1d"
  ,
  "x?x?xx?x"
  "?x?x?x?x"
  "?x?x?x?x"
  "?"
  ,
  25);

//Dai-4-Ji Super Robot Taisen SPC
//fa74: 8d 52     mov   y,#$52
//fa76: e8 00     mov   a,#$00
//fa78: da 00     movw  $00,ya
//fa7a: 8f ff 08  mov   $08,#$ff
//fa7d: 8f ff 09  mov   $09,#$ff
//fa80: 0b 18     asl   $18
//fa82: f8 18     mov   x, $18
//fa84 : f5 00 08  mov   a, $0800 + x
//fa87 : c4 21     mov   $21, a
//fa89 : c4 25     mov   $25, a
//fa8b : c4 29     mov   $29, a
//fa8d : f5 01 08  mov   a, $0801 + x
//fa90 : c4 22     mov   $22, a
//fa92 : c4 30     mov   $30, a
BytePattern WinkySoftSnesScanner::ptnLoadSongSRT4(
  "\x8d\x52\xe8\x00\xda\x00\x8f\xff"
  "\x08\x8f\xff\x09\x0b\x18\xf8\x18"
  "\xf5\x00\x08\xc4\x21\xc4\x25\xc4"
  "\x29\xf5\x01\x08\xc4\x22\xc4\x30"
  ,
  "x?x?x?x?"
  "?x??x?x?"
  "x??x?x?x"
  "?x??x?x?"
  ,
  32);

//Dai-3-Ji Super Robot Taisen SPC
//f51e : 8d 02     mov   y, #$02
//f520 : e8 00     mov   a, #$00
//f522 : da 0a     movw  $0a, ya
//f524 : 8d f1     mov   y, #$f1
//f526 : e8 80     mov   a, #$80
//f528 : da 0c     movw  $0c, ya
BytePattern WinkySoftSnesScanner::ptnLoadInstrSet(
  "\x8d\x02\xe8\x00\xda\x0a\x8d\xf1"
  "\xe8\x80\xda\x0c"
  ,
  "x?x?x?x?"
  "x?x?"
  ,
  12);

//Dai-4-Ji Super Robot Taisen SPC
//0d 2c 3c 6c 4c 5c 3d 2d 5d 6d 7d 0c 1c
//00 00 00 20 00 00 00 00 06 f1 00 7f 7f
BytePattern WinkySoftSnesScanner::ptnSetDIR(
  "\x0d\x2c\x3c\x6c\x4c\x5c\x3d\x2d"
  "\x5d\x6d\x7d\x0c\x1c\x00\x00\x00"
  "\x20\x00\x00\x00\x00\x06\xf1\x00"
  "\x7f\x7f"
  ,
  "xxxxxxxx"
  "xxxxx???"
  "????????"
  "??"
  ,
  26);

void WinkySoftSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForWinkySoftSnesFromARAM(file);
  }
  else {
    SearchForWinkySoftSnesFromROM(file);
  }
  return;
}

void WinkySoftSnesScanner::SearchForWinkySoftSnesFromARAM(RawFile* file) {
  WinkySoftSnesVersion version = WINKYSOFTSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSeq;
  uint16_t addrSeqHeader;
  uint32_t addrSeqList;
  int8_t songIndex;
  int8_t songIndexFun;
  int8_t songIndexSec;
  uint16_t addrInstrTable;
  uint32_t ofsSetDIR;
  uint16_t spcDirAddr;
  std::wstringstream desc;
  if (file->SearchBytePattern(ptnLoadSongOLD, ofsLoadSeq)) {
    songIndex = file->GetByte(ofsLoadSeq + 8);
    version = WINKYSOFTSNES_OLD;
    addrSeqHeader = songIndex << 8;
  }
  else if (file->SearchBytePattern(ptnLoadSongSRT3, ofsLoadSeq)) {
    songIndex = file->GetByte(ofsLoadSeq + 1);
    version = WINKYSOFTSNES_SRT3;
    addrSeqHeader = songIndex << 8;
  }
  else if (file->SearchBytePattern(ptnLoadSongSRT4, ofsLoadSeq)) {
    songIndex = file->GetByte(ofsLoadSeq + 1);
    version = WINKYSOFTSNES_SRT4;
    addrSeqHeader = songIndex << 8;
  }
  else {
    return;
  }

  //version = WinkySoftSNES_MAIN;
  WinkySoftSnesSeq* newSeq = new WinkySoftSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }

  if (file->SearchBytePattern(ptnLoadInstrSet, addrSeqList)) {
    addrInstrTable =
      file->GetByte(addrSeqList + 3) | (file->GetByte(addrSeqList + 1) << 8);
  }
  else {
    return;
  }

  if (file->SearchBytePattern(ptnSetDIR, ofsSetDIR)) {
    spcDirAddr = file->GetByte(ofsSetDIR + 22) << 8;
  }

  WinkySoftSnesInstrSet* newInstrSet = new WinkySoftSnesInstrSet(file, addrInstrTable, spcDirAddr);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }

}

void WinkySoftSnesScanner::SearchForWinkySoftSnesFromROM(RawFile* file) {
}


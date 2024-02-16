#include "pch.h"
#include "KadokawaSnesFormat.h"
#include "KadokawaSnesSeq.h"
#include "KadokawaSnesInstr.h"

//TODO : Treasure Hunter G

//; Youkai Buster - Ruka no Daibouken SPC
//0830: 3f 04 16 8d 4d 3f 04 16 e8 37 8d 5d 3f 04 16 8d
BytePattern KadokawaSnesScanner::ptnLoadSongOK(
"\x3f\x04\x16\x8d\x4d\x3f\x04\x16"
"\xe8\x37\x8d\x5d\x3f\x04\x16\x8d"
,
"x??xxx??"
"xxxxx??x"
,
16);

// Music Location:
//12e2: 1c        asl a
//12e3: 5d        mov x, a
//12e4: f5 01 18  ;mov a, $1801+x
//12e7: fd        ;mov y, a
//12e8: f5 00 18  ;mov a, $1800+x
//12eb: da 06     movw $06, ya
//12ed: e8 00     mov a, #$00
//12ef: c4 1a     mov $1a, a
//12f1: c4 1b     mov $1b, a
BytePattern KadokawaSnesScanner::ptnSongLocationOK(
  "\x1c\x5d\xf5\x01\x18\xfd\xf5\x00"
  "\x18\xda\x06\xe8\x00\xc4\x1a\xc4"
  "\x1b"
  ,
  "xxx??xx?"
  "?x?x?x?x"
  "?"
  ,
  17);

// InstrSet Location:
//7078: 3f 00 36 ;call $3600
BytePattern KadokawaSnesScanner::ptnInstrLocationOK(
  "\x3f\x00\x36"
  ,
  "xxx"
  ,
  3);

// DIR Set
//0838: e8 37 8d 5d
BytePattern KadokawaSnesScanner::ptnDIRLocationOK(
  "\xe8\x37\x8d\x5d"
  ,
  "xxxx"
  ,
  4);

// Music Location:
//; a < 0x10
//  12c4: 8f 00 c2  mov   $c2, #$00
//  12c7: 8f 3a c3  mov   $c3, #$3a; song list ptr $3a00
//  12ca: cd 00     mov   x, #$00
//  12cc: d8 02     mov   $02, x
//  12ce : d8 06     mov   $06, x
//  12d0 : d8 03     mov   $03, x
//  12d2 : d8 04     mov   $04, x
//  12d4 : d8 0f     mov   $0f, x
//  12d6 : 23 d0 03  bbs1  $d0, $12dc
//  12d9 : 8f ff 10  mov   $10, #$ff
BytePattern KadokawaSnesScanner::ptnLoadSongStingSquare(
  "\x8f\x00\xc2\x8f\x3a\xc3\xcd\x00"
  "\xd8\x02\xd8\x06\xd8\x03\xd8\x04"
  "\xd8\x0f\x23\xd0\x03\x8f\xff\x10"
  ,
  "x?xx?xx?"
  "x?x?x?x?"
  "x?x??x??"
  ,
  24);

void KadokawaSnesScanner::Scan(RawFile *file, void *info) {
uint32_t nFileLength = file->size();
if (nFileLength == 0x10000) {
SearchForKadokawaSnesFromARAM(file);
}
else {
SearchForKadokawaSnesFromROM(file);
}
return;
}

void KadokawaSnesScanner::SearchForKadokawaSnesFromARAM(RawFile* file) {
  KadokawaSnesVersion version = KADOKAWASNES_NONE;

  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSeq;
  uint32_t songLocation;
  uint16_t addrSeqHeader;
  if (file->SearchBytePattern(ptnSongLocationOK, ofsLoadSeq)) {
    uint8_t addrSeqHeaderPtr = file->GetByte(0xf4);
    uint16_t addrSeqPointer = file->GetShort(ofsLoadSeq + 7);
    addrSeqHeader = (addrSeqPointer += addrSeqHeaderPtr * 2);
    version = KADOKAWASNES_OK;
  }
  else if (file->SearchBytePattern(ptnLoadSongStingSquare, ofsLoadSeq)) {
    uint16_t songListPointer  = file->GetByte(ofsLoadSeq + 1) | file->GetByte(ofsLoadSeq + 4) << 8;
    addrSeqHeader = file->GetShort(songListPointer);
    version = STINGSNES_SQUARE;
  }
  else {
    return;
  }

  KadokawaSnesSeq* newSeq = new KadokawaSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
  else {
    return;
  }
/*  uint32_t ofsLoadInstrTableAddressASM;
  uint32_t addrInstrTable;
  uint16_t spcDirAddr = 0;
  if (file->SearchBytePattern(ptnInstrLocationOK, ofsLoadInstrTableAddressASM)) {
    addrInstrTable = file->GetShort(ofsLoadInstrTableAddressASM + 1);
  }
  if (spcDirAddr == 0) {
    uint32_t ofsSetDIR;
    if (file->SearchBytePattern(ptnDIRLocationOK, ofsSetDIR)) {
      spcDirAddr = file->GetByte(ofsSetDIR + 1) << 8;
    }
    else {
      return;
    }
  }
  KadokawaSnesInstrSet *newInstrSet = new KadokawaSnesInstrSet(file, addrInstrTable, spcDirAddr);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }*/
}




void KadokawaSnesScanner::SearchForKadokawaSnesFromROM(RawFile *file) {
}

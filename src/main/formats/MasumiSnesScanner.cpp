#include "pch.h"
#include "MasumiSnesScanner.h"
#include "MasumiSnesSeq.h"

//; Majyuuou SPC
//112e: f7 09     mov   a, ($09)+y
//1130: d4 65     mov   $65 + x, a
//1132 : c4 02     mov   $02, a
//1134 : e8 00     mov   a, #$00
//1136: d4 64     mov   $64 + x, a
//1138 : 8f 01 5b  mov   $5b, #$01
//113b: fc        inc   y
//113c : f8 5d     mov   x, $5d
//113e : f4 64     mov   a, $64 + x
//1140 : 04 5b or a, $5b
//1142 : d4 64     mov   $64 + x, a
//1144 : f7 09     mov   a, ($09)+y
//1146: fc        inc   y
//1147 : 80        setc
//1148 : 84 09     adc   a, $09
//114a: f8 5c     mov   x, $5c
//114c : d4 66     mov   $66 + x, a
BytePattern MasumiSnesScanner::ptnLoadSeq(
  "\xf7\x09\xd4\x65\xc4\x02\xe8\x00"
  "\xd4\x64\x8f\x01\x5b\xfc\xf8\x5d"
  "\xf4\x64\x04\x5b\xd4\x64\xf7\x09"
  "\xfc\x80\x84\x09\xf8\x5c\xd4\x66"
  ,
  "x?x?x?x?"
  "x?x??xx?"
  "x?x?x?x?"
  "xxx?x?x?"
  ,
  32);

//; Feda - The Emblem of Justice
//0a46: 4d        push  x
//0a47 : f5 0b 02  mov   a, $020b + x
//0a4a: 1c        asl   a
//0a4b : fd        mov   y, a
//0a4c : f6 40 0e  mov   a, $0e40 + y
//0a4f : c4 00     mov   $00, a
//0a51 : f6 41 0e  mov   a, $0e41 + y
//0a54 : c4 01     mov   $01, a
//0a56 : 8d 00     mov   y, #$00
//0a58: f7 00     mov   a, ($00)+y
//0a5a : d5 6b 02  mov   $026b + x, a
BytePattern MasumiSnesScanner::ptnLoadSeqEarly(
  "\x4d\xf5\x0b\x02\x1c\xfd\xf6\x40"
  "\x0e\xc4\x00\xf6\x41\x0e\xc4\x01"
  "\x8d\x00\xf7\x00\xd5\x6b\x02"
  ,
  "xx??xxx?"
  "?x?x??x?"
  "x?x?x??"
  ,
  23);

void MasumiSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForMasumiSnesFromARAM(file);
  }
  else {
    SearchForMasumiSnesFromROM(file);
  }
  return;
}

void MasumiSnesScanner::SearchForMasumiSnesFromARAM(RawFile* file) {
  MasumiSnesVersion version = MASUMISNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqList = file->GetByte(ofsLoadSeq + 1);

    version = MASUMISNES_MAIN;

  }
  else if (file->SearchBytePattern(ptnLoadSeqEarly, ofsLoadSeq)) {
    // TODO: Find Song Data from ROM for V1
    addrSeqList = file->GetShort(ofsLoadSeq + 7);

    version = MASUMISNES_V1;
  }
  else {
    return;
  }

  // TODO: guess song index

  addrSeqHeader = file->GetShort(addrSeqList);

  MasumiSnesSeq* newSeq = new MasumiSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void MasumiSnesScanner::SearchForMasumiSnesFromROM(RawFile* file) {

}

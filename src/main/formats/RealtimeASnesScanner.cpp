#include "pch.h"
#include "RealtimeASnesScanner.h"
#include "RealtimeASnesSeq.h"

// From moditied SPC File from COMNERK

// 8d 00 f7 12 28 f0 5c 5c 5c 5d 1f 36 19
BytePattern RealtimeASnesScanner::ptnLoadSeqACIDV(
  "ACID V1.7S (C) 1992"
  ,
  "xxxxxxxxxxxxxxxxxxx"
  ,
  19);

BytePattern RealtimeASnesScanner::ptnLoadSeqC1992(
  "COPYRIGHT (C) 1992"
  ,
  "xxxxxxxxxxxxxxxxxx"
  ,
  18);

BytePattern RealtimeASnesScanner::ptnLoadSeq1992(
  "AUDIO DRIVER (C) 1992"
  ,
  "xxxxxxxxxxxxxxxxxxxxx"
  ,
  21);

BytePattern RealtimeASnesScanner::ptnLoadSeq1993(
  "AUDIO DRIVER (C) 1993"
  ,
  "xxxxxxxxxxxxxxxxxxxxx"
  ,
  21);

//dc e2 db db db db db e8 ef 35 ef 3b 41 45 49 4d
//55 7e 82 96 9c b0 07 07 07 07 07 07 07 07 07 08
//07 08 08 08 08 08 08 08 08 08 08 08
BytePattern RealtimeASnesScanner::ptnWordtrisCheck1992(
  "\x9c\xb0\x07\x07"
  ,
  "xxxx"
  ,
  4);



//5d eb ba f6 20 05 d4 00 d4 0a f6 38 05 d4 01 d4 0b
BytePattern RealtimeASnesScanner::ptnLoadSeqFromTableACIDV(
  "\x5d\xeb\xba\xf6\x20\x05\xd4\x00"
  "\xd4\x0a\xf6\x38\x05\xd4\x01\xd4"
  "\x0b"
  ,
  "xx?x??x?"
  "x?x??x?x"
  "?"
  ,
  17);

//5d eb b9 f6 60 05 d4 00 f6 79 05 d4 01 6f
BytePattern RealtimeASnesScanner::ptnLoadSeqFromTableC1992(
  "\x5d\xeb\xb9\xf6\x60\x05\xd4\x00"
  "\xf6\x79\x05\xd4\x01\x6f"
  ,
  "xx?x??x?"
  "x??x?x"
  ,
  14);

//e4 d2 5d 1c fd f4 3c 5d f5 40 05 d6 00 00 f5 73
//05 d6 01 00 6f
BytePattern RealtimeASnesScanner::ptnLoadSeqFromTable(
  "\xe4\xd2\x5d\x1c\xfd\xf4\x3c\x5d"
  "\xf5\x40\x05\xd6\x00\x00\xf5\x73"
  "\x05\xd6\x01\x00\x6f"
  ,
  "x?xxxx?x"
  "x??x??x?"
  "?x??x"
  ,
  21);

void RealtimeASnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForRealtimeASnesFromARAM(file);
  }
  else {
    SearchForRealtimeASnesFromROM(file);
  }
  return;
}

void RealtimeASnesScanner::SearchForRealtimeASnesFromARAM(RawFile* file) {
  RealtimeASnesVersion version = REALTIMEASNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint32_t ofsLoadSeqTable;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeqFromTableACIDV, ofsLoadSeqTable)) {
    if (file->GetByte(0xf5) >= 0x80 && file->GetByte(0xf6) != 0x01) {
      songIndexMax = 0;
    }
    else {
      songIndexMax = file->GetByte(0xf5);
    }
    addrSeqHeader = file->GetByte(file->GetShort(ofsLoadSeqTable + 4) + songIndexMax) | file->GetByte(file->GetShort(ofsLoadSeqTable + 0x0b) + songIndexMax) << 8;
  }
  else if (file->SearchBytePattern(ptnLoadSeqFromTableC1992, ofsLoadSeqTable)) {
    //if (file->GetByte(0xf5) >= 0x80) {
    //  songIndexMax = 0;
    //}
    //else {
      songIndexMax = file->GetByte(0xf5);
    //}
    addrSeqHeader = file->GetByte(file->GetShort(ofsLoadSeqTable + 4) + songIndexMax) | file->GetByte(file->GetShort(ofsLoadSeqTable + 0x09) + songIndexMax) << 8;
  }
  else if (file->SearchBytePattern(ptnLoadSeqFromTable, ofsLoadSeqTable)) {
    //if (file->GetByte(0xf5) >= 0x80) {
    //  songIndexMax = 0;
    //}
    //else {
      songIndexMax = file->GetByte(0xf5);
    //}
    addrSeqHeader = file->GetByte(file->GetShort(ofsLoadSeqTable + 9) + songIndexMax) | file->GetByte(file->GetShort(ofsLoadSeqTable + 0x0f) + songIndexMax) << 8;
  }
  else {
    return;
  }

  if (file->SearchBytePattern(ptnLoadSeqACIDV, ofsLoadSeq)) {
    version = REALTIMEASNES_ACIDV;
  }
  else if (file->SearchBytePattern(ptnLoadSeqC1992, ofsLoadSeq)) {
    version = REALTIMEASNES_C1992;
    if (file->SearchBytePattern(ptnWordtrisCheck1992, ofsLoadSeq)) {
      version = REALTIMEASNES_C1992_WORDTRIS;
    }
  }
  else if (file->SearchBytePattern(ptnLoadSeq1992, ofsLoadSeq)) {
    version = REALTIMEASNES_1992;
  }
  else if (file->SearchBytePattern(ptnLoadSeq1993, ofsLoadSeq)) {
    version = REALTIMEASNES_1993;
  }

  // TODO: guess song index
  int8_t songIndex = 1;

  RealtimeASnesSeq* newSeq = new RealtimeASnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void RealtimeASnesScanner::SearchForRealtimeASnesFromROM(RawFile* file) {
}

#include "pch.h"
#include "HiroakiYagishitaSnesScanner.h"
#include "HiroakiYagishitaSnesSeq.h"

//; Slayers
  //mov a,!$1315+y
  //mov $00,a
  //mov a,!$1316+y
  //mov $01,a
  //mov y,#$00
  //mov $06,y
  //mov a,[$00]+y
  //bmi $02e5
  //inc y
  //mov x,a
  //mov a,$06
  //or a,!$08f5+x
  //mov $06,a
  //call !$0293
  //jmp !$02d2

BytePattern HiroakiYagishitaSnesScanner::ptnLoadSeq(
  "\xf6\x15\x13\xc4\x00\xf6\x16\x13"
  "\xc4\x01\x8d\x00\xcb\x06\xf7\x00"
  "\x30\x0f\xfc\x5d\xe4\x06\x15\xf5"
  "\x08\xc4\x06\x3f\x93\x02\x5f\xd2"
  "\x02"
  ,
  "x??x?x??"
  "x?x?x?x?"
  "x?xxx?x?"
  "?x?x??x?"
  "?"
  ,
  33);

// Captain Tsubasa J
  //mov a,#$00
  //mov y,#$18
  //movw $02,ya
  //mov y,a
  //mov $04,y
  //mov $09,y
  //mov $07,#$7f
  //mov a,[$02]+y
  //bmi $0338
  //mov x,a
  //call !$037e
BytePattern HiroakiYagishitaSnesScanner::ptnLoadSeqAlt(
  "\xe8\x00\x8d\x18\xda\x02\xfd\xcb"
  "\x04\xcb\x09\x8f\x7f\x07\xf7\x02"
  "\x30\x0b\x5d\x3f\x7e\x03\x04\x04"
  "\xc4\x04\x5f\x29\x03"
  ,
  "x?x?x?xx"
  "?x?x??x?"
  "x?xx??x?"
  "x?x??"
  ,
  29);

void HiroakiYagishitaSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForHiroakiYagishitaSnesFromARAM(file);
  }
  else {
    SearchForHiroakiYagishitaSnesFromROM(file);
  }
  return;
}

void HiroakiYagishitaSnesScanner::SearchForHiroakiYagishitaSnesFromARAM(RawFile* file) {
  HiroakiYagishitaSnesVersion version = HIROAKIYAGISHITASNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqList = file->GetShort(ofsLoadSeq + 1);
    addrSeqHeader = file->GetShort(addrSeqList);

    version = HIROAKIYAGISHITASNES_MAIN;

  } else if (file->SearchBytePattern(ptnLoadSeqAlt, ofsLoadSeq)) {
    addrSeqHeader = file->GetByte(ofsLoadSeq + 1) + (file->GetByte(ofsLoadSeq + 3) * 0x100);

    version = HIROAKIYAGISHITASNES_VER2;

  }
  else {
    return;
  }

  if (version == HIROAKIYAGISHITASNES_MAIN) {
    // check weird minor version
    if (file->GetByte(ofsLoadSeq - 1) == 0x6d) {
      version = HIROAKIYAGISHITASNES_MAIN_SLAYERS;
    } else {
      version = HIROAKIYAGISHITASNES_MAIN_KIDOU;
    }
  }

  // TODO: guess song index

  HiroakiYagishitaSnesSeq* newSeq = new HiroakiYagishitaSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void HiroakiYagishitaSnesScanner::SearchForHiroakiYagishitaSnesFromROM(RawFile* file) {

}

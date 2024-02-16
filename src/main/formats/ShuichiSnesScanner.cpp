#include "pch.h"
#include "ShuichiSnesScanner.h"
#include "ShuichiSnesSeq.h"

//; Down the World - Mervil's Ambition SPC
//0f29: f5 00 12  mov   a, $1200 + x
//0f2c : d5 00 02  mov   $0200 + x, a
//0f2f : f5 08 12  mov   a, $1208 + x
//0f32: d4 0b     mov   $0b + x, a
//0f34: 1d        dec   x
//0f35 : 10 ed     bpl   $0f24
//0f37 : 38 02 95 and $95, #$02
//0f3a: 8f 00 8f  mov   $8f, #$00
//0f3d: 8d 96     mov   y, #$96
//0f3f: 3f 48 0f  call  $0f48
BytePattern ShuichiSnesScanner::ptnLoadSongVER1(
  "\xf5\x00\x12\xd5\x00\x02\xf5\x08"
  "\x12\xd4\x0b\x1d\x10\xed\x38\x02"
  "\x95\x8f\x00\x8f\x8d\x96\x3f\x48"
  "\x0f"
  ,
  "x??x??x?"
  "?x?xx?x?"
  "?x??x?x?"
  "?"
  ,
  25);

void ShuichiSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForShuichiSnesFromARAM(file);
  }
  else {
    SearchForShuichiSnesFromROM(file);
  }
  return;
}

void ShuichiSnesScanner::SearchForShuichiSnesFromARAM(RawFile* file) {
  ShuichiSnesVersion version = SHUICHISNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSongVER1, ofsLoadSeq)) {
    addrSeqHeader = file->GetShort(ofsLoadSeq + 1);
    version = SHUICHISNES_VER1;
  }
  else {
    return;
  }

  ShuichiSnesSeq* newSeq = new ShuichiSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void ShuichiSnesScanner::SearchForShuichiSnesFromROM(RawFile* file) {
}

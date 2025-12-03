#include "pch.h"
#include "DaviddeSnesScanner.h"
#include "DaviddeSnesSeq.h"



BytePattern DaviddeSnesScanner::ptnLoadSeq("\xba\x06\xda\x00\x8d\x00\xcd\x00"
                                           "\x3d\xf7\x00\xfc\x17\x00\xf0\x12"
                                           "\xdc\xf7\x00\x60\x84\x06\xd7\x00"
                                           "\xfc\xf7\x00\x84\x07\xd7\x00\xfc"
                                           "\x2f\xe6"
                                           ,
                                           "x?x?x?x?"
                                           "xx?x??x?"
                                           "x??xx?x?"
                                           "xx?x?x?x"
                                           "x?",
                                           34);

void DaviddeSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForDaviddeSnesFromARAM(file);
  } else {
    SearchForDaviddeSnesFromROM(file);
  }
  return;
}

void DaviddeSnesScanner::SearchForDaviddeSnesFromARAM(RawFile* file) {
  DaviddeSnesVersion version = DAVIDDESNES_NONE;
  std::wstring name =
      file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqList = file->GetShort(file->GetByte(ofsLoadSeq + 1));
    version = DAVIDDESNES_MAIN;
  } else {
    return;
  }

  // TODO: guess song index
  int8_t songIndex = 1;

  uint32_t addrSeqHeader = addrSeqList + songIndex;
  DaviddeSnesSeq* newSeq = new DaviddeSnesSeq(file, version, addrSeqList, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void DaviddeSnesScanner::SearchForDaviddeSnesFromROM(RawFile* file) {
}

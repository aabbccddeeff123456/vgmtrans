#include "pch.h"
#include "InsomnyaSnesScanner.h"
#include "InsomnyaSnesSeq.h"

//; Hysteria (Varient 2)
//;play song
//0606: cd 0f     mov   x,#$0f
//0608: f5 32 08  mov   a,$0832+x   ;set playlist pointer
//060b: d5 20 00  mov   $0020+x,a
//060e: 1d        dec   x
//060f: 10 f7     bpl   $0608
//0611: 8f 00 02  mov   $02,#$00
//0614: cd 00     mov   x,#$00
//0616: f4 20     mov   a,$20+x
//0618: c4 00     mov   $00,a
//061a: f4 21     mov   a,$21+x
//061c: c4 01     mov   $01,a
//061e: f4 20     mov   a,$20+x
//0620: 60        clrc
//0621: 88 02     adc   a,#$02
//0623: d4 20     mov   $20+x,a
//0625: 3d        inc   x
//0626: 90 02     bcc   $062a
BytePattern InsomnyaSnesScanner::ptnLoadSeq(
	"\xcd\x0f\xf5\x32\x08\xd5\x20\x00"
  "\x1d\x10\xf7\x8f\x00\x02\xcd\x00"
  "\xf4\x20\xc4\x00\xf4\x21\xc4\x01"
  "\xf4\x20\x60\x88\x02\xd4\x20\x3d"
  "\x90\x02"
	,
	"x?x??x??"
	"xx?x??x?"
	"x?x?x?x?"
	"x?xx?x?x"
    "x?"
	,
	34);

void InsomnyaSnesScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForInsomnyaSnesFromARAM(file);
  }
  else {
    SearchForInsomnyaSnesFromROM(file);
  }
  return;
}

void InsomnyaSnesScanner::SearchForInsomnyaSnesFromARAM(RawFile *file) {
  InsomnyaSnesVersion version = INSOMNYASNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint16_t addrSeqList;
  uint8_t songIndexMax;
  uint8_t headerAlignSize;
  uint32_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    addrSeqHeader = file->GetShort(ofsLoadSeq + 3);
    version = INSOMNYASNES_MAIN;
  }
  else {
    return;
  }

  // TODO: guess song index
  InsomnyaSnesSeq *newSeq = new InsomnyaSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void InsomnyaSnesScanner::SearchForInsomnyaSnesFromROM(RawFile *file) {
}

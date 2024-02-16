#include "pch.h"
#include "WolfTeamSnesFormat.h"
#include "WolfTeamSnesSeq.h"

//; TOP SPC
//0853: 3f 00 40  call $4000
//0856: 58 80 80  eor $80,#$80
//0859: fa 80 f4  mov ($f4),($80)
//085c: 3f 9f 19  call $199f
//085f: 3f c9 09  call $09c9
//0862: 3f 58 09  call $0958 
//0865: 5f 5c 08  jmp $085c
BytePattern WolfTeamSnesScanner::ptnLoadSongTOP(
  "\x3f\x00\x40\x58\x80\x80\xfa\x80"
  "\xf4\x3f\x9f\x19\x3f\xc9\x09\x3f"
  "\x58\x09\x5f\x5c\x08"
  ,
  "x??x??x?"
  "?x??x??x"
  "??x??"
  ,
  21);

//; Arcus Spirits
//0839: 2d        push  a
//083a: 4d        push  x
//083b: 6d        push  y
//083c: 8d 23     mov   y,#$23
//083e: cd 00     mov   x,#$00
//0840: 8f 0b ed  mov   $ed,#$0b
//0843: f6 00 18  mov   a,$1800+y   ; Goto Music Address
//0846: fc        inc   y
//0847: af        mov   (x)+,a
BytePattern WolfTeamSnesScanner::ptnLoadSongOLD(
  "\x2d\x4d\x6d\x8d\x23\xcd\x00\x8f"
  "\x0b\xed\xf6\x00\x18\xfc\xaf"
  ,
  "xxxx?x?x"
  "??x??xx"
  ,
  15);

//; Zan 2 Spirits
//; Header Signature
//0400: 2f 38 24 48 65 61 64 65 72 3a 20 5f 4b 41 4e 2e
//0410: 41 53 76 20 20 31 2e 31 20 20 39 31 2f 31 32 2f
//0420: ...
BytePattern WolfTeamSnesScanner::ptnLoadSongZAN2(
  "\x2f\x38\x24\x48\x65\x61\x64\x65"
  "\x72\x3a\x20\x5f\x4b\x41\x4e\x2e"
  "\x41\x53\x76\x20\x20\x31\x2e\x31"
  "\x20\x20\x39\x31\x2f\x31\x32\x2f"
  ,
  "xxxxxxxx"
  "xxxxxxxx"
  "????????"
  "????????"
  ,
  32);

//; Zan 3 Spirits
//0b50: 3f 9d 1b e4 fe f0 03 3f 1b 0c e4 fd d0 03 5f 50

BytePattern WolfTeamSnesScanner::ptnLoadSongMID(
  "\x3f\x9d\x1b\xe4\xfe\xf0\x03\x3f"
  "\x1b\x0c\xe4\xfd\xd0\x03\x5f\x50"
  ,
  "x??x?x?x"
  "??x?x?x?"
  ,
  16);

void WolfTeamSnesScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForWolfTeamSnesFromARAM(file);
  }
  else {
    SearchForWolfTeamSnesFromROM(file);
  }
  return;
}

void WolfTeamSnesScanner::SearchForWolfTeamSnesFromARAM(RawFile *file) {
WolfTeamSnesVersion version = WOLFTEAMSNES_NONE;

//wstring basefilename = RawFile::removeExtFromPath(file->GetFileName());
//wstring newName = file->tag.HasTitle() ? file->tag.title : basefilename;

std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

uint32_t ofsLoadSeq;
uint16_t addrSeqHeader;
if (file->SearchBytePattern(ptnLoadSongTOP, ofsLoadSeq)) {
  uint8_t addrSeqHeaderPtr = file->GetByte(0x855);
  addrSeqHeader = file->GetShort(0x854);
/*  if (addrSeqHeader == 0x4000) {
    version = WOLFTEAMSNES_TOP;
  }
  else if (addrSeqHeader == 0x3800) {
    version = WOLFTEAMSNES_SO;
  }
  else if (addrSeqHeader == 0x3000) {
    version = WOLFTEAMSNES_TENSHI;
  }
  else {*/
    version = WOLFTEAMSNES_LATER;
 // }
}
else if (file->SearchBytePattern(ptnLoadSongOLD, ofsLoadSeq)) {
  uint8_t addrSeqHeaderPtr = file->GetByte(0x845);
  addrSeqHeader = file->GetShort(ofsLoadSeq + 0x0b);
    version = WOLFTEAMSNES_EARLIER;
}
else if (file->SearchBytePattern(ptnLoadSongZAN2, ofsLoadSeq)) {
  uint8_t addrSeqHeaderPtr = file->GetByte(0xb07);
  addrSeqHeader = file->GetShort(0xb07);
  version = WOLFTEAMSNES_ZAN2;
}
else if (file->SearchBytePattern(ptnLoadSongMID, ofsLoadSeq)) {
  uint8_t addrSeqHeaderPtr = file->GetByte(0xad);
  addrSeqHeader = addrSeqHeaderPtr * 0x100;
  version = WOLFTEAMSNES_MID;
}
else {
return;
}

WolfTeamSnesSeq *newSeq = new WolfTeamSnesSeq(file, version, addrSeqHeader, name);
if (!newSeq->LoadVGMFile()) {
delete newSeq;
return;
}
else {
  return;
}
}

void WolfTeamSnesScanner::SearchForWolfTeamSnesFromROM(RawFile *file) {
}

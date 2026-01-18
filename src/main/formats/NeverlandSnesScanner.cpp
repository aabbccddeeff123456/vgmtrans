#include "pch.h"
#include "NeverlandSnesFormat.h"
#include "NeverlandSnesSeq.h"
#include "NeverlandSnesInstr.h"

//; Lufia SPC
//16c3: 8f 10 08  mov   $08,#$10
//16c6: 8f 28 09  mov   $09,#$28
//16c9: cd 04     mov   x,#$04
//16cb: e8 00     mov   a,#$00
//16cd: d5 18 03  mov   $0318+x,a
//16d0: 3d        inc   x
//16d1: c8 08     cmp   x,#$08
//16d3: d0 f8     bne   $16cd
//16d5: 8d 00     mov   y,#$00
//16d7: f7 08     mov   a,($08)+y
//16d9: d6 00 02  mov   $0200+y,a
//16dc: 10 0e     bpl   $16ec
BytePattern NeverlandSnesScanner::ptnLoadSongSFC(
	"\x8f\x10\x08\x8f\x28\x09\xcd\x04"
	"\xe8\x00\xd5\x18\x03\x3d\xc8\x08"
	"\xd0\xf8\x8d\x00\xf7\x08\xd6\x00"
	"\x02\x10\x0e"
	,
	"xx?x??xx"
	"xxx??xxx"
	"xxxxx?x?"
	"?x?"
	,
	27);

//; Lufia II SPC
//356e: 8f 10 18  mov   $18,#$10
//3571: 8f 44 19  mov   $19,#$44
//3574: cd 00     mov   x,#$00
//3576: 8d 00     mov   y,#$00
//3578: f4 3d     mov   a,$3d+x
//357a: 28 02     and   a,#$02
//357c: d4 3d     mov   $3d+x,a
//357e: f7 18     mov   a,($18)+y
//3580: 68 ff     cmp   a,#$ff
//3582: f0 08     beq   $358c
BytePattern NeverlandSnesScanner::ptnLoadSongS2C(
	"\x8f\x10\x18\x8f\x44\x19\xcd\x00"
	"\x8d\x00\xf4\x3d\x28\x02\xd4\x3d"
	"\xf7\x18\x68\xff\xf0\x08"
	,
	"xx?x??xx"
	"xxx?xxx?"
	"x?xxx?"
	,
	22);

//; Lufia II SPC
//421f: e4 10     mov   a,$10
//4221: fa 10 f3  mov   ($f3),($10)       ; set ADSR
//4224: 1c        asl   a
//4225: 1c        asl   a                 ; index = SRCN * 4
//4226: fd        mov   y,a
//4227: f6 d2 08  mov   a,$08d2+y         ; read from instrument table
//422a: ab f2     inc   $f2
//422c: c4 f3     mov   $f3,a             ; offset +0: ADSR(1)
//422e: f6 d3 08  mov   a,$08d3+y
//4231: ab f2     inc   $f2
//4233: c4 f3     mov   $f3,a             ; offset +1: ADSR(2)
//4235: f6 d4 08  mov   a,$08d4+y
//4238: d5 93 08  mov   $0893+x,a         ; offset +2: pitch multiplier
//423b: f6 d5 08  mov   a,$08d5+y
//423e: d5 a3 08  mov   $08a3+x,a         ; offset +3: pitch multiplier
//4241: 6f        ret
BytePattern NeverlandSnesScanner::ptnLoadInstrS2C(
	"\xe4\x10\xfa\x10\xf3\x1c\x1c\xfd"
  "\xf6\xd2\x08\xab\xf2\xc4\xf3\xf6"
  "\xd3\x08\xab\xf2\xc4\xf3\xf6\xd4"
  "\x08\xd5\x93\x08\xf6\xd5\x08\xd5"
  "\xa3\x08\x6f"
	,
	"x?x??xxx"
  "x??x?x?x"
  "??x?x?x?"
  "?x??x??x"
  "??x"
	,
	35);

//; Energy Breaker SPC
//201b: f4 3d     mov   a,$3d+x
//201d: 28 02     and   a,#$02
//201f: f0 20     beq   $2041
//2021: f5 ef 06  mov   a,$06ef+x
//2024: 1c        asl   a
//2025: 1c        asl   a
//2026: fd        mov   y,a
//2027: f6 de 08  mov   a,$08de+y
//202a: d5 bf 08  mov   $08bf+x,a
//202d: f6 df 08  mov   a,$08df+y
//2030: d5 c7 08  mov   $08c7+x,a
//2033: f6 e0 08  mov   a,$08e0+y
//2036: d5 8f 08  mov   $088f+x,a
//2039: f6 e1 08  mov   a,$08e1+y
//203c: d5 9f 08  mov   $089f+x,a
BytePattern NeverlandSnesScanner::ptnLoadInstrEB(
	"\xf4\x3d\x28\x02\xf0\x20\xf5\xef"
  "\x06\x1c\x1c\xfd\xf6\xde\x08\xd5"
  "\xbf\x08\xf6\xdf\x08\xd5\xc7\x08"
  "\xf6\xe0\x08\xd5\x8f\x08\xf6\xe1"
  "\x08\xd5\x9f\x08"
	,
  "x?x?x?x?"
  "?xxxx??x"
  "??x??x??"
  "x??x??x?"
  "?x??"
	,
	36);

//; Lufia SPC
//1edd: 1c        asl   a
//1ede: 1c        asl   a
//1edf: 60        clrc
//1ee0: 88 00     adc   a,#$00
//1ee2: c4 18     mov   $18,a
//1ee4: e8 00     mov   a,#$00
//1ee6: 88 61     adc   a,#$61
//1ee8: c4 19     mov   $19,a
//1eea: 8d 00     mov   y,#$00
//1eec: f7 18     mov   a,($18)+y
//1eee: fc        inc   y
//1eef: 3d        inc   x
//1ef0: d8 f2     mov   $f2,x
//1ef2: c4 f3     mov   $f3,a             ; set ADSR(1)
//1ef4: f7 18     mov   a,($18)+y
//1ef6: fc        inc   y
//1ef7: 3d        inc   x
//1ef8: d8 f2     mov   $f2,x
//1efa: c4 f3     mov   $f3,a             ; set ADSR(2)
//1efc: f8 0d     mov   x,$0d
//1efe: f7 18     mov   a,($18)+y
//1f00: d5 08 03  mov   $0308+x,a
//1f03: fc        inc   y
//1f04: f7 18     mov   a,($18)+y
//1f06: d5 10 03  mov   $0310+x,a
//1f09: ee        pop   y
//1f0a: 6f        ret
BytePattern NeverlandSnesScanner::ptnLoadInstrSFC(
	"\x1c\x1c\x60\x88\x00\xc4\x18\xe8"
  "\x00\x88\x61\xc4\x19\x8d\x00\xf7"
  "\x18\xfc\x3d\xd8\xf2\xc4\xf3\xf7"
  "\x18\xfc\x3d\xd8\xf2\xc4\xf3\xf8"
  "\x0d\xf7\x18\xd5\x08\x03\xfc\xf7"
  "\x18\xd5\x10\x03\xee\x6f"
	,
	"xxxx?x?x"
  "?x?x?x?x"
  "?xxx?x?x"
  "?xxx?x?x"
  "?x?x??xx"
  "?x??xx"
	,
	46);

void NeverlandSnesScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForNeverlandSnesFromARAM(file);
  }
  else {
    SearchForNeverlandSnesFromROM(file);
  }
  return;
}

void NeverlandSnesScanner::SearchForNeverlandSnesFromARAM(RawFile *file) {
  NeverlandSnesVersion version = NEVERLANDSNES_NONE;

  std::wstring basefilename = RawFile::removeExtFromPath(file->GetFileName());
  std::wstring name = file->tag.HasTitle() ? file->tag.title : basefilename;

  uint32_t ofsLoadSong;
  uint16_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSongS2C, ofsLoadSong)) {
    addrSeqHeader = file->GetByte(ofsLoadSong + 4) << 8;
    version = NEVERLANDSNES_S2C;
  }
  else if (file->SearchBytePattern(ptnLoadSongSFC, ofsLoadSong)) {
    addrSeqHeader = file->GetByte(ofsLoadSong + 4) << 8;
    version = NEVERLANDSNES_SFC;
  }
  else {
    return;
  }

  NeverlandSnesSeq *newSeq = new NeverlandSnesSeq(file, version, addrSeqHeader);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }

  uint32_t LoadDSP;
  uint32_t ofsDSP;
  uint32_t LoadInstr;
  uint16_t instrTable;
  // fixed...?
  if (version == NEVERLANDSNES_SFC) {
    ofsDSP = 0x6000;
  } else if (version == NEVERLANDSNES_S2C) {
    ofsDSP = 0x0200;
  }
  if (file->SearchBytePattern(ptnLoadInstrS2C, LoadInstr)) {
    instrTable = file->GetShort(LoadInstr + 9);
  } else if (file->SearchBytePattern(ptnLoadInstrSFC, LoadInstr)) {
    instrTable = 0x6100;  // fixed?
  } else if (file->SearchBytePattern(ptnLoadInstrEB, LoadInstr)) {
    instrTable = file->GetShort(LoadInstr + 13);
  }

  NeverlandSnesInstrSet* newInstrSet = new  NeverlandSnesInstrSet(file, version, ofsDSP, instrTable);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }
}

void NeverlandSnesScanner::SearchForNeverlandSnesFromROM(RawFile *file) {
}

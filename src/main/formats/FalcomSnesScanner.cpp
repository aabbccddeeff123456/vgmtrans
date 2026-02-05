#include "pch.h"
#include "FalcomSnesScanner.h"
#include "FalcomSnesSeq.h"
#include "FalcomSnesInstr.h"

//; Ys V: Ushinawareta Suna no Miyako Kefin SPC
//0c05: 4b 67     lsr   $67
//0c07: f7 b9     mov   a,($b9)+y
//0c09: d4 73     mov   $73+x,a           ; load voice address (lo)
//0c0b: c4 00     mov   $00,a
//0c0d: fc        inc   y
//0c0e: f7 b9     mov   a,($b9)+y
//0c10: c4 01     mov   $01,a
//0c12: 60        clrc
//0c13: 84 ba     adc   a,$ba             ; convert offset to address
//0c15: d4 7d     mov   $7d+x,a           ; load voice address (hi)
//0c17: 09 01 00  or    ($00),($01)
//0c1a: f0 03     beq   $0c1f             ; offset 0 = unused track
//0c1c: 18 80 67  or    $67,#$80          ; indicate track availability
//0c1f: fc        inc   y
//0c20: 3d        inc   x
//0c21: c8 08     cmp   x,#$08
//0c23: d0 e0     bne   $0c05
BytePattern FalcomSnesScanner::ptnLoadSeq(
	"\x4b\x67\xf7\xb9\xd4\x73\xc4\x00"
	"\xfc\xf7\xb9\xc4\x01\x60\x84\xba"
	"\xd4\x7d\x09\x01\x00\xf0\x03\x18"
	"\x80\x67\xfc\x3d\xc8\x08\xd0\xe0"
	,
	"x?x?x?xx"
	"xx?xxxx?"
	"x?xxxxxx"
	"x?xxxxxx"
	,
	32);

//0cba: f6 00 1c  mov   a,$1c00+y
//0cbd: d5 78 01  mov   $0178+x,a         ; load voice address (lo)
//0cc0: fc        inc   y
//0cc1: f6 00 1c  mov   a,$1c00+y
//0cc4: 60        clrc
//0cc5: 88 1c     adc   a,#$1c            ; convert offset to address (base 0x1c00)
//0cc7: d5 82 01  mov   $0182+x,a         ; load voice address (hi)
//0cca: fc        inc   y
//0ccb: 3f ae 0e  call  $0eae
//0cce: 3d        inc   x
//0ccf: c8 0a     cmp   x,#$0a
//0cd1: 90 e0     bcc   $0cb3
BytePattern FalcomSnesScanner::ptnLoadSeqPM(
	"\xf6\x00\x1c\xd5\x78\x01\xfc\xf6"
  "\x00\x1c\x60\x88\x1c\xd5\x82\x01"
  "\xfc\x3f\xae\x0e\x3d\xc8\x0a\x90"
  "\xe0"
	,
	"x??x??xx"
  "??xx?x??"
  "xx??xx?x"
  "?"
	,
	25);

//; Ys V: Ushinawareta Suna no Miyako Kefin SPC
//0b2c: e8 1b     mov   a,#$1b
//0b2e: 8f 5d f2  mov   $f2,#$5d
//0b31: c4 f3     mov   $f3,a             ; DIR = 0x1b00
BytePattern FalcomSnesScanner::ptnSetDIR(
	"\xe8\x1b\x8f\x5d\xf2\xc4\xf3"
	,
	"x?xxxxx"
	,
	7);

//0344: e8 30     mov   a,#$30
//0346: 8d 5d     mov   y,#$5d
//0348: 3f a2 0e  call  $0ea2
BytePattern FalcomSnesScanner::ptnSetDIRPM(
	"\xe8\x30\x8d\x5d\x3f\xa2\x0e"
	,
	"x?xxxxx"
	,
	7);

//; Ys V: Ushinawareta Suna no Miyako Kefin SPC
//1446: cd 00     mov   x,#$00
//1448: 75 7c 0b  cmp   a,$0b7c+x         ; read instrument-SRCN lookup table
//144b: f0 03     beq   $1450             ; break if the instrument found (x=SRCN)
//144d: 3d        inc   x
//144e: 2f f8     bra   $1448
//1450: 8d 05     mov   y,#$05
//1452: cf        mul   ya
//1453: fd        mov   y,a
//1454: 7d        mov   a,x
//1455: f8 aa     mov   x,$aa
//1457: d5 28 02  mov   $0228+x,a         ; SRCN
//145a: f6 95 11  mov   a,$1195+y
BytePattern FalcomSnesScanner::ptnLoadInstr(
	"\xcd\x00\x75\x7c\x0b\xf0\x03\x3d"
	"\x2f\xf8\x8d\x05\xcf\xfd\x7d\xf8"
	"\xaa\xd5\x28\x02\xf6\x95\x11"
	,
	"xxx??xxx"
	"xxxxxxxx"
	"?x??x??"
	,
	23);

//09a7: 5d        mov   x,a
//09a8: 3f 5a 09  call  $095a
//09ab: d5 6e 01  mov   $016e+x,a
//09ae: 8d 06     mov   y,#$06
//09b0: cf        mul   ya
//09b1: da 05     movw  $05,ya
//09b3: 60        clrc
//09b4: 98 47 05  adc   $05,#$47
//09b7: 98 0f 06  adc   $06,#$0f
//09ba: 3f 35 0c  call  $0c35
//09bd: b0 38     bcs   $09f7
BytePattern FalcomSnesScanner::ptnLoadInstrPM(
	"\x5d\x3f\x5a\x09\xd5\x6e\x01\x8d"
  "\x06\xcf\xda\x05\x60\x98\x47\x05"
  "\x98\x0f\x06\x3f\x35\x0c\xb0\x38"
	,
	"xx??x??x"
  "?xx?xx??"
  "x??x??x?"
	,
	24);

void FalcomSnesScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForFalcomSnesFromARAM(file);
  }
  else {
    SearchForFalcomSnesFromROM(file);
  }
  return;
}

void FalcomSnesScanner::SearchForFalcomSnesFromARAM(RawFile *file) {
  FalcomSnesVersion version = FALCOMSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSeq;
  uint16_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSeq, ofsLoadSeq)) {
    uint8_t addrSeqHeaderPtr = file->GetByte(ofsLoadSeq + 3);
    addrSeqHeader = file->GetShort(addrSeqHeaderPtr);
    version = FALCOMSNES_YS5;
  } else if (file->SearchBytePattern(ptnLoadSeqPM, ofsLoadSeq)) {
    addrSeqHeader = file->GetShort(ofsLoadSeq + 1);
    version = FALCOMSNES_PM;
  }
  else {
    return;
  }

  FalcomSnesSeq *newSeq = new FalcomSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }

  // scan for DIR address
  uint16_t spcDirAddr;
  uint32_t ofsSetDIR;
  if (file->SearchBytePattern(ptnSetDIR, ofsSetDIR)) {
    spcDirAddr = file->GetByte(ofsSetDIR + 1) << 8;
  } else if (file->SearchBytePattern(ptnSetDIRPM, ofsSetDIR)) {
    spcDirAddr = file->GetByte(ofsSetDIR + 1) << 8;
  }
  else {
    return;
  }

  // scan for instrument tables
  uint32_t ofsLoadInstr;
  uint16_t addrSampToInstrTable;
  uint16_t addrInstrTable;
  if (file->SearchBytePattern(ptnLoadInstr, ofsLoadInstr)) {
    addrSampToInstrTable = file->GetShort(ofsLoadInstr + 3);
    addrInstrTable = file->GetShort(ofsLoadInstr + 21);
  } else if (file->SearchBytePattern(ptnLoadInstrPM, ofsLoadInstr)) {
    addrSampToInstrTable = 0;
    addrInstrTable = file->GetByte(ofsLoadInstr + 14) + (file->GetByte(ofsLoadInstr + 17) * 0x100);
  }
  else {
    return;
  }

  FalcomSnesInstrSet *newInstrSet = new FalcomSnesInstrSet(file,
                                                           version,
                                                           addrInstrTable,
                                                           addrSampToInstrTable,
                                                           spcDirAddr,
                                                           newSeq->instrADSRHints);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }
}

void FalcomSnesScanner::SearchForFalcomSnesFromROM(RawFile *file) {
}

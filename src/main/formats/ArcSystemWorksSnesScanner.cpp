#include "pch.h"
#include "ArcSystemWorksSnesFormat.h"
#include "ArcSystemWorksSnesInstr.h"
#include "ArcSystemWorksSnesSeq.h"

//; Bishoujo Senshi Sailor Moon R SPC
//0a66: 9c        dec   a
//0a67 : 1c        asl   a
//0a68 : 5d        mov   x, a
//0a69 : f5 8f 13  mov   a, $138f + x
//0a6c : c4 04     mov   $04, a
//0a6e : 3d        inc   x
//0a6f : f5 8f 13  mov   a, $138f + x; read song table
//0a72: c4 05     mov   $05, a; $04 / 5: song header address
//0a74 : 3f c7 0b  call  $0bc7
//0a77: cd ff     mov   x, #$ff
//0a79: e8 01     mov   a, #$01
//0a7b: c4 34     mov   $34, a
//0a7d : 8f 00 37  mov   $37, #$00
//0a80: 3f 20 0b  call  $0b20; load song header
//0a83: fa 37 35  mov($35), ($37)
//0a86 : 6f        ret
BytePattern ArcSystemWorkSnesScanner::ptnLoadSongNORMAL(
  "\x9c\x1c\x5d\xf5\x8f\x13\xc4\x04"
  "\x3d\xf5\x8f\x13\xc4\x05\x3f\xc7"
  "\x0b\xcd\xff\xe8\x01\xc4\x34\x8f"
  "\x00\x37\x3f\x20\x0b\xfa\x37\x35"
  "\x6f"
  ,
  "xxxx??x?"
  "xx??x?x?"
  "?x?x?x?x"
  "??x??x??"
  "?"
  ,
  33);

//; Shinseiki GPX - Cyber Formula
//; dispatch vcmd
//0d9a: 80        setc
//0d9b : a8 66     sbc   a, #$66
//0d9d: 1c        asl   a
//0d9e : 4d        push  x
//0d9f : 5d        mov   x, a
//0da0 : 1f a3 0d  jmp($0da3 + x)
BytePattern ArcSystemWorkSnesScanner::ptnVcmdSHINGFX(
  "\x80\xa8\x66\x1c\x4d\x5d\x1f\xa3"
  "\x0d"
  ,
  "xxxxxxx?"
  "?"
  ,
  9);

//; Bishoujo Senshi Sailor Moon SPC
//0813: e8 3c     mov   a, #$3c
//0815: 8d 5d     mov   y, #$5d; DIR
BytePattern ArcSystemWorkSnesScanner::ptnReadSampTable(
  "\xe8\x3c\x8d\x5d"
  ,
  "x?x?"
  ,
  4);

//; Bishoujo Senshi Sailor Moon SPC
//0c50: 1c        asl   a
//0c51 : 1c        asl   a
//0c52 : fd        mov   y, a
//0c53 : f6 00 3f  mov   a, $3f00 + y
//0c56 : d5 80 02  mov   $0280 + x, a; SRCN
//0c59: fc        inc   y
BytePattern ArcSystemWorkSnesScanner::ptnLoadInstr(
  "\x1c\x1c\xfd\xf6\x00\x3f\xd5\x80"
  "\x02\xfc"
  ,
  "xxxx??x?"
  "?x"
  ,
  10);



void ArcSystemWorkSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForArcSystemWorkSnesFromARAM(file);
  }
  else {
    SearchForArcSystemWorkSnesFromROM(file);
  }
  return;
}

void ArcSystemWorkSnesScanner::SearchForArcSystemWorkSnesFromARAM(RawFile* file) {
  ArcSystemWorkSnesVersion version = ARCSYSTEMSNES_NONE;

  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSong;
  uint32_t newVcmdAddressGFX;
  uint16_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSongNORMAL, ofsLoadSong)) {
    uint16_t SeqOffset = file->GetShort(ofsLoadSong + 0x04);
    uint8_t songID = file->GetByte(0xf7);
    if (songID == 0x00) {
      songID = 2;
    }
    addrSeqHeader = file->GetShort(SeqOffset + ((songID -= 0x01) * 0x02));
    version = ARCSYSTEMSNES_NORMAL;
    if (file->SearchBytePattern(ptnVcmdSHINGFX, newVcmdAddressGFX)) {
      version = ARCSYSTEMSNES_SHINGFX;
      addrSeqHeader = file->GetShort(0x04);
    }
  }
  else {
    return;
  }

  ArcSystemWorkSnesSeq* newSeq = new ArcSystemWorkSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }

  uint32_t LoadDSP;
  uint32_t ofsDSP;
  uint32_t LoadInstr;
  uint16_t instrTable;
  if (file->SearchBytePattern(ptnReadSampTable, LoadDSP)) {
    uint16_t InitDSP = file->GetByte(LoadDSP + 1);
    ofsDSP = InitDSP * 0x100;
  }
  if (file->SearchBytePattern(ptnLoadInstr, LoadInstr)) {
    instrTable = file->GetShort(LoadInstr + 4);
  }

  ArcSystemWorkSnesInstrSet* newInstrSet = new ArcSystemWorkSnesInstrSet(file, version, ofsDSP, instrTable);
  if (!newInstrSet->LoadVGMFile()) {
    delete newInstrSet;
    return;
  }
}

void ArcSystemWorkSnesScanner::SearchForArcSystemWorkSnesFromROM(RawFile* file) {
}

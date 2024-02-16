#include "pch.h"
#include "OpusSnesScanner.h"
#include "OpusSnesSeq.h"

//; Super Bomberman SPC
//087e: 28 7f and a, #$7f
//0880: c5 00 02  mov   $0200, a
//0883 : 28 3f and a, #$3f
//0885: 68 37     cmp   a, #$37
//0887: 90 19     bcc   $08a2
//0889 : a8 37     sbc   a, #$37
//088b: 1c        asl   a
//088c : 5d        mov   x, a
//088d : 1f 90 08  jmp($0890 + x)
BytePattern OpusSnesScanner::ptnLoadSongIndex(
  "\x28\x7f\xc5\x00\x02\x28\x3f\x68"
  "\x37\x90\x19\xa8\x37\x1c\x5d\x1f"
  "\x90\x08"
  ,
  "x?x??x?x"
  "?x?x?xxx"
  "??"
  ,
  18);

//; Brandish 2 - The Planet Buster SPC
//08ee: 28 7f and a, #$7f
//08f0: c4 24     mov   $24, a
//08f2 : 68 77     cmp   a, #$77
//08f4: 90 11     bcc   $0907
//08f6 : a8 77     sbc   a, #$77
//08f8: 1c        asl   a
//08f9 : 5d        mov   x, a
//08fa : 1f fd 08  jmp($08fd + x)
BytePattern OpusSnesScanner::ptnLoadSongIndexShort(
  "\x28\x7f\xc4\x24\x68\x77\x90\x11"
  "\xa8\x77\x1c\x5d\x1f\xfd\x08"
  ,
  "x?x?x?x?"
  "x?xxx??"
  ,
  15);

//; Dark Half SPC
//088c: 28 7f c5 00 02 28 3f 68 30 f0 0b 68 36 90 23 a8 36 1c 5d 1f aa 08
BytePattern OpusSnesScanner::ptnLoadSongIndexAltVersion(
  "\x28\x7f\xc5\x00\x02\x28\x3f\x68"
  "\x30\xf0\x0b\x68\x36\x90\x23\xa8"
  "\x36\x1c\x5d\x1f\xaa\x08"
  ,
  "x?x??x?x"
  "?x?x?x?x"
  "?xxx??"
  ,
  22);

//; Super Bomberman SPC
//; read song id table
//08c5: fd        mov   y, a
//08c6 : f6 e8 da  mov   a, $dae8 + y; read song id from([$0808] + y).
//08c9: 6f        ret
BytePattern OpusSnesScanner::ptnLoadSongIndexPointer(
  "\xfd\xf6\xe8\xda\x6f"
  ,
  "xx??x"
  ,
  5);


//; Super Bomberman SPC
//0989 : ae        pop   a
//098a : 1c        asl   a
//098b : 5d        mov   x, a
//098c : f5 52 d3  mov   a, $d352 + x
//098f : c4 02     mov   $02, a
//0991 : f5 53 d3  mov   a, $d353 + x; read address from song table([$0806] + 2 + x)
//0994: c4 03     mov   $03, a; $02 / 3 = song address
//0996: 8d 00     mov   y, #$00
//0998: cd 00     mov   x, #$00
//099a: 8d 00     mov   y, #$00
//099c: f7 02     mov   a, ($02)+y; offset + 0: track size(lo)
//099e : c4 04     mov   $04, a
//09a0 : 3a 02     incw  $02
//09a2 : f7 02     mov   a, ($02)+y; offset + 1: track size(hi)
//09a4 : 68 ff     cmp   a, #$ff
//09a6: f0 3c     beq   $09e4; branch if $ffxx, end of header
BytePattern OpusSnesScanner::ptnLoadSongPointer(
  "\xae\x1c\x5d\xf5\x52\xd3\xc4\x02"
  "\xf5\x53\xd3\xc4\x03\x8d\x00\xcd"
  "\x00\x8d\x00\xf7\x02\xc4\x04\x3a"
  "\x02\xf7\x02\x68\xff\xf0\x3c"
  ,
  "xxxx??x?"
  "x??x?x?x"
  "?x?x?x?x"
  "?x?x?x?"
  ,
  31);

void OpusSnesScanner::Scan(RawFile* file, void* info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForOpusSnesFromARAM(file);
  }
  else {
    SearchForOpusSnesFromROM(file);
  }
  return;
}

void OpusSnesScanner::SearchForOpusSnesFromARAM(RawFile* file) {
  OpusSnesVersion version = OPUSSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  // search song list
  uint32_t ofsLoadSeq;
  uint32_t ofsLoadSongIndex;
  uint32_t ofsLoadSongIndexPointer;
  uint16_t addrSeqList;
  uint16_t songIndexMax;
  uint8_t headerAlignSize;
  uint8_t songIndex;
  if (file->SearchBytePattern(ptnLoadSongPointer, ofsLoadSeq)) {
    if (file->SearchBytePattern(ptnLoadSongIndex, ofsLoadSongIndex)) {
      uint8_t andByte = file->GetByte(ofsLoadSongIndex + 6);
      songIndexMax = file->GetByte(file->GetShort(ofsLoadSongIndex + 3)) & andByte;
      version = OPUSSNES_V108;
    }
    else if (file->SearchBytePattern(ptnLoadSongIndexShort, ofsLoadSongIndex)) {
      songIndexMax = file->GetByte(file->GetByte(ofsLoadSongIndex + 3));
      version = OPUSSNES_KOEI;
    }
    else if (file->SearchBytePattern(ptnLoadSongIndexAltVersion, ofsLoadSongIndex)) {
      uint8_t andByte = file->GetByte(ofsLoadSongIndex + 6);
      songIndexMax = file->GetByte(file->GetShort(ofsLoadSongIndex + 3)) & andByte;
      version = OPUSSNES_V111;
    }
    file->SearchBytePattern(ptnLoadSongIndexPointer, ofsLoadSongIndexPointer);


    uint16_t songIndexTable = file->GetShort(ofsLoadSongIndexPointer + 2);

    songIndex = file->GetByte(songIndexTable + songIndexMax);

    addrSeqList = file->GetShort(ofsLoadSeq + 4);

    // TODO: detact version
  }
  else {
    return;
  }

  // TODO: guess song index
  uint32_t addrSeqHeader = file->GetShort(addrSeqList + (songIndex * 2));
  OpusSnesSeq* newSeq = new OpusSnesSeq(file, version, addrSeqHeader, addrSeqList, songIndex, name);

  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void OpusSnesScanner::SearchForOpusSnesFromROM(RawFile* file) {
}

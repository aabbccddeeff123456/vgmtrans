#include "pch.h"
#include "OceanSnesFormat.h"
#include "OceanSnesSeq.h"

// Jurassic Park
  //mov !$0439,a
  //mov !$036c,a
  //asl a
  //asl a
  //asl a
  //beq $0cb8
  //mov x,a
  //mov a,!$7ff9+x
  //mov y,a
  //mov a,!$7ff8+x
  //movw $1d,ya
  //mov a,!$7ffa+x
  //mov !$03fa,a
  //mov a,!$7ffd+x
  //mov y,a
  //mov a,!$7ffc+x
  //movw $19,ya
BytePattern OceanSnesScanner::ptnLoadSongNSPC(
  "\xc5\x39\x04\xc5\x6c\x03\x1c\x1c"
  "\x1c\xf0\xf4\x5d\xf5\xf9\x7f\xfd"
  "\xf5\xf8\x7f\xda\x1d\xf5\xfa\x7f"
  "\xc5\xfa\x03\xf5\xfd\x7f\xfd\xf5"
  "\xfc\x7f\xda\x19"
	,
	"x??x??xx"
  "xx?xx??x"
  "x??x?x??"
  "x??x??xx"
  "??x?"
	,
	36);

//8D 0F CD 0F F7 23 F0 04 D4 26 D4 70 DC 1D 10 F4
BytePattern OceanSnesScanner::ptnLoadSongNSPC2(
	"\x8d\x0f\xcd\x0f\xf7\x23\xf0\x04"
  "\xd4\x26\xd4\x70\xdc\x1d\x10\xf4"
	,
	"x?x?x?x?"
  "x?x?xxx?"
	,
	16);

// Jurassic Park 2
  //mov $30,$e8
  //mov $31,$e9
  //mov y,#$00
  //mov a,[$30]+y
  //mov $34,a
  //inc y
  //mov a,[$30]+y
  //mov $35,a
  //inc y
// ...
BytePattern OceanSnesScanner::ptnLoadSongOCEAN(
	"\xfa\xe8\x30\xfa\xe9\x31\x8d\x00"
  "\xf7\x30\xc4\x34\xfc\xf7\x30\xc4"
  "\x35\xfc"
	,
	"x??x??x?"
  "x?x?xx?x"
  "?x"
	,
	18);

void OceanSnesScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  if (nFileLength == 0x10000) {
    SearchForOceanSnesFromARAM(file);
  }
  else {
    SearchForOceanSnesFromROM(file);
  }
  return;
}

void OceanSnesScanner::SearchForOceanSnesFromARAM(RawFile *file) {
  OceanSnesVersion version = OCEANSNES_NONE;
  std::wstring name = file->tag.HasTitle() ? file->tag.title : RawFile::removeExtFromPath(file->GetFileName());

  uint32_t ofsLoadSong;
  uint16_t addrSeqHeader;
  if (file->SearchBytePattern(ptnLoadSongNSPC, ofsLoadSong)) {
    uint8_t songIndex = file->GetByte(file->GetShort(ofsLoadSong + 1)) * 8;
    addrSeqHeader = file->GetShort(file->GetShort(ofsLoadSong + 17) + songIndex);
    version = OCEANSNES_NSPC;
  }
  else if (file->SearchBytePattern(ptnLoadSongOCEAN, ofsLoadSong)) {
    uint16_t songlistPointer = file->GetShort(file->GetByte(ofsLoadSong + 1));
    addrSeqHeader = file->GetShort(songlistPointer);
    version = OCEANSNES_STANDALONE;
  } else if (file->SearchBytePattern(ptnLoadSongNSPC2, ofsLoadSong)) {
    uint8_t songIndex = file->GetByte(ofsLoadSong + 5);
    addrSeqHeader = file->GetShort(songIndex);  // may need to check?
    version = OCEANSNES_NSPC2;
  }
  else {
    return;
  }

  OceanSnesSeq *newSeq = new OceanSnesSeq(file, version, addrSeqHeader, name);
  if (!newSeq->LoadVGMFile()) {
    delete newSeq;
    return;
  }
}

void OceanSnesScanner::SearchForOceanSnesFromROM(RawFile *file) {
}

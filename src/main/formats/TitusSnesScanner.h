#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class TitusSnesScanner :
  public VGMScanner {
public:
  TitusSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~TitusSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForTitusSnesFromARAM(RawFile* file);
  void SearchForTitusSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSongEarly;
  static BytePattern ptnLoadSongVer3;
  static BytePattern ptnLoadSong;
};

#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class WinkySoftSnesScanner :
  public VGMScanner {
public:
  WinkySoftSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~WinkySoftSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForWinkySoftSnesFromARAM(RawFile* file);
  void SearchForWinkySoftSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSongOLD;
  static BytePattern ptnLoadSongSRT3;
  static BytePattern ptnLoadSongSRT4;
  static BytePattern ptnLoadInstrSet;
  static BytePattern ptnSetDIR;
};

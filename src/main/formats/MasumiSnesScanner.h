#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class MasumiSnesScanner :
  public VGMScanner {
public:
  MasumiSnesScanner(void) {
    USE_EXTENSION(L"spc");

    // Stream SNES Version
    //USE_EXTENSION(L"smc");
    //USE_EXTENSION(L"sfc");
  }
  virtual ~MasumiSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForMasumiSnesFromARAM(RawFile* file);
  void SearchForMasumiSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
  static BytePattern ptnLoadSeqEarly;
};

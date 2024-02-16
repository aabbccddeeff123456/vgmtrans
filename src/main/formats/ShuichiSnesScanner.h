#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class ShuichiSnesScanner :
  public VGMScanner {
public:
  ShuichiSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~ShuichiSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForShuichiSnesFromARAM(RawFile* file);
  void SearchForShuichiSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSongVER1;
  static BytePattern ptnLoadSongVER2;
};

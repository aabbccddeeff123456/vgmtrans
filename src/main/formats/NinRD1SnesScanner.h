#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class NinRD1SnesScanner :
  public VGMScanner {
public:
  NinRD1SnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~NinRD1SnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForNinRD1SnesFromARAM(RawFile* file);
  void SearchForNinRD1SnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
};

#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class SensibleSnesScanner :
  public VGMScanner {
public:
  SensibleSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~SensibleSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForSensibleSnesFromARAM(RawFile* file);
  void SearchForSensibleSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
  static BytePattern ptnLoadSeq2;
};

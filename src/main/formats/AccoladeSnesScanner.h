#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class AccoladeSnesScanner :
  public VGMScanner {
public:
  AccoladeSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~AccoladeSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForAccoladeSnesFromARAM(RawFile* file);
  void SearchForAccoladeSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeqListPointer;
};

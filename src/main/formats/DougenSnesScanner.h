#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class DougenSnesScanner :
  public VGMScanner {
public:
  DougenSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~DougenSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForDougenSnesFromARAM(RawFile* file);
  void SearchForDougenSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
};

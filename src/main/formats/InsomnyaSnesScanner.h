#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class InsomnyaSnesScanner:
    public VGMScanner {
 public:
  InsomnyaSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~InsomnyaSnesScanner(void) {
  }

  virtual void Scan(RawFile *file, void *info = 0);
  void SearchForInsomnyaSnesFromARAM(RawFile *file);
  void SearchForInsomnyaSnesFromROM(RawFile *file);

 private:
  static BytePattern ptnLoadSeq;
};

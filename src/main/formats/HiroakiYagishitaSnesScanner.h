#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class HiroakiYagishitaSnesScanner :
  public VGMScanner {
public:
  HiroakiYagishitaSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~HiroakiYagishitaSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForHiroakiYagishitaSnesFromARAM(RawFile* file);
  void SearchForHiroakiYagishitaSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
  static BytePattern ptnLoadSeqAlt;
};

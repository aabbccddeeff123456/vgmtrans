#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class GremlinSnesScanner :
  public VGMScanner {
public:
  GremlinSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~GremlinSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForGremlinSnesFromARAM(RawFile* file);
  void SearchForGremlinSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSection;
  static BytePattern ptnLoadSeq;
};

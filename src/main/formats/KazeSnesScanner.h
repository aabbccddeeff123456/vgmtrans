#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class KazeSnesScanner :
  public VGMScanner {
public:
  KazeSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~KazeSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForKazeSnesFromARAM(RawFile* file);
  void SearchForKazeSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
};

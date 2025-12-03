#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class DSEScanner : public VGMScanner {
public:
  DSEScanner(void) {
    USE_EXTENSION(L"smd");
    USE_EXTENSION(L"bgm");
    USE_EXTENSION(L"nds");
  }
  virtual ~DSEScanner(void) {}

  virtual void Scan(RawFile *file, void *info = 0);
  void SearchForDSEFromFILE(RawFile *file);

private:
  static BytePattern ptnCheckHeader;
};


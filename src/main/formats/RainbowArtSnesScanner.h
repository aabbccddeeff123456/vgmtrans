#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class RainbowArtSnesScanner :
  public VGMScanner {
public:
  RainbowArtSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~RainbowArtSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForRainbowArtSnesFromARAM(RawFile* file);
  void SearchForRainbowArtSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeq;
};

#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class OceanSnesScanner:
    public VGMScanner {
 public:
  OceanSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~OceanSnesScanner(void) {
  }

  virtual void Scan(RawFile *file, void *info = 0);
  void SearchForOceanSnesFromARAM(RawFile *file);
  void SearchForOceanSnesFromROM(RawFile *file);

 private:
  static BytePattern ptnLoadSongNSPC;
  static BytePattern ptnLoadSongOCEAN;
  static BytePattern ptnLoadSongNSPC2;
};

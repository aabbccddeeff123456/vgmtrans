#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class OpusSnesScanner :
  public VGMScanner {
public:
  OpusSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~OpusSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForOpusSnesFromARAM(RawFile* file);
  void SearchForOpusSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSongIndex;
  static BytePattern ptnLoadSongPointer;
  static BytePattern ptnLoadSongIndexPointer;
  static BytePattern ptnLoadSongIndexShort;
  static BytePattern ptnLoadSongIndexAltVersion;
};

#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class GameFreakSnesScanner :
  public VGMScanner {
public:
  GameFreakSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~GameFreakSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForGameFreakSnesFromARAM(RawFile* file);
  void SearchForGameFreakSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSong;
  static BytePattern ptnLoadInstrSet;
};

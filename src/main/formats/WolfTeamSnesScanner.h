#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class WolfTeamSnesScanner:
  public VGMScanner {
public:
  WolfTeamSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~WolfTeamSnesScanner(void) {
  }

  virtual void Scan(RawFile *file, void *info = 0);
  void SearchForWolfTeamSnesFromARAM(RawFile *file);
  void SearchForWolfTeamSnesFromROM(RawFile *file);

private:
  static BytePattern ptnLoadSongTOP;
  static BytePattern ptnLoadSongOLD;
  static BytePattern ptnLoadSongZAN2;
  static BytePattern ptnLoadSongMID;
  //static BytePattern ptnSetDIR;
  //static BytePattern ptnLoadSongSO;
};

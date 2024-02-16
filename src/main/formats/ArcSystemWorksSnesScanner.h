#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class ArcSystemWorkSnesScanner :
  public VGMScanner {
public:
  ArcSystemWorkSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~ArcSystemWorkSnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForArcSystemWorkSnesFromARAM(RawFile* file);
  void SearchForArcSystemWorkSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSongNORMAL;
  static BytePattern ptnVcmdSHINGFX;
  static BytePattern ptnReadSampTable;
  static BytePattern ptnLoadInstr;
};

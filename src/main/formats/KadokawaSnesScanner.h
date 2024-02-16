#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class KadokawaSnesScanner:
  public VGMScanner {
public:
  KadokawaSnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~KadokawaSnesScanner(void) {
  }

  virtual void Scan(RawFile *file, void *info = 0);
  void SearchForKadokawaSnesFromARAM(RawFile *file);
  void SearchForKadokawaSnesFromROM(RawFile *file);

private:
  static BytePattern ptnLoadSongOK;
  static BytePattern ptnSongLocationOK;
  static BytePattern ptnInstrLocationOK;
  static BytePattern ptnDIRLocationOK;
  static BytePattern ptnLoadSongStingSquare;
};

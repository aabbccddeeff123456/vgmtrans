#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class DaviddeSnesScanner : public VGMScanner {
public:
  DaviddeSnesScanner(void) { USE_EXTENSION(L"spc"); }
  virtual ~DaviddeSnesScanner(void) {}

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForDaviddeSnesFromARAM(RawFile* file);
  void SearchForDaviddeSnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSection;
  static BytePattern ptnLoadSeq;
};

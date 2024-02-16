#pragma once
#include "Scanner.h"
#include "BytePattern.h"

class RealtimeASnesScanner :
  public VGMScanner {
public:
  RealtimeASnesScanner(void) {
    USE_EXTENSION(L"spc");
  }
  virtual ~RealtimeASnesScanner(void) {
  }

  virtual void Scan(RawFile* file, void* info = 0);
  void SearchForRealtimeASnesFromARAM(RawFile* file);
  void SearchForRealtimeASnesFromROM(RawFile* file);

private:
  static BytePattern ptnLoadSeqACIDV;
  static BytePattern ptnLoadSeqC1992;
  static BytePattern ptnLoadSeq1992;
  static BytePattern ptnLoadSeq1993;
  static BytePattern ptnLoadSeqFromTableACIDV;
  static BytePattern ptnLoadSeqFromTableC1992;
  static BytePattern ptnLoadSeqFromTable;
  static BytePattern ptnWordtrisCheck1992;
};

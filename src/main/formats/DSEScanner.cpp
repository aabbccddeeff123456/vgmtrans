#include "pch.h"
#include "DSEFormat.h"
#include "DSESeq.h"

void DSEScanner::Scan(RawFile *file, void *info) {
  uint32_t nFileLength = file->size();
  SearchForDSEFromFILE(file);
  return;
}

void DSEScanner::SearchForDSEFromFILE(RawFile *file) {
  DSEVersion version = DSE_NONE;

  std::wstring basefilename = RawFile::removeExtFromPath(file->GetFileName());
  std::wstring name = file->tag.HasTitle() ? file->tag.title : basefilename;

  uint32_t ofsLoadSong;
  uint32_t addrSeqHeader;
  uint32_t nFileLength = file->size();
  for (uint32_t i = 0; i + 4 < nFileLength; i++) {
    if ((*file)[i] == 's' && (*file)[i + 1] == 'm' && (*file)[i + 2] == 'd' &&
        (*file)[i + 3] == 'l' && (*file)[i + 4] == 0x00 && (*file)[i + 5] == 0x00 &&
        (*file)[i + 6] == 0 && (*file)[i + 7] == 0x00 &&
        (file->GetWord(i + 0x8) < 0x100000)) {
      version = DSE_MAIN;
      addrSeqHeader = i;
      DSESeq *newSeq = new DSESeq(file, version, addrSeqHeader);
      if (!newSeq->LoadVGMFile()) {
        pRoot->AddLogItem(new LogItem(
            FormatString<wstring>(L"Failed to load DSESeq at 0x%08x\n", i).c_str(),
            LOG_LEVEL_ERR, L"DSEScanner"));
      }
    }
  }
}

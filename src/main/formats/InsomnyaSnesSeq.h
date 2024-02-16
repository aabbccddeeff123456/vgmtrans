#pragma once
#include "VGMSeq.h"
#include "SeqTrack.h"
#include "SeqEvent.h"
#include "InsomnyaSnesFormat.h"

enum InsomnyaSnesSeqEventType {
  //start enum at 1 because if map[] look up fails, it returns 0, and we don't want that to get confused with a legit event
  EVENT_UNKNOWN0 = 1,
  EVENT_UNKNOWN1,
  EVENT_UNKNOWN2,
  EVENT_UNKNOWN3,
  EVENT_UNKNOWN4,
  EVENT_NOTE,
  EVENT_DELTA_TIME,
  EVENT_PROGCHANGE,
  EVENT_VOLUME,
  EVENT_TICK,
  EVENT_PLAYLIST_END,
};

class InsomnyaSnesSeq
    : public VGMSeq {
 public:
  InsomnyaSnesSeq(RawFile *file,
                   InsomnyaSnesVersion ver,
                   uint32_t seqdata_offset,
                   std::wstring newName = L"Insomnya SNES Seq");
  virtual ~InsomnyaSnesSeq(void);

  virtual bool GetHeaderInfo(void);
  virtual bool GetTrackPointers(void);
  virtual void ResetVars(void);

  InsomnyaSnesVersion version;
  std::map<uint8_t, InsomnyaSnesSeqEventType> EventMap;
  uint16_t channelPlaylistPointer[8];
  uint8_t currentTick;

 private:
  void LoadEventMap(void);

  uint8_t headerAlignSize;
};


class InsomnyaSnesTrack
    : public SeqTrack {
 public:
  InsomnyaSnesTrack(InsomnyaSnesSeq *parentFile, long offset = 0, long length = 0);
  virtual void ResetVars(void);
  virtual bool ReadEvent(void);

  uint8_t currentPlaylist[8];
  uint8_t currentTick;
};

#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "RealtimeASnesScanner.h"


// ***************
// RealtimeASnesFormat
// ***************

BEGIN_FORMAT(RealtimeASnes)
USING_SCANNER(RealtimeASnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum RealtimeASnesVersion {
  REALTIMEASNES_NONE = 0,  // Unknown Version
  REALTIMEASNES_ACIDV,
  REALTIMEASNES_C1992,
  REALTIMEASNES_C1992_WORDTRIS,
  REALTIMEASNES_1992,
  REALTIMEASNES_1993,
};

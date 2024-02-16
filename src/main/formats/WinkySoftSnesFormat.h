#pragma once
#include "Format.h"
#include "Root.h"
#include "WinkySoftSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// WinkySoftSnesFormat
// *************

BEGIN_FORMAT(WinkySoftSnes)
USING_SCANNER(WinkySoftSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum WinkySoftSnesVersion {
  WINKYSOFTSNES_NONE = 0,           // Not Supported
  WINKYSOFTSNES_OLD,                // Early version
  WINKYSOFTSNES_SRT3,               // Dai 3 Ji Super Robot Taisen
  WINKYSOFTSNES_SRT4,               // Dai 4 Ji Super Robot Taisen,etc.
};

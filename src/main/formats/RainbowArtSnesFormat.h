#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "RainbowArtSnesScanner.h"


// ***************
// RainbowArtSnesFormat
// ***************

BEGIN_FORMAT(RainbowArtSnes)
USING_SCANNER(RainbowArtSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum RainbowArtSnesVersion {
  RAINBOWARTSNES_NONE = 0,  // Unknown Version
  RAINBOWARTSNES_MAIN,      // Rendering Ranger R2
};

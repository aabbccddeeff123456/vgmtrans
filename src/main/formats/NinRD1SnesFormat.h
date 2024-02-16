#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "NinRD1SnesScanner.h"


// ***************
// NinRD1SnesFormat
// ***************

BEGIN_FORMAT(NinRD1Snes)
USING_SCANNER(NinRD1SnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum NinRD1SnesVersion {
  NINRD1SNES_NONE = 0,  // Unknown Version
  NINRD1SNES_MAIN,      // Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo
};

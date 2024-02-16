#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "AccoladeSnesScanner.h"


// ***************
// AccoladeSnesFormat
// ***************

BEGIN_FORMAT(AccoladeSnes)
USING_SCANNER(AccoladeSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum AccoladeSnesVersion {
  ACCOLADESNES_NONE = 0,  // Unknown Version
  ACCOLADESNES_EARLY,   // WarpSpeed
  ACCOLADESNES_MAIN,    // All Games later
};

#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "SensibleSnesScanner.h"


// ***************
// SensibleSnesFormat
// ***************

BEGIN_FORMAT(SensibleSnes)
USING_SCANNER(SensibleSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum SensibleSnesVersion {
  SENSIBLESNES_NONE = 0,  // Unknown Version
  SENSIBLESNES_MAIN,
};

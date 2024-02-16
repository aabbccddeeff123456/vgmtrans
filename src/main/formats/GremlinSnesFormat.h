#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "GremlinSnesScanner.h"


// ***************
// GremlinSnesFormat
// ***************

BEGIN_FORMAT(GremlinSnes)
USING_SCANNER(GremlinSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum GremlinSnesVersion {
  GREMLINSNES_NONE = 0,  // Unknown Version
  GREMLINSNES_MAIN,       
};


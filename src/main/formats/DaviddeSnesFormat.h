#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "DaviddeSnesScanner.h"

// ***************
// DaviddeSnesFormat
// ***************

BEGIN_FORMAT(DaviddeSnes)
USING_SCANNER(DaviddeSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum DaviddeSnesVersion {
  DAVIDDESNES_NONE = 0,  // Unknown Version
  DAVIDDESNES_MAIN,
};

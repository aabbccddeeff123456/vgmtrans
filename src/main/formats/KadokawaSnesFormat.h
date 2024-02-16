#pragma once
#include "Format.h"
#include "Root.h"
#include "KadokawaSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// NeverlandSnesFormat
// *************

BEGIN_FORMAT(KadokawaSnes)
USING_SCANNER(KadokawaSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum KadokawaSnesVersion {
  KADOKAWASNES_NONE = 0,           // Not Supported
  KADOKAWASNES_OK,                  // Youkai Buster - Ruka no Daibouken
  STINGSNES_SQUARE,                  // Treasure Hunter G
};

#pragma once
#include "Format.h"
#include "Root.h"
#include "TitusSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// TitusSnesFormat
// *************

BEGIN_FORMAT(TitusSnes)
USING_SCANNER(TitusSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum TitusSnesVersion {
  TITUSSNES_NONE = 0,           // Not Supported
// Main Branch
  TITUSSNES_V1,
  TITUSSNES_V2,
  TITUSSNES_V3,
// Company Branch
  TITUSSNES_V2_ZEPPELIN,
  TITUSSNES_V3_FLAIR,
};

#pragma once
#include "Format.h"
#include "Root.h"
#include "DSEScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// DSEFormat
// *************

BEGIN_FORMAT(DSE)
USING_SCANNER(DSEScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum DSEVersion {
  DSE_NONE = 0,      // Not Supported
  DSE_MAIN,          // V1
};

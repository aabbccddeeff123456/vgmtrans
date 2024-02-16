#pragma once
#include "Format.h"
#include "Root.h"
#include "ShuichiSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// ShuichiSnesFormat
// *************

BEGIN_FORMAT(ShuichiSnes)
USING_SCANNER(ShuichiSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum ShuichiSnesVersion {
  SHUICHISNES_NONE = 0,           // Not Supported
  SHUICHISNES_VER1,               // Down the World: Mervil's Ambition,etc.
  SHUICHISNES_VER2,               // Unknown Engine.
};

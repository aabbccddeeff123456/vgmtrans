#pragma once
#include "Format.h"
#include "Root.h"
#include "OpusSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// OpusSnesFormat
// *************

BEGIN_FORMAT(OpusSnes)
USING_SCANNER(OpusSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum OpusSnesVersion {
  OPUSSNES_NONE = 0,           // Not Supported
// Main Branch
  OPUSSNES_EARLY,              // DBOOT VER1.07
  OPUSSNES_V108,               // DBOOT VER1.08
  OPUSSNES_SQV1228,            // DBOOT VER1.228?
  OPUSSNES_V110,               // DBOOT VER1.10
  OPUSSNES_V111,               // DBOOT VER1.11
// Company Branch
  OPUSSNES_DBSQ,               // DBOOT DB/SQ
  OPUSSNES_KOEI,               // DBOOT Koei
  OPUSSNES_SHVC,               // DBOOT SHVC
};

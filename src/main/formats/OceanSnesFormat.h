#pragma once
#include "Format.h"
#include "Root.h"
#include "OceanSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// OceanSnesFormat
// *************

BEGIN_FORMAT(OceanSnes)
  USING_SCANNER(OceanSnesScanner)
  USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum OceanSnesVersion {
  OCEANSNES_NONE = 0,           // Not Supported
  OCEANSNES_NSPC,               // Jurassic Park
  OCEANSNES_NSPC2,              // Other N-SPC Related Ocean games
  OCEANSNES_STANDALONE,         // Jurassic Park 2 etc.
};

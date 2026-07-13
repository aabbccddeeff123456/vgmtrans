#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "HiroakiYagishitaSnesScanner.h"


// ***************
// HiroakiYagishitaSnesFormat
// ***************

BEGIN_FORMAT(HiroakiYagishitaSnes)
USING_SCANNER(HiroakiYagishitaSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum HiroakiYagishitaSnesVersion {
  HIROAKIYAGISHITASNES_NONE = 0,  // Unknown Version
  HIROAKIYAGISHITASNES_MAIN,      // has song list
  HIROAKIYAGISHITASNES_VER2,     // no song list

  HIROAKIYAGISHITASNES_MAIN_SLAYERS,      // Slayers
  HIROAKIYAGISHITASNES_MAIN_KIDOU,        // Kidou Keisastu Patlabor
};

#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "MasumiSnesScanner.h"


// ***************
// MasumiSnesFormat
// ***************

BEGIN_FORMAT(MasumiSnes)
USING_SCANNER(MasumiSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum MasumiSnesVersion {
  MASUMISNES_NONE = 0,  // Unknown Version
  MASUMISNES_V1,        // Feda - The Emblem of Justice(This version should search directly in ROM)
  MASUMISNES_MAIN,      // Majuu Ou,Kishin Douji Zenki - Battle Raiden,etc.
};

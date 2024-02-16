#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "GameFreakSnesScanner.h"


// ***************
// GameFreakSnesFormat
// ***************

BEGIN_FORMAT(GameFreakSnes)
USING_SCANNER(GameFreakSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum GameFreakSnesVersion {
  GAMEFREAKSNES_NONE = 0,  // Unknown Version
  GAMEFREAKSNES_MAIN,     // Bushi Seiryuuden - Futari no Yuusha
};

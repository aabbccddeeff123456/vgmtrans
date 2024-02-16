#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "KazeSnesScanner.h"


// ***************
// KazeSnesFormat
// ***************

BEGIN_FORMAT(KazeSnes)
USING_SCANNER(KazeSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum KazeSnesVersion {
  KAZESNES_NONE = 0,  // Unknown Version
  KAZESNES_MAIN,      
};

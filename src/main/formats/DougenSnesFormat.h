#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "DougenSnesScanner.h"


// ***************
// DougenSnesFormat
// ***************

BEGIN_FORMAT(DougenSnes)
USING_SCANNER(DougenSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum DougenSnesVersion {
  DOUGENSNES_NONE = 0,  // Unknown Version
  DOUGENSNES_MAIN,      // Crayon Shin-chan: Nagagutsu Dobon!!, Der Langrisser
};

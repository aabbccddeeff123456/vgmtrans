#pragma once
#include "Format.h"
#include "Matcher.h"
#include "Root.h"
#include "InsomnyaSnesScanner.h"


// ***************
// InsomnyaSnesFormat
// ***************

BEGIN_FORMAT(InsomnyaSnes)
  USING_SCANNER(InsomnyaSnesScanner)
  USING_MATCHER(FilegroupMatcher)
END_FORMAT()


enum InsomnyaSnesVersion {
  INSOMNYASNES_NONE = 0,  // Unknown Version
  INSOMNYASNES_MAIN,  // All Varient
};

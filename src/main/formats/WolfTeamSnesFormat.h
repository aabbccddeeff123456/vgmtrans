#pragma once
#include "Format.h"
#include "Root.h"
#include "WolfTeamSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// WolfTeamSnesFormat
// *************

BEGIN_FORMAT(WolfTeamSnes)
USING_SCANNER(WolfTeamSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum WolfTeamSnesVersion {
  WOLFTEAMSNES_NONE = 0,           // Not Supported
  WOLFTEAMSNES_ZAN2,               // Zan 2 Spirits
  WOLFTEAMSNES_EARLIER,            // Arcus Spirits,Neugier,etc
  WOLFTEAMSNES_MID,                // Dark Kingdom,Zan 3 Spirits,etc
  WOLFTEAMSNES_TOP,                // Tales Of Phantasia
  WOLFTEAMSNES_SO,                 // Star Ocean
  WOLFTEAMSNES_TENSHI,             // Tenshi no Uta - Shiroki Tsubasa no Inori
  WOLFTEAMSNES_LATER,               // Later Unknown Minor Version
};

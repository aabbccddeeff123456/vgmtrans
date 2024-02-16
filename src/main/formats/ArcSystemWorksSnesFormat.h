#pragma once
#include "Format.h"
#include "Root.h"
#include "ArcSystemWorksSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// NeverlandSnesFormat
// *************

BEGIN_FORMAT(ArcSystemWorkSnes)
USING_SCANNER(ArcSystemWorkSnesScanner)
USING_MATCHER(FilegroupMatcher)
END_FORMAT()

enum ArcSystemWorkSnesVersion {
  ARCSYSTEMSNES_NONE = 0,           // Not Supported
  ARCSYSTEMSNES_NORMAL,             // Normal System,common

  ARCSYSTEMSNES_SHINGFX,             // Shinseiki GPX - Cyber Formula
};

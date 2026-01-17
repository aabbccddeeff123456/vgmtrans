#pragma once
#include "Format.h"
#include "Root.h"
#include "NinSnesScanner.h"
#include "Matcher.h"
#include "VGMColl.h"

// *************
// NinSnesFormat
// *************

enum NinSnesVersion {
  NINSNES_NONE = 0,           // Not Supported
  NINSNES_UNKNOWN,            // Unknown version (only header support)
  NINSNES_EARLIER,            // Eariler version (Super Mario World, Pilotwings)
  NINSNES_STANDARD,           // Common version with voice commands $e0-fa (e.g. Yoshi Island)
  NINSNES_STANDARD_WITH_FE3_COMMAND,  // Common version with voice commands $d6-fd

                              // derived formats
  NINSNES_RD1,                // Nintendo RD1 (e.g. Super Metroid, Earthbound)
  NINSNES_RD2,                // Nintendo RD2 (e.g. Marvelous)
  NINSNES_HAL,                // HAL Laboratory games (e.g. Kirby series)
  NINSNES_KONAMI,             // Old Konami games (e.g. Gradius III, Castlevania IV, Legend of the Mystical Ninja)
  NINSNES_LEMMINGS,           // Lemmings
  NINSNES_INTELLI_FE3,        // Fire Emblem 3
  NINSNES_INTELLI_TA,         // Tetris Attack
  NINSNES_INTELLI_FE4,        // Fire Emblem 4
  NINSNES_INTELLI_MAIN,       // Other games used Intelligent Systems SPC Engine
  NINSNES_HUMAN,              // Human games (e.g. Clock Tower, Firemen, Septentrion)
  NINSNES_TOSE,               // TOSE games (e.g. Yoshi's Safari, Dragon Ball Z: Super Butouden 2)
  NINSNES_QUINTET_ACTR,       // ActRaiser, Soul Blazer
  NINSNES_QUINTET_ACTR2,      // ActRaiser 2
  NINSNES_QUINTET_IOG,        // Illusion of Gaia, Robotrek
  NINSNES_QUINTET_TS,         // Terranigma (Tenchi Souzou)
  NINSNES_FALCOM_YS4,         // Ys IV // In fact,the engine was made by Cube.
  NINSNES_MAKE_SOFTWARE,      // Make Software Games
  NINSNES_PRODUCE,            // The 7th Saga
// OCEAN need a new format
// NINSNES_OCEAN,              // Ocean Games
  NINSNES_STING,              // Sting Games (Different Header)

  NINSNES_ANTHROX,            // Vortex
  NINSNES_ACC,                // Advance Communication Company
};

BEGIN_FORMAT(NinSnes)
  USING_SCANNER(NinSnesScanner)
  USING_MATCHER(FilegroupMatcher)

  static inline bool IsQuintetVersion(NinSnesVersion version) {
    return version == NINSNES_QUINTET_ACTR ||
        version == NINSNES_QUINTET_ACTR2 ||
        version == NINSNES_QUINTET_IOG ||
        version == NINSNES_QUINTET_TS;
  }
END_FORMAT()

#include "stdafx.h"

#include "Achievements.hpp"

#ifdef R_STEAM_RUNTIME
#include "2-achievements/AchievementsMgr.h"
#include "shared/steam/2-achievements/SteamAchievementsMgr.h"
#endif // R_STEAM_RUNTIME

#ifdef R_GOG_RUNTIME
#include "2-achievements/AchievementsMgr.h"
#include "shared/steam/2-achievements/GogAchievementsMgr.h"
#endif // R_GOG_RUNTIME
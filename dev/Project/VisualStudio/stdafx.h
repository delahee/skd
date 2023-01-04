// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN

#include "platform.h"

#include <cstdio>
#include <cstring>
#ifdef PASTA_WIN
#include <tchar.h>
#endif

#include <vector>
#include <functional>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <iosfwd>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "r/all.hpp"
#include "rs/all.hpp"
#include "rd/all.hpp"
#include "r2/all.hpp"
#include "fmt/all.hpp"
#include "ri18n/all.hpp"

using namespace ri18n;

#include "UserDefines.hpp"




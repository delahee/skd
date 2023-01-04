#pragma once

#ifdef PASTA_WIN
#define WIN32_LEAN_AND_MEAN
#include "platform.h"
#endif

#include <stdio.h>
#include <cstring>

#ifdef PASTA_WIN
#include <tchar.h>
#endif

#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <map>
#include <any>

#include "EASTL/vector.h"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-input/InputEnums.h"
#include "1-input/InputMgr.h"

#include "4-menus/imgui.h"
#include "rapidjson/document.h"

#include "r/all.hpp"
#include "rs/all.hpp"
#include "rd/all.hpp"
#include "r2/all.hpp"
#include "r3/all.hpp"
#include "rui/all.hpp"
#include "fmt/all.hpp"

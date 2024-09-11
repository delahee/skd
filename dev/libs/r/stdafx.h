#pragma once

#ifdef PASTA_WIN
#define WIN32_LEAN_AND_MEAN
#include "platform.h"
#endif

#include <cstdio>
#include <cstring>

#ifdef PASTA_WIN
#include <tchar.h>
#endif

#include <algorithm>
#include <any>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iosfwd>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


#include "EASTL/vector.h"
#include "EASTL/functional.h"

#include <sol/sol.hpp>

#include "1-time/Profiler.h"
#include "1-files/FileMgr.h"
#include "1-files/File.h"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/GraphicContext.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"
#include "1-graphics/Texture.h"
#include "1-input/InputEnums.h"
#include "1-input/InputMgr.h"
#include "1-texts/TextMgr.h"
#include "1-json/jsoncpp/json.h"
#include "1-json/jsoncpp/reader.h"
#include "1-graphics/FrameBuffer.h"
#include "2-fonts/FontResource.h"
#include "2-application/OS.h"
#include "4-menus/imgui.h"
#include "4-ecs/JsonReflect.h"

#include "rapidjson/document.h"
#include "../tb64/turbob64.h"

#include "Str.h"

#include "r/all.hpp"
#include "rs/all.hpp"

#include "rd/all.hpp"

#include "rd/PointerWrapper.hpp"
#include "rd/LuaScriptHost.hpp"

#include "r2/all.hpp"
#include "r3/all.hpp"
#include "rui/all.hpp"
#include "fmt/all.hpp"


using namespace rs;

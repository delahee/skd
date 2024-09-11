#include "stdafx.h"

#include "1-time/Profiler.h"

#include "LuaScriptHost.hpp"
#include "ri18n/T.hpp"
#include "PointerWrapper.hpp"

#include "r2/im/HierarchyExplorer.hpp"
#include "r2/im/NodeExplorer.hpp"

#include "r3/Node3D.hpp"
#include "r3/Sprite3D.hpp"
#include "r3/Graphics3D.hpp"
//#include "AudioMan.hpp"

using namespace std;
using namespace r2;
using namespace rd;
using namespace Pasta;
using namespace rs;


#define SAFE_LUA(fnc) \
try { fnc } \
catch (const sol::error& e) { if (log) log(string("Lua Error\n") + e.what() + '\n'); else std::cout << "Lua Error\n" << e.what() << '\n'; } \
catch (std::exception& e) { if (log) log(string("Std Error\n") + e.what() + "\n"); else std::cout << "Std Error\n" << e.what() << '\n'; } \
catch (...) { if (log) log(string("Unknown Error\n")); else std::cout << "Unknown Error\n" << '\n'; }

typedef r::Color Color;

class TestGC {
public:
	int test = 0;
	TestGC() { std::cout << "Constructor default" << std::endl; }
	TestGC(int i) { std::cout << "Constructor " << i << std::endl; test = i; }
	~TestGC() { std::cout << "Destructor " << test << std::endl; }

	void testPointer(TestGC* pointer) {
		std::cout << "Pointer " << pointer->test << std::endl;
	}
};

static sol::state luaEngine = {};
bool rd::LuaScriptHost::globalStateInit = false;

LuaScriptHost::LuaScriptHost(const std::string & scriptPath, AgentList * _al, std::string _defaultFunc) : Super(_al), defaultFunc(_defaultFunc) {
	FileMgr* mgr = FileMgr::getSingleton();
	file = mgr->createFile(filePath = scriptPath);

#if _DEBUG
	if (!file) {
		printf("Warn:Lua::No such file %s\n", scriptPath.c_str());
	}
#endif
	init = false;
}

rd::LuaScriptHost::~LuaScriptHost() {
	onDestruction();
    for (r2::Node* node : destroyList)
        node->destroy();
	destroyList.clear();
	file->release();
	localEnvironment = {};
	onDispose();
	luaEngine.collect_garbage();
}

sol::state& rd::LuaScriptHost::getEngine() {
	return luaEngine;
}

void LuaScriptHost::eval() {
	init = false;

	FileMgr * mgr = FileMgr::getSingleton();

	file->load();
	char * rawContent = (char*)file->getContent();
	if (rawContent == nullptr) {
		traceError("ERR: no file content for lua script");
		return;
	}

	SAFE_LUA(
		if (!globalStateInit) {//build context is called once to ensure "structural calls" can be done ( think set_function )
			luaEngine = sol::state();
			buildContext();
			globalStateInit = true;
		}
		else {
			updateContext(luaEngine);
			//traceWarning("no need to build lua context");
		}
		funcRun = {};

		//ensure the env is global so we can pass variables
		localEnvironment = sol::environment(luaEngine, sol::create, luaEngine.globals());
		localEnvironment.set_function("END_SCRIPT", [this]() {
			traceWarning("END_SCRIPT called");
			deleteAsked = true;
		});
		localEnvironment["host"] = this;

		// here you can ensure the env is fed back with each host creation, be careful as globals will be shared, so if you want per host locals
		// you will have to feed them in a system variable with a unique id from caller
		// you probably don't want to do that anyway
		onAfterContextCreation.trigger();
		luaEngine.script(rawContent, localEnvironment, sol::script_default_on_error, (std::string("@") + file->getPath()));

		if(!defaultFunc.empty())
			funcRun = localEnvironment[defaultFunc];
		init = true;

		onBeforeEval.trigger();

		if (!defaultFunc.empty())
			funcRun();

		onAfterEval.trigger();
	)

	file->release();
	lastReadTime = mgr->getLastWriteTime(file->getConvertedPath());
}

void LuaScriptHost::callFunc(const char* name) {
	if (!init) {
		trace("no env to run lua func");
		return;
	}

    SAFE_LUA(localEnvironment[name]();)
}

void LuaScriptHost::reeval() {
	if (!init) return;
	PASTA_CPU_AUTO_MARKER("Script reeval");

    SAFE_LUA(
        onBeforeEval.trigger();

		funcRun();

		onAfterEval.trigger();
    )
}

bool rd::LuaScriptHost::im() {
	return false;
}

void LuaScriptHost::update(double dt) {
	checkProgress += dt;
	if (checkProgress >= DELTA_CHECK && watchFile) {
		FileMgr * mgr = FileMgr::getSingleton();
		Pasta::u64 writeTime = mgr->getLastWriteTime(file->getConvertedPath());
		if (writeTime > lastReadTime) {
			eval();
		}
		checkProgress = 0.0f;
	}
	if (evalEachFrame) {
		if(!init) eval();
		else reeval();
	}
	if (deleteAsked) 
		safeDestruction();
}

void LuaScriptHost::updateContext(sol::state& luaSol){

}

void LuaScriptHost::buildContext() {
	traceWarning("LuaScriptHost::buildContext");
	luaEngine.open_libraries(sol::lib::base, sol::lib::math, sol::lib::io, sol::lib::string, sol::lib::table, sol::lib::coroutine);

	injectR(luaEngine);
#ifndef HBC_NO_LUA_IMGUI
	injectImGui(luaEngine);
#endif
#ifndef HBC_NO_LUA_FMOD
	injectFmod(luaEngine);
#endif
	//luaEngine["tw"] = std::ref(tw);
	luaEngine.new_usertype<LuaScriptHost>("LuaScriptHost",
		"addToDestroyList", [](LuaScriptHost* self, r2::Node* n) {
			self->destroyList.push_back(n);
		},
        "cleanupDestroyList", [](LuaScriptHost* self) {
            for (r2::Node* node : self->destroyList)
                node->destroy();
            self->destroyList.clear();
        }
	);

	onBuildContext();
}

void LuaScriptHost::injectR(sol::state& luaSol) {

#pragma region REGION: Pasta
	luaSol.new_enum<TransparencyType>("TransparencyType", {
		{"TT_OPAQUE"	, TransparencyType::TT_OPAQUE },
		{"TT_CLIP_ALPHA", TransparencyType::TT_CLIP_ALPHA },
		{"TT_ALPHA"		, TransparencyType::TT_ALPHA },
		{"TT_ADD"		, TransparencyType::TT_ADD },
		{"TT_SCREEN"	, TransparencyType::TT_SCREEN },
		{"TT_MULTIPLY"	, TransparencyType::TT_MULTIPLY }
		});

	luaSol.new_usertype<Str>("Str",
		sol::constructors<Str(), Str(const char*)>(),
		"cpp_str", &Str::cpp_str,
		"set", sol::overload(
			sol::resolve<void(const char*)>(&Str::set),
			sol::resolve<void(const std::string&)>(&Str::set),
			sol::resolve<void(const Str&)>(&Str::set))
		);

	luaSol.new_usertype<Vector2>("Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y
	);

	luaSol.new_usertype<Vector3>("Vector3",
		sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z
	);

	luaSol.new_usertype<Matrix44>("Matrix44",
		"getRow", &Matrix44::getRow,
		"setRow", sol::resolve<void(int, const Vector4&)>(&Matrix44::setRow),
		"getCol", &Matrix44::getCol,
		"setCol", sol::resolve<void(int, const Vector4&)>(&Matrix44::setCol),
		"transpose", &Matrix44::transpose,
		"identity", sol::var(Matrix44::identity)
		);
#pragma endregion

#pragma region REGION: r
	luaSol.new_usertype<r::Color>("Color",
		sol::constructors<r::Color(), r::Color(float, float, float), r::Color(float, float, float, float)>(),
		"r", &r::Color::r,
		"g", &r::Color::g,
		"b", &r::Color::b,
		"a", &r::Color::a,
		sol::meta_function::multiplication, sol::resolve<r::Color(float) const>(&r::Color::operator*),
		sol::meta_function::division, sol::resolve<r::Color(float) const>(&r::Color::operator/),

		"mk", [](int rgb) {
			return r::Color(rgb, 1.0f);
		},

		"mulAlpha", &r::Color::mulAlpha,
			"lighten", &r::Color::lighten,

			"Black", sol::var(r::Color::Black),
			"White", sol::var(r::Color::White),
			"Black", sol::var(r::Color::Black),
			"Grey", sol::var(r::Color::Grey),
			"Red", sol::var(r::Color::Red),
			"Green", sol::var(r::Color::Green),
			"Blue", sol::var(r::Color::Blue),
			"Yellow", sol::var(r::Color::Yellow),
			"Cyan", sol::var(r::Color::Cyan),
			"Magenta", sol::var(r::Color::Magenta),
			"Orange", sol::var(r::Color::Orange),
			"Pink", sol::var(r::Color::Pink),
			"Violet", sol::var(r::Color::Violet)
			);

	luaSol.new_enum<rd::Dir>("DIRECTION", {
		{"UP",				rd::UP },
		{"DOWN",			rd::DOWN },
		{"LEFT",			rd::LEFT },
		{"RIGHT",			rd::RIGHT },
		{"UP_LEFT",			rd::UP_LEFT },
		{"UP_RIGHT",		rd::UP_RIGHT },
		{"DOWN_LEFT",		rd::DOWN_LEFT },
		{"DOWN_RIGHT",		rd::DOWN_RIGHT },
		{"DIAG_TL",			rd::DIAG_TL },
		{"DIAG_TR",			rd::DIAG_TR },
		{"DIAG_BR",			rd::DIAG_BR },
		{"DIAG_BL",			rd::DIAG_BL },
		{"UP_DOWN",			rd::UP_DOWN },
		{"LEFT_RIGHT",		rd::LEFT_RIGHT },
		{"UP_DOWN_LEFT",	rd::UP_DOWN_LEFT },
		{"UP_DOWN_RIGHT",	rd::UP_DOWN_RIGHT },
		{"UP_LEFT_RIGHT",	rd::UP_LEFT_RIGHT },
		{"DOWN_LEFT_RIGHT", rd::DOWN_LEFT_RIGHT },
		{"TLDR",			rd::TLDR },
		{"NONE",			rd::NONE }
	});
#pragma endregion

#pragma region REGION: rs
	luaSol.set_function("trace", sol::resolve<void(const char*)>(&rs::trace));

	luaSol.set_function("warning", sol::resolve<void(const char*)>(&rs::traceWarning));
	luaSol.set_function("traceWarning", sol::resolve<void(const char*)>(&rs::traceWarning));

	luaSol.set_function("error", sol::resolve<void(const char*)>(&rs::traceError));
	luaSol.set_function("traceError", sol::resolve<void(const char*)>(&rs::traceError));

	luaSol.set_function("TT", sol::resolve<const char* (const char*)>(&TT));
	luaSol.set_function("TU", sol::resolve<const char* (const char*)>(&TU));
	luaSol.set_function("TFst", sol::resolve< Str (const char*)>(&TFst));

	luaSol.set_function("TD", &TD);

	luaSol.set_function("nodeExplorer", [](r2::Node* n) {
		if (!n) return;
		r2::im::NodeExplorer::edit(n);
		});

	luaSol.set_function("rint", [](float f) -> int {
		return std::rint(f);
		});

	luaSol.set_function("cpp_str", [](const char* s) -> std::string {
		return std::string(s);
		});

	//for test at string/cchar* conversion
	luaSol.set_function("traceString", sol::resolve<void(const std::string&)>(&rs::trace));

	luaSol.new_enum<TVar>("TVar", {
		{"VNone"	, TVar::VNone		},
		{"VX"		, TVar::VX			},
		{"VY"		, TVar::VY			},
		{"VZ"		, TVar::VZ			},
		{"VScaleX"	, TVar::VScaleX		},
		{"VScaleY"	, TVar::VScaleY		},
		{"VAlpha"	, TVar::VAlpha		},
		{"VRotation", TVar::VRotation	},
		{"VR"		, TVar::VR			},
		{"VG"		, TVar::VG			},
		{"VB"		, TVar::VB			},
		{"VA"		, TVar::VA			},
		{"VScale"	, TVar::VScale		},
		{"VWidth"	, TVar::VWidth		},
		{"VHeight"	, TVar::VHeight		},
		{"VCustom0"	, TVar::VCustom0	},
		{"VCustom1"	, TVar::VCustom1	},
		{"VCustom2"	, TVar::VCustom2	},
		{"VCustom3"	, TVar::VCustom3	},
		{"VCustom4"	, TVar::VCustom4	},
		{"VCustom5"	, TVar::VCustom5	},
		{"VCustom6"	, TVar::VCustom6	},
		{"VCustom7"	, TVar::VCustom7	}
		});
	luaSol.new_usertype<ITweenable>("ITweenable");
	luaSol.new_usertype<rd::Garbage>("Garbage",
		"trash", sol::overload(
			sol::resolve<void(r2::Node*)>(&rd::Garbage::trash),
			sol::resolve<void(r2::BatchElem*)>(&rd::Garbage::trash))
		);

	//"findByName", sol::overload( sol::resolve<r2::Node* (const char*)>(&Node::findByName)),

	auto sysNamespace = luaSol["Sys"].get_or_create<sol::table>();
	sysNamespace.set_function("getUID", &rs::Sys::getUID);

	luaSol.new_usertype<rs::Timer>("Timer",
		"dt", sol::var(std::ref(rs::Timer::dt)),
		"now", sol::var(std::ref(rs::Timer::now)),
		"frameCount", sol::var(std::ref(rs::Timer::frameCount))
		);
#pragma endregion

#pragma region REGION: r2
	luaSol.new_usertype<GpuObjects>("GpuObjects",
		"whiteTile", sol::var(std::ref(GpuObjects::whiteTile))
	);

	luaSol.set_function("getWhiteTile", []() -> const r2::Tile* {
		return GpuObjects::whiteTile;
	});

	luaSol.set_function("getFont", [](const char * name) -> rd::Font* {
		return FontManager::get().getFont(name);
	});

	luaSol.new_usertype<Node>("Node",
		sol::base_classes, sol::bases<ITweenable>(),
		// A factory is used here to keep the pointer ownership to C++, Node.new(...) syntax
		sol::meta_function::construct, sol::factories([](Node* parent) { return new Node(parent); }),
		"destroy", &Node::destroy,

		"visible", &Node::visible,

		"x", &Node::x,
		"y", &Node::y,
		"z", &Node::z,

		"uid", &Node::uid,
		"name", &Node::name,
		"trsDirty", &Node::trsDirty,

		"parent", &Node::parent,
		"vars", &Node::vars,

		"rotation", &Node::rotation,
		"scaleX", &Node::scaleX,
		"scaleY", &Node::scaleY,

		"setPos", &Node::setPos,
		"setScale", &Node::setScale,
		"setScaleX", &Node::setScaleX,
		"setScaleY", &Node::setScaleY,

		"getSize", &Node::getSize,

		"alpha", &Node::alpha,
		"children", &Node::children,
		"parent", &Node::parent,

		"width", &Node::width,
		"height", &Node::height,

		"toBack", &Node::toBack,
		"toFront", &Node::toFront,
		"setName", &Node::setName,

		"syncAllMatrix", &Node::syncAllMatrix,
		"getGlobalMatrix", &Node::getGlobalMatrix,
		"getBounds", &Node::getBounds,
		"setAlpha", &Node::setAlpha,

		"findByName", sol::overload(
			sol::resolve<r2::Node* (const char*)>(&Node::findByName)),

		"destroyAllChildren", &Node::destroyAllChildren,
		"show", &Node::show,
		"hide", &Node::hide,

		"removeBhv", [](r2::Node* self) { self->bhv = nullptr; }
	);

	//removed to relax lua compiler pressure, use filterLinear() on node
	/*luaSol.new_usertype<r2::TexFilter>("TexFilter",
		"NEAREST", sol::var(TexFilter::TF_NEAREST),
		"LINEAR", sol::var(TexFilter::TF_LINEAR),
		"ANISO", sol::var(TexFilter::TF_ANISO)
	);*/

	luaSol.new_usertype<Scene>("Scene",
		sol::constructors<Scene(Node*)>(),
		sol::base_classes, sol::bases<Node, ITweenable>()
	);

	
	luaSol.new_usertype<Sprite>("Sprite",
		sol::base_classes, sol::bases<Node, ITweenable>(),
		// A factory is used here to keep the pointer ownership to C++, Sprite.new(...) syntax
		sol::meta_function::construct, sol::factories([](Node* parent) { return new Sprite(parent); }),
		"color", &Sprite::color,
		"setColor",
		sol::overload(
			sol::resolve<void(int, float)>(&Sprite::setColor),
			sol::resolve<void(const r::Color&)>(&Sprite::setColor)),
		"blendAdd", &Sprite::blendAdd,
		"blendAlpha", &Sprite::blendAlpha,
		"blendMultiply", &Sprite::blendMultiply,
		"texFilterLinear", &Sprite::texFilterLinear,
		"texFilterNearest", &Sprite::texFilterNearest,
		"depthRead", &Sprite::depthRead,
		"depthWrite", &Sprite::depthWrite,
		"killAlpha", &Sprite::killAlpha,

		"blendmode", &Sprite::blendmode
	);

	luaSol.new_usertype<TileAnimPlayer>("TileAnimPlayer",
		"unsync", &TileAnimPlayer::unsync
	);

	luaSol.new_usertype<Bitmap>("Bitmap",
		sol::base_classes, sol::bases<Sprite, Node, ITweenable>(),
		// A factory is used here to keep the pointer ownership to C++, Bitmap.new(...) syntax
		sol::meta_function::construct, sol::factories([](Node* parent) { return new Bitmap(parent); }),
		"tile", &Bitmap::tile,
		"fromImageFile", &Bitmap::fromImageFile,
		"fromTile", &Bitmap::fromTile,
		"setCenterRatio", &Bitmap::setCenterRatio
	);

	luaSol.new_usertype<Graphics>("Graphics",
		sol::base_classes, sol::bases<Sprite, Node, ITweenable>(),
		// A factory is used here to keep the pointer ownership to C++, Graphics.new(...) syntax
		sol::meta_function::construct, sol::factories([](Node* parent) { return new Graphics(parent); }),
		//todo add tri & overloads here
		//todo add quad overload here
		//"drawQuad", &Graphics::drawQuad,
		//"drawLine", &Graphics::drawLine,
		"drawLine", sol::overload(
			sol::resolve<void(float, float, float, float, float)>(&Graphics::drawLine),
			sol::resolve<void(const Vector3&, const Vector3&, float)>(&Graphics::drawLine)
		),
		"setDefaultColor", sol::overload(
			sol::resolve<void(const r::Color&)>(&Graphics::setGeomColor),
			sol::resolve<void(int, float)>(&Graphics::setGeomColor)
		)
	);

	luaSol.new_usertype<r3::Graphics3D>("Graphics3D",
		sol::base_classes, sol::bases<r3::Sprite3D, r3::Node3D, r2::Node, ITweenable>(),
		sol::meta_function::construct, sol::factories([](Node* parent) { return new r3::Graphics3D(parent); }),
		"setDefaultColor", sol::overload(
			sol::resolve<void(const r::Color&)>(&r3::Graphics3D::setGeomColor),
			sol::resolve<void(int, float)>(&r3::Graphics3D::setGeomColor)
		),
		"setGeomColor", sol::overload(
			sol::resolve<void(const r::Color&)>(&r3::Graphics3D::setGeomColor),
			sol::resolve<void(int, float)>(&r3::Graphics3D::setGeomColor)
		),
		"drawLine", sol::overload(
			sol::resolve<void(const vec3& start, const vec3& end, float thicc)>(&r3::Graphics3D::drawLine))
	);

	luaSol.new_usertype<Tile>("Tile",
		"x", &Tile::x, "y", &Tile::y,
		"width", &Tile::width,
		"height", &Tile::height,
		"dx", &Tile::dx, "dy", &Tile::dy,
		"u1", &Tile::u1, "v1", &Tile::v1,
		"u2", &Tile::u2, "v2", &Tile::v2
	);

	luaSol.new_usertype<Text>("Text",
		sol::base_classes, sol::bases<Batch, Sprite, Node, ITweenable>(),
		// A factory is used here to keep the pointer ownership to C++, Text.new(...) syntax
		sol::meta_function::construct, sol::factories([](Font* fnt, const char* txt, Node* par) { return new Text(fnt, txt, par); }),
		"addDropShadow", &Text::addDropShadow,
		"setText", sol::resolve<const char* (const char*)>(&Text::setText),
		"getText", &Text::getText,
		"setFontSize", &Text::setFontSize,
		"getFontSize", &Text::getFontSize,
		"setBlockAlign", &Text::setBlockAlign,

		"ALIGN_LEFT", sol::var(static_cast<int>(Text::ALIGN_LEFT)),
		"ALIGN_TOP", sol::var(static_cast<int>(Text::ALIGN_TOP)),
		"ALIGN_RIGHT", sol::var(static_cast<int>(Text::ALIGN_RIGHT)),
		"ALIGN_BOTTOM", sol::var(static_cast<int>(Text::ALIGN_BOTTOM)),
		"ALIGN_HCENTER", sol::var(static_cast<int>(Text::ALIGN_HCENTER)),
		"ALIGN_VCENTER", sol::var(static_cast<int>(Text::ALIGN_VCENTER)),
		"ALIGN_CENTER", sol::var(static_cast<int>(Text::ALIGN_CENTER))
	);

#pragma endregion

#pragma region REGION: rd
	luaSol.new_enum<TType>("TType", {
		{"TLinear"		, TType::TLinear		},
		{"TLoop"		, TType::TLoop			},
		{"TLoopEaseIn"	, TType::TLoopEaseIn	},
		{"TLoopEaseOut"	, TType::TLoopEaseOut	},
		{"TCos"			, TType::TCos			},
		{"TSin"			, TType::TSin			},
		{"TEase"		, TType::TEase			},
		{"TEaseIn"		, TType::TEaseIn		},
		{"TEaseOut"		, TType::TEaseOut		},
		{"TBurn"		, TType::TBurn			},
		{"TBurnIn"		, TType::TBurnIn		},
		{"TBurnOut"		, TType::TBurnOut		},
		{"TZigZag"		, TType::TZigZag		},
		{"TJump"		, TType::TJump			},
		{"TBounceOut"	, TType::TBounceOut		},
		{"TShake"		, TType::TShake			},
		{"TElasticEnd"	, TType::TElasticEnd	},
		{"TQuad"		, TType::TQuad	},
		{"TCubic"		, TType::TCubic	}
	});

	luaSol.new_usertype<Tweener>("Tweener",
		"create", &Tweener::create,//rs::ITweenable* parent, rs::TVar varName, double to, rd::TType tp = TType::TLinear, double duration_ms = -1.0
		"createFromTo", &Tweener::createFromTo,
		"delay", &Tweener::delay,
		"delayFromTo", &Tweener::delayFromTo
	);

	luaSol.new_usertype<Tween>("Tween",
		"to", &Tween::to,
		"nbPlays", &Tween::nbPlays
	);

	luaSol.new_usertype<ColorLib>("ColorLib",
		"colorHSV", &ColorLib::colorHSV,
		"colorColorize", &ColorLib::colorColorizeInt,
		"toInt", sol::overload(
			sol::resolve<unsigned int(const std::string&)>(ColorLib::toInt),
			sol::resolve<unsigned int(r::Color)>(ColorLib::toInt)
		)
	);

	luaSol.new_usertype<TileLib>("TileLib", //Todo add variant for optional parameters
		"getTile", sol::resolve<r2::Tile*(const char*, int, float, float, r2::Tile*)>(&TileLib::getTile)
	);

	luaSol.new_usertype<rd::Sig>("Sig",
		"add", &rd::Sig::add,
		"addOnce", &rd::Sig::addOnce,
		"trigger", &rd::Sig::trigger,
		"clear", &rd::Sig::clear
	);

	luaSol.new_usertype<Rand>("Rand",
		"get", &Rand::get,
		"dice", &Rand::dice,
		"diceF", &Rand::diceF,
		"pc", 
		sol::overload(
			sol::resolve<bool(int)>(&Rand::pc),
			sol::resolve<bool(float)>(&Rand::pc)
		)
	);
	
	luaSol.new_usertype<rd::ABitmap>("ABitmap",
		sol::base_classes, sol::bases<r2::Bitmap, r2::Sprite, Node, ITweenable>(),
		"play", sol::overload(
			sol::resolve<void(const char*, bool)>(&rd::ABitmap::play)
		),
		"stop", &rd::ABitmap::stop,
		"set", sol::overload(
			sol::resolve<void(TileLib*, const std::string&, int , bool )>(&rd::ABitmap::set)
		),
		"setFlippedX",&rd::ABitmap::setFlippedX,
		"setFlippedY",&rd::ABitmap::setFlippedY,
		"andDestroy",&rd::ABitmap::andDestroy,
		"getLib",&rd::ABitmap::getLib
	);

	luaSol.new_usertype<rd::Promise>("Promise",
		"isSettled", &rd::Promise::isSettled,
		"autoReleased", &rd::Promise::autoReleased,
		"getSuccess", &rd::Promise::getSuccess,
		"getFailure", &rd::Promise::getFailure
	);

	luaSol.new_usertype<Vars>("Vars",
		"setString", [](Vars* self, const char* name, const char* value) {
			self->set(name, value);
		},
		//"setInt", [](Vars* self, const char* name, int value) {
		//	self->set(name, value);
		//},
		"setInt", sol::overload(
			[](Vars* self, const char* name, int value){
				self->set(name, value);
			},
			[](Vars* self, std::string name, int value) {
				self->set(name.c_str(), value);
			}
		),
		"setFloat", [](Vars* self, const char* name, float value) {
			self->set(name, value);
		},
		"setBool", [](Vars* self, const char* name, bool value) {
			self->set(name, value);
		},
		"getString", &Vars::getString,
		"getInt", &Vars::getInt,
		"getFloat", &Vars::getFloat,
		"getBool", &Vars::getBool,
		"hasTag", &Vars::hasTag,
		"setTag", &Vars::setTag,
		"addTag", &Vars::addTag,
		"has", &Vars::has
	);
#pragma endregion

#pragma region REGION: Debug
	luaSol.new_usertype<TestGC>("TestGC",
		sol::constructors<TestGC(), TestGC(int)>(),
		"testPointer", &TestGC::testPointer
	);
#pragma endregion
}

#include "rd/Console.hpp"
#include "rplatform/CrossPlatform.hpp"
void rd::Console::mkLuaHost() {
	hostLua->collect_garbage();
	*hostLua = sol::state();
	auto& state = *hostLua;
	state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::io, sol::lib::string, sol::lib::table);
	state.set_function("log", sol::overload(
		[this](const char* str) { log(str); },
		[this](std::string str) { log(str); }
	));

	state.set_function("dump_log", [this] {
		rs::File::writeSaveFile("log.txt", fullLog);
		});
	/*
	state.set_function("sim_assert", [this] {
		rs::Timer::delay(1, []() {
			PASTA_ASSERT_MSG(false, "simulated catastrophe");
			});
		});

	state.set_function("sim_cs", [this] {
		std::string str;
		rplatform::getCallStack(str);
		trace(str);
	});

	state.set_function("sim_crash", [this] {
		rs::Timer::delay(1, []() {
			r2::Node* t = nullptr;
			r2::Node& lt = *t;
			lt.x = 5;
			});
		});

	state.set_function("sim_div0", [this] {
		rs::Timer::delay(1, []() {
			volatile float z = 0.0f;
			volatile int div = 1 / z;
			});
		});*/
}


#include "stdafx.h"

#include "LuaScriptHost.hpp"
#include "1-files/FileMgr.h"
#include "1-time/Profiler.h"

using namespace std;
using namespace r2;
using namespace rd;
using namespace Pasta;

#define SUPER Agent
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

sol::state rd::LuaScriptHost::luaEngine = nullptr;
bool rd::LuaScriptHost::globalStateInit = false;

LuaScriptHost::LuaScriptHost(const std::string & scriptPath, AgentList * _al, std::string _defaultFunc) : SUPER(_al), defaultFunc(_defaultFunc) {
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
	file->release();
	dispose();
	luaEngine.collect_garbage();
}

static void my_panic(sol::optional<std::string> maybe_msg) {
	std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
	if (maybe_msg) {
		const std::string& msg = maybe_msg.value();
		std::cerr << "\terror message: " << msg << std::endl;
	}
	// When this function exits, Lua will exhibit default behavior and abort()
}

void LuaScriptHost::eval() {
	init = false;

	FileMgr * mgr = FileMgr::getSingleton();

	file->load();
	char * rawContent = (char*)file->getContent();
	if (rawContent == nullptr)
		return;

	try {
		if (!globalStateInit) {
			luaEngine = sol::state(sol::c_call<decltype(&my_panic), &my_panic>);
			buildContext();
			globalStateInit = true;
		}
		tw.clear();
		funcRun = {};

		localEnvironment = sol::environment(luaEngine, sol::create, luaEngine.globals());
		localEnvironment.set_function("END_SCRIPT", [this]() {
			deleteAsked = true;
		});
		onAfterContextCreation.trigger();
		luaEngine.script(rawContent, localEnvironment);

		funcRun = localEnvironment[defaultFunc];
		init = true;

		onBeforeEval.trigger();

		funcRun();

		onAfterEval.trigger();
	} catch (const sol::error& e) {
		if (log)
			log(string("Lua Error\n") + e.what() + '\n');
		else
			std::cout << "Lua Error\n" << e.what() << '\n';
	} catch (std::exception &e) {
		if (log)
			log(string("Std Error\n") + e.what() + "\n");
		else
			std::cout << "Std Error\n" << e.what() << '\n';
	}
	catch(...){
		if (log)
			log(string("Unknown Error\n"));
		else
			std::cout << "Unknown Error\n" << '\n';
	}
	file->release();
	lastReadTime = mgr->getLastWriteTime(file->getConvertedPath());
}

void LuaScriptHost::reeval() {
	if (!init) return;
	PASTA_CPU_AUTO_MARKER("Script reeval");

	try {
		onBeforeEval.trigger();

		funcRun();

		onAfterEval.trigger();
	} catch (const sol::error& e) {
		if (log)
			log(string("Lua Error\n") + e.what() + '\n');
		else
			std::cout << "Lua Error\n" << e.what() << '\n';
	} catch (std::exception &e) {
		if (log)
			log(string("Std Error\n") + e.what() + "\n");
		else
			std::cout << "Std Error\n" << e.what() << '\n';
	}
	catch(...){
		if (log)
			log(string("Unknown Error\n"));
		else
			std::cout << "Unknown Error\n" << '\n';
	}

}

void rd::LuaScriptHost::im() {

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
	tw.update(dt);
	if (deleteAsked) {
		delete this;
	}
}

void LuaScriptHost::buildContext() {
	luaEngine.open_libraries(sol::lib::base, sol::lib::math, sol::lib::io, sol::lib::string, sol::lib::table, sol::lib::coroutine);

	injectR(luaEngine);
	injectImGui(luaEngine);
	injectFmod(luaEngine);
	luaEngine["tw"] = std::ref(tw);
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

	luaSol.new_enum<DIRECTION>("DIRECTION", {
		{"UP",				DIRECTION::UP },
		{"DOWN",			DIRECTION::DOWN },
		{"LEFT",			DIRECTION::LEFT },
		{"RIGHT",			DIRECTION::RIGHT },

		{"UP_LEFT",			DIRECTION::UP_LEFT },
		{"UP_RIGHT",		DIRECTION::UP_RIGHT },
		{"DOWN_LEFT",		DIRECTION::DOWN_LEFT },
		{"DOWN_RIGHT",		DIRECTION::DOWN_RIGHT },

		{"DIAG_TL",			DIRECTION::DIAG_TL },
		{"DIAG_TR",			DIRECTION::DIAG_TR },
		{"DIAG_BR",			DIRECTION::DIAG_BR },
		{"DIAG_BL",			DIRECTION::DIAG_BL },

		{"UP_DOWN",			DIRECTION::UP_DOWN },
		{"LEFT_RIGHT",		DIRECTION::LEFT_RIGHT },

		{"UP_DOWN_LEFT",	DIRECTION::UP_DOWN_LEFT },
		{"UP_DOWN_RIGHT",	DIRECTION::UP_DOWN_RIGHT },
		{"UP_LEFT_RIGHT",	DIRECTION::UP_LEFT_RIGHT },
		{"DOWN_LEFT_RIGHT", DIRECTION::DOWN_LEFT_RIGHT },
		{"TLDR",			DIRECTION::TLDR },

		{"NONE",			DIRECTION::NONE }
		});
#pragma endregion

#pragma region REGION: rs
	luaSol.set_function("trace", sol::resolve<void(const char*)>(&rs::trace));

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

		"syncAllMatrix", &Node::syncAllMatrix,
		"getGlobalMatrix", &Node::getGlobalMatrix,
		"getBounds", &Node::getBounds,

		"findByName", sol::overload(
			sol::resolve<r2::Node*(const char* )>( &Node::findByName)),

		"destroyAllChildren", & Node::destroyAllChildren,
		"show", & Node::show,
		"hide", & Node::hide
	);

	luaSol.new_usertype<r2::TexFilter>("TexFilter",
		"NEAREST",sol::var(TexFilter::TF_NEAREST),
		"LINEAR",sol::var(TexFilter::TF_LINEAR),
		"ANISO",sol::var(TexFilter::TF_ANISO)
	);

	luaSol.new_usertype<Scene>("Scene",
		sol::constructors<Scene(Node*)>(),
		sol::base_classes, sol::bases<Node, ITweenable>()
	); 
		
	luaSol.new_usertype<OffscreenScene>("OffscreenScene",
		sol::base_classes, sol::bases<Scene, Node, ITweenable>()
	);

	luaSol.new_usertype<OffscreenScene>("EarlyDepthScene",
		sol::base_classes, sol::bases<Scene, Node, ITweenable>()
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
		"blendmode", &Sprite::blendmode
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
		"setText", sol::resolve<std::string& (const char*)>(&Text::setText),
		"getText", &Text::getText,
		"setFontSize", &Text::setFontSize,
		"getFontSize", &Text::getFontSize,
		"setBlockAlign", &Text::setBlockAlign,

		"ALIGN_LEFT", sol::var(Text::ALIGN_LEFT),
		"ALIGN_TOP", sol::var(Text::ALIGN_TOP),
		"ALIGN_RIGHT", sol::var(Text::ALIGN_RIGHT),
		"ALIGN_BOTTOM", sol::var(Text::ALIGN_BOTTOM),
		"ALIGN_HCENTER", sol::var(Text::ALIGN_HCENTER),
		"ALIGN_VCENTER", sol::var(Text::ALIGN_VCENTER),
		"ALIGN_CENTER", sol::var(Text::ALIGN_CENTER)
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
		"create", &Tweener::create,
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
		"pc", &Rand::pc
	);

	
	luaSol.new_usertype<rd::ABitmap>("ABitmap",
		sol::base_classes, sol::bases<r2::Bitmap, r2::Sprite, Node, ITweenable>(),
		"play",&rd::ABitmap::play,
		"stop", &rd::ABitmap::stop,
		"set", sol::overload(
			sol::resolve<void(TileLib*, const std::string&, int , bool )>(&rd::ABitmap::set)
		),
		"setFlippedX",&rd::ABitmap::setFlippedX,
		"setFlippedY",&rd::ABitmap::setFlippedY,
		"getLib",&rd::ABitmap::getLib
	);
	

	luaSol.new_usertype<Vars>("Vars",
		"setString", [](Vars* self, const char* name, const char* value) {
			self->set(name, value);
		},
		"setInt", [](Vars* self, const char* name, int value) {
			self->set(name, value);
		},
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

#undef SUPER
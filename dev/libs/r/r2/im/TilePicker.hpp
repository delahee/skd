#pragma once

#include "rd/Agent.hpp"
#include "rd/TileLib.hpp"

namespace r2 {
	namespace im {
		class TilePicker : public rd::Agent {
			public:

				virtual void				update(double dt) override;

				void						forceClose();
				static void					forBitmap(r2::Bitmap* bmp);
				static rd::Promise*			forTile(r2::Tile & tile, rd::Vars* store = 0);
				static rd::Promise*			forPackage(rd::TilePackage & t);
				static rd::Promise*			forPicker();
				void						visit(Pasta::JReflect& functor);

				r2::Tile*					pickedTile = nullptr;
				rd::TileGroup*				pickedGroup = nullptr;
				rd::TileLib*				pickedLib = nullptr;
				r::Color					pickedColor;
			public:
				
				static rd::TileLib*			getLib(const std::string & name);
				static rd::TileLib*			getLib(const Str& name) { return getLib(name.c_str()); };
				static rd::TileLib*			getLib(const char* name);

				//if the caches gets bloated, modify the tile picker to maintain an internal stack of libs and know what to flush
				static rd::TileLib*			getOrLoadLib(const char* name, rd::TPConf* tpc = nullptr);
				static rd::TileLib*			getOrLoadLib(const std::string& name, rd::TPConf* tpc = nullptr);
				static rd::TileLib*			getOrLoadLib(const Str& name, rd::TPConf* tpc = nullptr) { return getOrLoadLib(name.c_str(), tpc); };
				static void					unloadLib(const char * name);
				static void					registerTileSource(rd::TileLib * lib);
				static void					unregisterTileSource(rd::TileLib * lib);
				static bool					hasLib(const char * name);

				static std::vector<rd::TileLib*>	sources;
			protected:
				bool						opened = true;
				bool						scheduleTermination = false;
				rd::Promise*				prom = nullptr;

											TilePicker(rd::Promise* prom);
				virtual						~TilePicker();

				void						load();
				void						save();

				void						imTiles(rd::TileLib* lib);
				
				std::string					curSearch;
				bool						excludeAnimSubFrames = true;
				bool						preview = false;
				bool						animated = true;
		};
	}
}

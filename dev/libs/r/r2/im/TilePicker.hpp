#pragma once

#include <vector>
#include "rd/Agent.hpp"
#include "rd/TileLib.hpp"
#include "4-ecs/JsonReflect.h"

namespace r2 {
	namespace im {
		class TilePicker : public rd::Agent {
			public:

				virtual void				update(double dt) override;

				void						forceClose();
				static void					forBitmap(r2::Bitmap* bmp);
				static Promise*				forTile(r2::Tile & tile);
				static Promise*				forPackage(rd::TilePackage & t);
				static Promise*				forPicker();
				void						visit(Pasta::JReflect& functor);

				r2::Tile*					pickedTile = nullptr;
				rd::TileGroup*				pickedGroup = nullptr;
				rd::TileLib*				pickedLib = nullptr;
				r::Color					pickedColor;
			public:
				
				static rd::TileLib*			getLib(const std::string & name);
				static rd::TileLib*			getLib(const char* name);

				//if the caches gets bloated, modify the tile picker to maintain an internal stack of libs and know what to flush
				static rd::TileLib*			getOrLoadLib(const std::string& name, TPConf* tpc = nullptr);
				static void					registerTileSource(rd::TileLib * lib);
				static void					unregisterTileSource(rd::TileLib * lib);
				static bool					hasLib(const char * name);

				static std::vector<rd::TileLib*>	sources;
			protected:
				bool						opened = true;
				bool						scheduleTermination = false;
				Promise* prom = nullptr;

				static void					pick(Promise* prom);
				void						load();
				void						save();

											TilePicker(Promise* prom);
				virtual						~TilePicker();
											void imTiles(rd::TileLib* lib);

				
				std::string					curSearch;
				bool						excludeAnimSubFrames = true;
				bool						preview = false;
				bool						animated = true;
		};
	}
}

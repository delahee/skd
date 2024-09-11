#pragma once

#include "r2/Tile.hpp"
#include "rd/TileLib.hpp"

namespace r2 { class Node; }

namespace fmt{
	namespace hed {
		class SlbEntry {
		public:
			std::string		path;
			std::string		shorthand;
			rd::TileLib*	slb=0;
			r2::Node*		pane=0; 
		};

		class BmpEntry {
		public:
			std::string		path;
			std::string		shorthand;
			r2::Tile*		tile=0;
		};

		class TileOrigin {
		public:
			std::string		bmp;
			std::string		sheet;
			std::string		group;
			
			int				color=0;
			float			pivotX=0.f;
			float			pivotY=0.f;
		};
	}

	class CurveControlPoint : public r2::Node {
	public:
		CurveControlPoint(r2::Node * parent) : r2::Node(parent){
			
		};
	};

	class CurveNode : public r2::Node {
	public:
		CurveNode(r2::Node * parent) : r2::Node(parent) {

		};
	};

	class CurveLine {
	public:
		std::vector<CurveControlPoint*> cps;

		CurveLine(std::vector<CurveControlPoint*> cps) {
			this->cps = cps;
		}

		static CurveLine * fromCPS(std::vector<CurveControlPoint*> cps) {
			CurveLine* c = new CurveLine(cps);
			return c;
		}
	};

	struct CurveEntry{
		CurveNode * start = nullptr;
		CurveNode * end = nullptr;
		CurveLine * line = nullptr;

		~CurveEntry() {
			delete line;
			line = nullptr;
			end = start = nullptr;
		}
	};

	class CurveMan {
	public:
		std::vector<CurveNode*> nodes;
		std::vector<CurveEntry*> curves;

		void load(Json::Value & d, r2::Node * scene);
	};

	struct HedJsonLevelConf {
		r2::TexFilter filter = r2::TexFilter::TF_NEAREST;
	};

	class HedJsonLevel {
		public:
			double					LIB_SPEED = 1.0;
			bool					fitPixels = true;
			HedJsonLevelConf		conf;
			CurveMan				cman;

			std::unordered_map<std::string, hed::SlbEntry> slib;
			std::unordered_map<std::string, hed::BmpEntry> bmap;

		public:
									HedJsonLevel(HedJsonLevelConf conf);

			void					parseAndLoad(std::string path, r2::Node * scene);

			//TODO
			//void					save();

			bool					loadToScene(Json::Value * root, r2::Node * scene);
			fmt::hed::TileOrigin	getOrigin(Json::Value & tile);
			void					loadRec(Json::Value & d, r2::Node * current);
			
			bool					loadGfx(Json::Value * r);
			std::string				mkShortHand(std::string path);
			void					importBitmap(std::string path );
			void					importLib(std::string path );
			void					resetTRS(r2::Node * sc);
			void					cleanup();
			void					dispose();

	protected:
			rd::JSONResource json;
		
	};

}// end namespace fmt
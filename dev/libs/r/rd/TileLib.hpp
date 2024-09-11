#pragma once

#include "r2/Rect.hpp"
#include "r2/Tile.hpp"

#include "IAnimated.hpp"

namespace Pasta {
	struct JReflect;
}

namespace r2{
	class Bitmap;
	class BatchElem;
}

namespace rd{
	struct RealFrame {
		int x = 0;
		int y = 0;
		int realWid = 0;
		int realHei = 0;

		RealFrame(int _x, int _y, int _realWid, int _realHei) : x(_x), y(_y), realWid(_realWid), realHei(_realHei) {

		}

		RealFrame() {}

		RealFrame(const RealFrame & rf ) {
			x = rf.x;
			y = rf.y;
			realWid = rf.realWid;
			realHei = rf.realHei;
		}
	};

	struct FrameData {
		bool	hasPivot = false;

		int		x		= 0;
		int		y		= 0;
		int		wid		= 0;
		int		hei		= 0;
		int		texSlot	= 0;
		double	pX		= 0.0;
		double	pY		= 0.0;

		RealFrame realFrame;
		r2::Rect rect;

		FrameData() {}
		FrameData(const FrameData &fd) {
			x = fd.x;
			y = fd.y;
			wid = fd.wid;
			hei = fd.hei;
			realFrame = fd.realFrame;
			rect = fd.rect;
			hasPivot = fd.hasPivot;
			pX = fd.pX;
			pY = fd.pY;
			texSlot = fd.texSlot;
		}

		inline float getDX(float pivotRatioX) {
			return -(realFrame.realWid * pivotRatioX + realFrame.x);
		};

		inline float getDY(float pivotRatioY) {
			return -(realFrame.realHei * pivotRatioY + realFrame.y);
		};

		void im();
	};

	class TileGroup {
	public:
		Str						id;
		int						maxWid=0;
		int						maxHei=0;
		std::vector<FrameData>	frames;//avoid to tap into this one, favor frames[anim[frame]] which starts from zero
		std::vector<int>		anim;
		TileLib*				lib = nullptr;

		TileGroup(const char *_id,TileLib * _lib):id(_id),lib(_lib) {

		};

		TileGroup(const TileGroup& grp) {
			id = grp.id;
			maxWid = grp.maxWid;
			maxHei = grp.maxHei;
			frames = grp.frames;
			anim = grp.anim;
			lib = nullptr;
		};

		void im();
	};

	class TileLib {
	public:
		static bool					RETAIN_TEXTURE_DATA;//=false
		bool						hasChanged			= false;
		float						defaultCenterX		= 0.0f;
		float						defaultCenterY		= 0.0f;
		double						defaultFrameRate	 = (1.0 / 0.066);
		double						speed	= 1.0;
		
		r::Texture*					tex		= nullptr;
		r2::Tile*					tile	= nullptr;
		
		std::string					defaultTileName;
		std::string					name;
		std::unordered_map<Str, TileGroup*> 
									groups;
		std::vector<r::Texture*>	textures;
		std::vector<r2::Tile*>		tiles;
		std::vector<std::string>	updateTags;

									TileLib();
									~TileLib();

		void						destroy();
		void						setDefaultTile( const char * name );
		void						setPageUpdateTag(const char* updateTag);
		bool						loadImage(const std::string & path, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, bool keepTextureData = false);
		void						defineAnim(const std::string & name, std::vector<int> & frames);
		void						sliceCustom(const std::string & groupName, int frame, int x, int y, int wid, int hei, rd::RealFrame & rf );
		
		TileGroup*					createGroup(const char * k);
		TileGroup*					getGroup(const char * k);
		
		r2::Tile *					getTile(const char * str, int frame = 0, float px = 0.0, float py = 0.0, r2::Tile * res = nullptr);
		r2::Tile *					getTile(const std::string & str, int frame, float px = 0.0, float py = 0.0, r2::Tile * res = nullptr);
		r2::Tile *					getTile(const Str & str, int frame, float px = 0.0, float py = 0.0, r2::Tile * res = nullptr);
		

		static rd::TileLib*			get(const char* lib);

		//used for scripts
		r2::Tile *		_getTile(const std::string & str);

		FrameData *		getFrameData(const char * str, int frame = 0);
		FrameData *		getFrameData(const std::string & str, int frame = 0) { return getFrameData(str.c_str(), frame); };
		FrameData *		getFrameData(const Str & str, int frame = 0) { return getFrameData(str.c_str(), frame); };
		
		//warning, can exist but not be an anim
		bool			isAnim(const std::string & str) { return isAnim(str.c_str()); }
		bool			isAnim(const Str & str) { return isAnim(str.c_str()); }
		bool			isAnim(const char * k);

		//warning, can exist but not be an anim
		bool			exists(const char* k, int frame = 0);
		bool			exists(const std::string & str, int frame = 0);
		bool			exists(const Str& str, int frame = 0) { return exists(str.c_str()); };
		
		bool			hasSimilar(const char* grp);
		void			getSimilar(const char* grp, rd::TileGroup*& res);
		const char*		getSimilar(const char* grp);

		std::string			getTagSignature();

		static TileLib*		mock(const TileLib* src, TileLib*preallocated = 0);

		void			aliasGroup(const char * newName, const char* oldName);
		void			setup(r2::Bitmap* to, const char* grp, int frm=0, float px=0.0f, float py=1.0f);
		void			setup(r2::BatchElem* to, const char* grp, int frm=0, float px=0.0f, float py=1.0f);
		void			im();
	public:

	protected:
		Str				imFilter;
		
	};

	struct TilePackage {
		Str 			lib;
		Str				group;

		r2::Tile*		tile = 0;
		rd::TileLib*	cachedLib = 0;

		bool			dirty = false;

		TilePackage();
		TilePackage(const TilePackage& tp);
		~TilePackage();

		TilePackage& operator=(const TilePackage& tp) {
			lib = tp.lib;
			group = tp.group;
			if (!tile)
				tile = r2::Tile::fromWhite();
			tile->copy( *tp.tile );
			return *this;
		}

		inline bool		isReady()const { return 0 != tile; };

		void			dispose();

		void			setAnon(r2::Tile* t);
		void			empty();

		TilePackage		clone() const;

		bool			im();
		void			serialize(Pasta::JReflect* j, const char* _name = 0);
		rd::TileLib*	getLib();

		void			writeTo(rd::Vars& v)const;
		bool			readFrom(const rd::Vars& v);
	};
}//end namespace rd
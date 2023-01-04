#pragma once

#include <string>

#include "1-graphics/Texture.h"
#include "EASTL/vector.h"
#include "EASTL/unordered_map.h"

namespace r2 {
	namespace svc {

		struct PaletteRange {
			std::string									name;
			eastl::vector<uint32_t>						index;
		};

		//keep TextureData cached
		//create instances 
		//keeps the mainpalette
		//big warning we don't manage texture backing format
		class ChangePalette {
		public:
			std::string					path;
			Pasta::TextureData *		data = nullptr;
			eastl::vector<uint32_t>		mainPalette;
			eastl::vector<uint32_t>		curPalette;
			eastl::vector<PaletteRange>	ranges;
			Pasta::TextureData 			workData;

			ChangePalette();
			ChangePalette(Pasta::TextureData* data);
			ChangePalette(const std::string& root);
			~ChangePalette();

			void dispose();

			void						reset(const std::string& root);
			eastl::vector<uint32_t>		extractPalette();

			eastl::vector<uint32_t>		extractPalette(Pasta::TextureData * data);
			void						setMainPalette(Pasta::TextureData * data);

			void						setMainPalette(const eastl::vector<uint32_t>& mainPalette);
			void						extractRange(const std::string & rangeName, eastl::vector<uint32_t>& paletteToDiff);

			//warning takes indexes not colors
			void						resetImage();
			void						resetRange(const std::string & rangeName);
			void						addRange(const std::string & rangeName, eastl::vector<uint32_t>&index);

			void						changeAll(const Pasta::Matrix44 & mat);
			void						changeAll(const eastl::vector<uint32_t>& nuColors);

			void						change(const std::string & rangeName, const Pasta::Matrix44 & mat);

			//send a full palette with what changed among old palette
			void						change(const std::string & rangeName, const eastl::vector<uint32_t>& nuColors);

			void						changeSlice(const eastl::vector<uint32_t>& idx, const Pasta::Matrix44 & mat);
			void						apply(const eastl::unordered_map<uint32_t, uint32_t> & changeMap);

			void						reset();
			bool						isInRange(const char* rgName, uint32_t color);
			PaletteRange*				getRange(const char * rgName);

			eastl::unordered_map<uint32_t /*color*/, uint32_t /*palettedIdx*/> colorToIdx;
		protected:
			void						beginChanges();
			bool						loadTextureData();
		};
	}
}
#pragma once 

#include "r/Color.hpp"
#include "1-graphics/geo_vectors.h"

typedef Pasta::Matrix44 Matrix44;


struct ColorBus {
	r::Color					value;
	eastl::vector<r::Color*>	sources;

	void update(){
		if (sources.size() == 0) {
			value = r::Color::White;
		}
		else {
			for (auto& c : sources) 
				value += *c;
			value /= sources.size();
		}
	}
};

namespace rd {
	class ColorLib {
	public:
		static r::Color	getHSV(r::Color rgb );
		static r::Color	getGreyscale(r::Color c);
		static r::Color	saturate(r::Color c, float delta);
		static r::Color	fromHSV(r::Color hsv);
		static r::Color	fromInt(unsigned int argb);
		static r::Color	fromString( const char * val);
		static r::Color	fromString( const std::string & val );

		//erase rgb high bits and replace with alpha
		static r::Color	get(unsigned int rgb, float alpha = 1.0);

		static unsigned int	toInt(const std::string & c);
		static unsigned int	toInt(r::Color c);

		/**
		* identity is one
		*/
		static Matrix44&	colorSaturation(Pasta::Matrix44 & mat, float sat);

		/**
		* identity is zero
		*/
		static Matrix44&	colorContrast(Pasta::Matrix44 & mat, float contrast);

		/**
		* identity is zero
		* @param	brightness
		*/
		static Matrix44&	colorBrightness(Pasta::Matrix44 &mat, float brightness);

		/**
		* identity is zero
		*/
		static Matrix44&	colorHue(Pasta::Matrix44 & mat, float hue);


		//float hue,float sat,float val
		//float	 hue = 0;sat = 1,val = 1, 
		// will affect ownMatrix only
		// hue [0,360]
		// sat [0,2]
		// val [0,2]
		static Matrix44&	colorHSV(Pasta::Matrix44 & mat, float hue, float sat, float value);

		static Matrix44&	colorColorizeInt(Pasta::Matrix44 & mat, unsigned int col, float ratioNewColor = 1.0, float ratioOldColor = 1.0, float alpha = 1.0);
		static Matrix44&	colorColorize(Pasta::Matrix44 & mat, const r::Color & col, float ratioNewColor = 1.0, float ratioOldColor = 1.0, float alpha = 1.0);
	};
}
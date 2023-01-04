#pragma once

namespace r2 {
	enum class Shader : u32 {
		SH_Basic,
		SH_Uber,
		SH_MSDF,

		SH_END
	};

	enum TexFilter  : u32 {
		TF_NEAREST,// use nearest neighbour, fast, better fror pixel art, generates square alike edges
		TF_LINEAR,// tiny bit slower, better for hd generate, generates blur
		
		TF_ANISO,// much slower, useful for 3d triangles mapping filtering ( same as linear but takes perspective into account )

		TF_INHERIT,//go with the flow

		TF_Count,
	};

	static inline Pasta::u32 toPastaTextureFilter(r2::TexFilter f) {
		Pasta::u32 fl = 0;
		switch (f) {
		case r2::TexFilter::TF_LINEAR:
			fl |= PASTA_TEXTURE_LINEAR;
			break;
		case r2::TexFilter::TF_ANISO:
			fl |= PASTA_TEXTURE_ANISOTROPIC;
			break;
		default:
			fl |= PASTA_TEXTURE_POINT;
			break;
		}
		return fl;
	};

	
}



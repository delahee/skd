


#pragma once

#include "rd/Agent.hpp"
#include "4-ecs/JsonReflect.h"

namespace r2 {
	namespace tools {
		enum class GradientMode : u32 {
			Linear,
			Box,
			Bands,
		};
		class GradientEditor : public rd::Agent {
		public:
			static GradientEditor* me;

		public:
			GradientMode	mode = GradientMode::Linear;
			std::string		name;

			r2::Node*		root = 0;
			r2::Graphics*	target = 0;

			r2::Bitmap*		checker = 0;

			eastl::vector<eastl::pair<float, r::Color>> cols;
			eastl::vector<eastl::tuple<float, r::Color, r::Color>> boxCols;

							GradientEditor(r2::Node* root);
			virtual			~GradientEditor();

			virtual void	update(double dt) override;
			bool			im();
			static void		toggle(r2::Node* root);

			void			save(const char * name);

			void			dumpCpp();

			void			serialize(Pasta::JReflect& functor,const char * name = 0);
		};
	}
}


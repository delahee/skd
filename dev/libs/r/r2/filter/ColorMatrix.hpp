#pragma once

#include "../Filter.hpp"
#include "../Lib.hpp"
#include "Layer.hpp"

namespace r2 {
	namespace filter {
		class ColorMatrix : public Layer {
			typedef Layer				Super;
		public:
			//use default own matrix;
										ColorMatrix();
										ColorMatrix(const Pasta::Matrix44& _mat);
			virtual						~ColorMatrix() {}

			ColorMatrixControl			ctrl;
			void						setMatrix(const Pasta::Matrix44& _mat);

			//in case you want to disconnect the control
			void						setMatrix(const Pasta::Matrix44* _mat);


			//float hue,float sat,float val
			//float	 hue = 0;sat = 1,val = 1, 
			// will affect ownMatrix only
			// hue [0,360]
			// sat [0,2]
			// val [0,2]
			void						setHSV(float hue = 0.0f, float sat = 1.0f, float val = 1.0f);
			virtual void				im() override;

			virtual void				invalidate() override;
			virtual void				serialize(Pasta::JReflect& jr, const char* name) override;
			virtual r2::Filter*			clone(r2::Filter* obj = 0) override;
		protected:
			const Pasta::Matrix44* 		mat;
		private:
			//Pasta::Matrix44 			ownMatrix;

			void						bmpOp(r2::Bitmap&bmp);
		};
	}
}

//cpp has a weir name because visual studio is stupid enough to not associate .obj with namespaces

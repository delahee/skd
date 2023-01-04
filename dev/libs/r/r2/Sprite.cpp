#include "stdafx.h"

#include "Tile.hpp"
#include "Node.hpp"
#include "Sprite.hpp"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"
#include "1-graphics/Texture.h"
#include "1-time/Profiler.h"

#include "../rs/ITweenable.hpp"

#include "r2/Im.hpp"


using namespace std;
using namespace r;
using namespace rs;
using namespace r2;
using namespace rd;

#define SUPER Node

Sprite::Sprite(Node * parent) : SUPER( parent ){
	
}

Sprite::~Sprite() {
	
}

void r2::Sprite::setColor(int col, float a) {
	color.r = ((col >> 16) & 0xff) / 255.0f;
	color.g = ((col >> 8) & 0xff) / 255.0f;
	color.b = ((col) & 0xff) / 255.0f;
	color.a = a;
}

void r2::Sprite::setColor(const r::Color& col) {
	color = col;
}

void r2::Sprite::setZOffset(float topOffset, float bottomOffset) {
	useSimpleZ = false;
	zTopOffset = topOffset;
	zBottomOffset = bottomOffset;
}

bool Sprite::shouldRenderThisPass(rs::GfxContext* ctx) {
	if (!visible) return false;

	if (ctx->currentPass == rs::Pass::Picking) {
		if (nodeFlags & (NF_UTILITY | NF_EDITOR_FROZEN | NF_EDITOR_PROTECT))
			return false; // do not pick frozen, protected or custom shaded node
		if (overrideShader)
			return false;
		return true;
	}

	if (ctx->currentPass == rs::Pass::ExtraPass0) {
		return useExtraPass;
	}

	bool earlyDepthCompatible = ctx->supportsEarlyDepth && !forceBasicPass;
	earlyDepthCompatible &= depthWrite;
	earlyDepthCompatible &= blendmode == Pasta::TT_CLIP_ALPHA || blendmode == Pasta::TT_OPAQUE;
	earlyDepthCompatible &= !filter;

	switch (ctx->currentPass) {
		case rs::Pass::EarlyDepth: // fallthrough
		case rs::Pass::DepthEqWrite:
			return earlyDepthCompatible;
		case rs::Pass::Basic:
			return !earlyDepthCompatible;
		default:
			PASTA_ASSERT_MSG(false, "invalid pass");
			return false;
	}
}

double r2::Sprite::getFinalAlpha() {
	return color.a * alpha;
}

void r2::Sprite::dispose() {
	SUPER::dispose();
	
	color = r::Color();
	alpha = 1.0f;

	depthRead = false;
	depthWrite = false;
	depthGreater = false;
	killAlpha = false;
	useSimpleZ = true;

	texFiltering = r2::TexFilter::TF_INHERIT;
	shader = r2::Shader::SH_Basic;
	overrideShader = nullptr;
	additionnalTexture = nullptr;

	zTopOffset = 0.0f;
	zBottomOffset = 0.0f;

	blendmode = r::TransparencyType::TT_ALPHA;
	shaderValues.dispose();
}

void r2::Sprite::reset(){
	r2::Node::reset();
	depthRead = false;
	depthWrite = false;
	depthGreater = false;
	killAlpha = false;
	useSimpleZ = true;

	texFiltering = r2::TexFilter::TF_INHERIT;

	zTopOffset = 0.0f;
	zBottomOffset = 0.0f;

	color = r::Color::White;//color alpha is not counting against hierarchical alpha mods
	blendmode = r::TransparencyType::TT_ALPHA;

	vcustom0 = 0.0, vcustom1 = 0.0, vcustom2 = 0.0, vcustom3 = 0.0,
	vcustom4 = 0.0, vcustom5 = 0.0, vcustom6 = 0.0, vcustom7 = 0.0;

	overrideShader = nullptr;
	additionnalTexture = nullptr;
	shader = r2::Shader::SH_Basic;
	shaderValues.dispose();
}

void r2::Sprite::draw(rs::GfxContext* ctx) {
	if (useExtraPass && ctx->currentPass == rs::Pass::ExtraPass0 && beforeExtraPass) beforeExtraPass(this);
	bool cont = drawPrepare(ctx);
	if (cont) {
		drawSubmitGeometry(ctx);
		drawCleanup(ctx);
	}
	if (useExtraPass && ctx->currentPass == rs::Pass::ExtraPass0 && afterExtraPass) afterExtraPass(this);
}

void Sprite::applyBlendmode(rs::GfxContext* ctx) {
	Pasta::Graphic * g = ctx->gfx;
	Pasta::GraphicContext * gfxCtx = Pasta::GraphicContext::GetCurrent();
	switch (ctx->currentPass) {
		case rs::Pass::EarlyDepth:
			gfxCtx->setBlendState(r2::GpuObjects::opaqueNoColor);
			return;
		case rs::Pass::Picking:
			g->setTransparencyType(TransparencyType::TT_OPAQUE);
			break;
		default: // basic or deptheq
			g->setTransparencyType(blendmode);
			break;
	}
	g->applyContextTransparency();
}

bool Sprite::drawPrepare(rs::GfxContext * ctx) {
	if (!shouldRenderThisPass(ctx)) return false;

	Tile * tile = getPrimaryTile();
	if (!tile) return false;

	Pasta::Texture * texture = tile->getTexture();
	if (!texture) return false;

	Pasta::Graphic * g = ctx->gfx;
	g->pushContext();

	applyDepth(ctx);

	g->setVertexDeclaration(Pasta::VD_POSITIONS | Pasta::VD_TEXCOORDS);

	r::Color col = color; // make a copy to factor final alpha in
	g->setColor(Pasta::Color(col.r, col.g, col.b, 1.0)); // crush uniform to 1 to avoid having uniform detected as available, alpha is not interpreted here so let it goooooo

	float finalAlpha = col.a * ctx->alpha.back();
	g->setAlpha(col.a * ctx->alpha.back());
	if (finalAlpha <= 0.0f) { // early skip this
		g->popContext();
		return false;
	}

	bindTexture(ctx, texture);//will need blend?
	bindTexture(ctx, additionnalTexture, 1);
	applyBlendmode(ctx); // need to be done after the texture binding in case of premultiplied alpha

	//set matrix
	if(!rd::Bits::hasOne(nodeFlags, NF_MANUAL_MATRIX))
		syncMatrix(); //ensure matrix is correct

	if (trsDirty) syncMatrix();

	ctx->loadModelMatrix(mat);

	bindShader(ctx);

	return true;
}

void Sprite::drawSubmitGeometry(rs::GfxContext* ctx) {
	Pasta::Graphic * g = ctx->gfx;
	Tile * tile = getRenderTile();
	const int stride = 3 + 2;
	float vertexData[4 * stride];//pos uv
	Pasta::u32 texCoordOffset = 3;//after pos

	//could cache the full buffer if tile and color hasn't changed
	// positions
	{
		float *pos0 = &vertexData[0 * stride];
		float *pos1 = &vertexData[1 * stride];
		float *pos2 = &vertexData[2 * stride];
		float *pos3 = &vertexData[3 * stride];

		float v0x = tile->dx;
		float v1x = tile->dx + tile->width;

		float v0y = tile->dy;
		float v1y = tile->dy + tile->height;

		pos0[0] = v0x;//could remove gpumat and put dx
		pos0[1] = v0y;
		pos0[2] = useSimpleZ ? 0.0 : zTopOffset;

		pos1[0] = v1x;//could remove gpumat and put dx + width
		pos1[1] = v0y;
		pos1[2] = useSimpleZ ? 0.0 : zTopOffset;

		pos2[0] = v0x;
		pos2[1] = v1y;
		pos2[2] = useSimpleZ ? 0.0 : zBottomOffset;

		pos3[0] = v1x;
		pos3[1] = v1y;
		pos3[2] = useSimpleZ ? 0.0 : zBottomOffset;
	}

	// texcoords
	{
		float *tex0 = &vertexData[0 * stride + texCoordOffset];
		float *tex1 = &vertexData[1 * stride + texCoordOffset];
		float *tex2 = &vertexData[2 * stride + texCoordOffset];
		float *tex3 = &vertexData[3 * stride + texCoordOffset];

		const float u1 = tile->u1;
		const float v1 = tile->v1;
		const float u2 = tile->u2;
		const float v2 = tile->v2;

		tex0[0] = u1; tex0[1] = v1;
		tex1[0] = u2; tex1[1] = v1;
		tex2[0] = u1; tex2[1] = v2;
		tex3[0] = u2; tex3[1] = v2;
	}

	g->drawPrimitives(Pasta::PT_TRIANGLE_STRIP, 2, vertexData);
}

void Sprite::drawCleanup(rs::GfxContext* ctx) {
	Pasta::Graphic * g = ctx->gfx;
	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);
	
	g->popContext();
}

Tile * r2::Sprite::getPrimaryTile(){
	return nullptr;
}

Tile * Sprite::getRenderTile(){
	return getPrimaryTile();
}

void r2::Sprite::applyDepth(rs::GfxContext* ctx) {
	Pasta::Graphic * g = ctx->gfx;

	if (depthGreater) {
		g->setDepthState(GpuObjects::depthReadOnlyGreater);
		return;
	}

	if (	ctx->currentPass == rs::Pass::Basic
	||		ctx->currentPass == rs::Pass::EarlyDepth
	||		ctx->currentPass == rs::Pass::Picking) {
		Pasta::DepthStateID did = GpuObjects::depthNoSupport;
		if (depthRead && depthWrite)
			did = GpuObjects::depthReadWrite;
		else if (depthRead)
			did = GpuObjects::depthReadOnly;
		else if (depthWrite)
			did = GpuObjects::depthWriteOnly;
		g->setDepthState(did);
	}
	else if (ctx->currentPass == rs::Pass::DepthEqWrite) {
		g->setDepthState(GpuObjects::depthReadEq);
	}
}


Node * r2::Sprite::clone(Node * n){
	if (!n) n = new Sprite();
	Sprite * s = dynamic_cast<Sprite*>(n);
	SUPER::clone(n);

	s->forceBasicPass = forceBasicPass;
	s->useExtraPass = useExtraPass;
	s->killAlpha = killAlpha;
	s->useSimpleZ = useSimpleZ;
	s->color = color;
	s->alpha = alpha;
	s->blendmode = blendmode;
	s->depthRead = depthRead;
	s->depthWrite = depthWrite;
	s->depthGreater = depthGreater;
	s->zTopOffset = zTopOffset;
	s->zBottomOffset = zBottomOffset;
	s->useSimpleZ = useSimpleZ;
	s->texFiltering = texFiltering;
	s->zTopOffset = zTopOffset;
	s->zBottomOffset = zBottomOffset;
	s->shader = shader;
	s->shaderFlags = shaderFlags;
	s->shaderValues = shaderValues;
	return n;
}

double Sprite::getValue(TVar valType){
	switch (valType) {
		case VX:
		case VY:
		case VScaleX:
		case VScaleY:
		case VRotation:
		case VScale:
		case VWidth:
		case VHeight:
						return SUPER::getValue(valType);

		case VR:		return color.r;
		case VG:		return color.g;
		case VB:		return color.b;
		case VA:		return color.a;
		case VAlpha:		return alpha;

		case VVisibility:	return (visible) ? 1.0 : 0.0;

		case VCustom0:	return vcustom0;
		case VCustom1:	return vcustom1;
		case VCustom2:	return vcustom2;
		case VCustom3:	return vcustom3;
		case VCustom4:	return vcustom4;
		case VCustom5:	return vcustom5;
		case VCustom6:	return vcustom6;
		case VCustom7:	return vcustom7;
	}
	return 0.0;
}

double Sprite::setValue(TVar valType, double val){
	switch (valType) {
		case VX:
		case VY:
		case VScaleX:
		case VScaleY:
		case VRotation:
		case VScale:
		case VWidth:
		case VHeight:
						return SUPER::setValue(valType,val);

		case VR:		return color.r	= val;
		case VG:		return color.g	= val;
		case VB:		return color.b	= val;
		case VA:		return color.a	= val;

		case VAlpha:		return alpha	= val;
		case VVisibility:	return visible = (val >= 0.999) ? true : false;

		case VCustom0:	return vcustom0	= val;
		case VCustom1:	return vcustom1	= val;
		case VCustom2:	return vcustom2 = val;
		case VCustom3:	return vcustom3 = val;
		case VCustom4:	return vcustom4 = val;
		case VCustom5:	return vcustom5 = val;
		case VCustom6:	return vcustom6 = val;
		case VCustom7:	return vcustom7 = val;
	};
	return val;
}

Texture* Sprite::getTexture(rs::GfxContext* ctx, int slot) {
	//could be 
	//return _g->getGpuContext()->getTexture(Pasta::ShaderStage::Fragment, slot);
	return getRenderTile() ? getRenderTile()->getTexture() : nullptr; 
}

bool _currentVertexDeclHasColors(Pasta::Graphic * g)
{
	Pasta::VertexDeclID declID = g->getVertexDeclaration();
	const Pasta::VertexElement* vertexDecl = Pasta::GraphicContext::GetVertexDecl(declID);
	int i = 0;
	while (vertexDecl[i] != Pasta::PASTA_VERTEX_ELEMENT_LAST)
	{
		if (vertexDecl[i].semantic == Pasta::VS_COLORS)
			return true;

		++i;
	}
	return false;
}

void r2::Sprite::setShaderParam(const std::string & name, int val) {
	rd::Anon * v = new rd::Anon();
	v->name = name;
	v->mkInt(val);
	addShaderParam(v);
}

void r2::Sprite::setShaderParam(const std::string & name, float val) {
	rd::Anon * v = new rd::Anon();
	v->name = name;
	v->mkFloat(val);
	addShaderParam(v);
}

void r2::Sprite::updateShaderParam(const std::string & name, float val) {
	if (shader != Shader::SH_Uber) mkUber(); // you don't update for dummies, ensure it's ok
	Anon * prm = getShaderParam(name);
	if (prm)
		prm->setFloat(val);
	else {
		rd::Anon * v = new rd::Anon();
		v->name = name;
		v->mkFloat(val);
		addShaderParam(v);
	}
}

bool r2::Sprite::hasShaderParam(const std::string & name) {
	return shaderValues.has(name.c_str());
}

bool r2::Sprite::hasShaderParam(const char * name) {
	return shaderValues.has(name);
}

void r2::Sprite::removeShaderParam(const char* name) {
	shaderValues.destroy(name);
}

void r2::Sprite::removeShaderParam(const std::string & name) {
	shaderValues.destroy(name.c_str());
}

void r2::Sprite::updateShaderParam(const char* name, float val){
	if (shader != Shader::SH_Uber) mkUber(); // you don't update for dummies, ensure it's ok
	Anon* prm = getShaderParam(name);
	if (prm)
		prm->setFloat(val);
	else {
		rd::Anon* v = new rd::Anon();
		v->name = name;
		v->mkFloat(val);
		addShaderParam(v);
	}
}

void r2::Sprite::updateShaderParam(const char* name, const float* buf, int nbFloats) {
	if (shader != Shader::SH_Uber) mkUber(); // you don't update for dummies, ensure it's ok
	Anon* prm = getShaderParam(name);
	if (prm)
		prm->setFloatBuffer(buf, nbFloats);
	else {
		rd::Anon* v = new rd::Anon();
		v->name = name;
		v->mkFloatBuffer(buf, nbFloats);
		addShaderParam(v);
	}
}

void r2::Sprite::updateShaderParam(const std::string & name, const float * buf, int nbFloats){
	updateShaderParam(name.c_str(), buf, nbFloats);
}

void r2::Sprite::setShaderParam(const char * name, const float* buf, int nbFloats) {
	if (shader != Shader::SH_Uber)
		mkUber();
	Anon* prm = getShaderParam(name);
	if (prm) removeShaderParam(name);
	rd::Anon* v = new rd::Anon();
	v->name = name;
	v->mkFloatBuffer(buf, nbFloats);
	addShaderParam(v);
}

void r2::Sprite::setShaderParam(const std::string & name, const float * buf, int nbFloats) {
	if (shader != Shader::SH_Uber)
		mkUber();

	Anon * prm = getShaderParam(name);
	if (prm) removeShaderParam(name);
	rd::Anon * v = new rd::Anon();
	v->name = name;
	v->mkFloatBuffer(buf, nbFloats);
	addShaderParam(v);
}

void r2::Sprite::addShaderParam(rd::Anon * val) {
	if (shader != Shader::SH_Uber)
		mkUber();

	shaderValues.add(val);
}

void r2::Sprite::updateShaderParam(const char * name, rd::Anon* val) {
	if (shader != Shader::SH_Uber)
		mkUber();
	Anon* prm = getShaderParam(name);
	Anon& _prm = *prm;
	if (prm)
		_prm = *val;
	else
		addShaderParam(val);
}

void r2::Sprite::updateShaderParam(const std::string& name, rd::Anon* val) {
	if (shader != Shader::SH_Uber)
		mkUber();
	Anon* prm = getShaderParam(name);
	Anon& _prm = *prm;
	if (prm)
		_prm = *val;
	else
		addShaderParam(val);
}

//don't release everything as it can serve
void r2::Sprite::mkClassic() {
	shader = Shader::SH_Basic;
}

void r2::Sprite::mkUber(){
	if (shader == Shader::SH_Uber) return;

	shader = Shader::SH_Uber;
	// maybe cleanup shaderValues?
}

rd::Anon * r2::Sprite::getShaderParam(const char * str) {
	return shaderValues.get(str);
}

rd::Anon * r2::Sprite::getShaderParam(const std::string & str){
	return shaderValues.get(str.c_str());
}

Pasta::u32 Sprite::computeShaderFlags(rs::GfxContext* ctx) {
	Pasta::Graphic* g = ctx->gfx;
	Pasta::u32 flags = 0;
	Pasta::Color gCol = g->getColor();
	float gAlpha = g->getAlpha();

	Texture* tex = getTexture(ctx);
	if (tex)
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;

	if (_currentVertexDeclHasColors(g))
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_VERTEX_COLOR;

	if (tex && tex->isPremultipliedAlpha() && g->isBlendingEnabled())
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_PREMUL_ALPHA;

	if (gAlpha != 1.0 || gCol.r != 1 || gCol.g != 1 || gCol.b != 1)
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_UNIFORM_COLOR;

	if (killAlpha)
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;

	if (!ctx->supportsEarlyDepth || ctx->currentPass == rs::Pass::EarlyDepth) {
		if (blendmode == Pasta::TT_CLIP_ALPHA)
			flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;
	}
	return flags;
}

Pasta::ShaderProgram * Sprite::pickShader(rs::GfxContext* ctx) {
	Pasta::u32 flags = computeShaderFlags(ctx);
	r2::Shader myShader = overrideShader ? Shader::SH_END : this->shader;

	if (!overrideEarlyDepthShader && ctx->currentPass == Pass::EarlyDepth) {
		if (blendmode != Pasta::TT_CLIP_ALPHA && blendmode != Pasta::TT_OPAQUE) throw "unsupported op, cannot pick shader for non clipped alpha non opaque";
		myShader = Shader::SH_Basic;
		flags &= ~Pasta::Graphic::BasicShaderFlags::BSF_UNIFORM_COLOR;
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;
	}

	if (ctx->currentPass == Pass::Picking) {
		myShader = Shader::SH_Basic;
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_UNIFORM_COLOR;
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;
		flags &= ~Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE_ALPHA_ONLY;
		flags &= ~Pasta::Graphic::BasicShaderFlags::BSF_PREMUL_ALPHA;
	}

	Pasta::ShaderProgram* shader = nullptr;
	if (myShader == Shader::SH_END) {
		if (ctx->currentPass == Pass::EarlyDepth) 
			shader = overrideEarlyDepthShader;
		else
			shader = overrideShader;
	}
	else shader = r2::Lib::getShader(myShader, (myShader == Shader::SH_Basic) ? flags : (flags | shaderFlags));
	if (!shader) printf("no basic shader with such flags %d", flags);

	return shader;
}

void r2::Sprite::bindShader(rs::GfxContext* ctx) {
	Pasta::Graphic* g = ctx->gfx;
	Pasta::ShaderProgram* sh = pickShader(ctx);
	if (!sh) return;

	// todo cleanup all this mess:
	Pasta::Color gCol = g->getColor();
	float gAlpha = g->getAlpha();
	Texture* tex = getTexture(ctx);

	Pasta::ShaderParam* param = sh->getParam("uColor");
	if (param) {
		Pasta::Color c = gCol;
		c.a = gAlpha;

		if (ctx->currentPass == Pass::Picking) {
			c.r = ((uid) & 0x00FFFFFF);
			c.g = ((uid >> 24) & 0x00FFFFFF);
			c.b = ((uid >> 48) & 0x00FFFFFF);
			c.a = 1.0;
		} else if (tex && tex->isPremultipliedAlpha() && g->isBlendingEnabled()) {
			c.r *= c.a;
			c.g *= c.a;
			c.b *= c.a;
		}

		param->setValue(c.ptr());
	}

	if (shader == Shader::SH_Basic && !overrideShader) {
		g->setShader(sh);
		return;
	}

	{
		Pasta::ShaderParam* p = sh->getParam("uModel");
		if (p) p->setValue(mat.transpose(), Pasta::ArrayLayout::AL_COLUMN_MAJOR);
	}

	r2::Lib::applyShaderValues(g, sh, shaderValues);

	if (shader == Shader::SH_Uber && !overrideShader) {
		bool isBlur = shaderFlags & UberShaderFlags::USF_Gaussian_Blur;

		//setup resolutions
		if (isBlur) {
			Anon* val = getShaderParam("uKernel");
			Pasta::ShaderParam* p = sh->getParam("uNbSamples");
			if (p && val) p->setValue(val->getSize());
		}
	}

	g->setShader(sh);
}

void r2::Sprite::bindTexture(rs::GfxContext* ctx, Pasta::Texture* tex, int slot) {
	Pasta::Graphic* g = ctx->gfx;

	if (!tex) {
		g->setTexture(Pasta::ShaderStage::Fragment, slot, nullptr);
		return;
	}
	
	if (texFiltering == r2::TexFilter::TF_INHERIT) {
		g->setTexture(Pasta::ShaderStage::Fragment, slot, tex);
		return;
	}

	Pasta::TextureFlags fl = tex->getFlags() & (PASTA_TEXTURE_MIN__MASK__ | PASTA_TEXTURE_MAG__MASK__ | PASTA_TEXTURE_MIP__MASK__);
	Pasta::TextureFlags ofl = fl;

	switch (texFiltering)
	{
	case r2::TexFilter::TF_LINEAR:
		fl |= PASTA_TEXTURE_LINEAR;
		tex->setFilterMode(PASTA_TEXTURE_LINEAR);
		break;
	case r2::TexFilter::TF_ANISO:
		fl |= PASTA_TEXTURE_ANISOTROPIC;
		tex->setFilterMode(PASTA_TEXTURE_ANISOTROPIC);
		break;
	default://defaults to nearest
		fl |= PASTA_TEXTURE_POINT;
		tex->setFilterMode(PASTA_TEXTURE_POINT);
		break;
	}

	g->setTexture(Pasta::ShaderStage::Fragment, slot, tex);
	tex->setFilterMode(ofl);
}

void r2::Sprite::setShaderTimeOffset(float f){
	shaderValues.set("uTimeOffset", f);
}

void Sprite::drawFilteringResult(rs::GfxContext* ctx) {
	r2::Tile * t = filter->getResult();
	if (!t) return;

	//g->ensureConsistency();

	r2::ImSprite spr(this);
	spr.depthRead = false;
	spr.tile = t;
	spr.color.a *= ctx->alpha.back();
	Im::draw(ctx, spr);
}

void Sprite::blendAdd() {
	blendmode = Pasta::TT_ADD;
}

#undef SUPER


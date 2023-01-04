#include "Video.hpp"
#include "stdafx.h"
#include "Sprite.hpp"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"

using namespace r2;

#define SUPER Sprite

r2::Video::Video(Node * parent) : SUPER(parent) {
	tile = rd::Pools::tiles.alloc();
	tile->enableTextureOwnership(true);
	tile->set(nullptr, 0, 0, 1280, 720);

	decoder = h264bsdAlloc();
	h264bsdInit(decoder, 0);
}

Video* r2::Video::fromVideoFile(const std::string& path, r2::Node* parent) {
	auto vid = new Video(parent);

	std::string npath = Pasta::FileMgr::getSingleton()->convertResourcePath(path);
	vid->fileStart = vid->bytes = (u8*)Pasta::FileMgr::getSingleton()->load(npath, &vid->len);

	return vid;
}

void r2::Video::play() {
	if (isPlaying) return;
	isPlaying = true;
	frameReady = false;
	frame = 0;
	lastFrameDecoded = 0;
	playStartTime = rs::Timer::now;
}

void r2::Video::update(double dt) {
	SUPER::update(dt);

	if (!isPlaying)
		return;

	if (frameReady) {
		auto ctx = Pasta::GraphicContext::GetCurrent();
		ctx->DestroyTexture(tile->getTexture());
		auto texture = ctx->CreateTexture(texData);
		tile->setTexture(texture);
		frameReady = false;
	}

	if (getTime() >= currentVideoTime) {
		decodeFrame(frame);

		u32 frameInc = 1;
		while (getTime() >= currentVideoTime) {
			frameInc++;
			currentVideoTime += 1.0 / 60.0;
		}
		frame += frameInc;

		frameReady = true; // delay decode/upload for more reliable FPS
	}
	if (len == 0) { // video ended
		isPlaying = false;
		onEnd.trigger();
	}
}

bool r2::Video::decodeFrame(u32 wantedFrame) {
	u32 status = H264BSD_RDY;
	u32 bytesRead = 0;
	while (len > 0) {
		status = h264bsdDecode(decoder, bytes, len, 0, &bytesRead);

		if (status == H264BSD_PIC_RDY) {
			++lastFrameDecoded;
			if (lastFrameDecoded >= wantedFrame) {
				if (tex == nullptr) {
					width = h264bsdPicWidth(decoder) * 16;
					height = h264bsdPicHeight(decoder) * 16;
					tex = (u32*)malloc(1280 * 720 * sizeof(u32));
					memset(tex, 0x00, 1280 * 720 * sizeof(u32));
					auto ctx = Pasta::GraphicContext::GetCurrent();
					auto loader = Pasta::TextureLoader::getSingleton();
					texData = loader->loadRaw(tex, 1280, 720);
				}

				u32 picId, isIdrPic, numErrMbs;
				u32* picData = h264bsdNextOutputPictureRGBA(decoder, &picId, &isIdrPic, &numErrMbs);

				for (unsigned int i = 0; i < width * height; i++) {
					tex[i] = picData[i];
				}

				bytes += bytesRead;
				len -= bytesRead;
				status = H264BSD_RDY;
				currentVideoTime = lastFrameDecoded * ((double)1 / (double)60);
				return true;
			}
		}

		if (status == H264BSD_ERROR) {
			printf("General Error with %i bytes left\n", len);
		}

		if (status == H264BSD_PARAM_SET_ERROR) {
			printf("Param Set Error with %i bytes left\n", len);
		}

		if (status == H264BSD_MEMALLOC_ERROR) {
			printf("Malloc Error with %i bytes left\n", len);
		}

		bytes += bytesRead;
		len -= bytesRead;
		status = H264BSD_RDY;
	}
	return true;
}

Node * r2::Video::clone(Node * n) {
	if (!n) n = new Video();
	
	SUPER::clone(n);

	Video* v = dynamic_cast<Video*>(n);
	if (v) {// we may sometime want to transfer only parent!
		v->tile = tile->clone();
	}
	return n;
}

Video::~Video() {
	dispose();
}

void r2::Video::releaseTile(){
	if (tile) {
		if(tile->mustDestroyTex())
			tile->disposeTexture();

		if ( tile->isPooled() ) {
			rd::Pools::tiles.free(tile);
			tile = nullptr;
		}
		else {
			delete(tile);
			tile = nullptr;
		}
	}

	//no own no touch get rid of this
	tile = nullptr;
}

void Video::dispose(){
	SUPER::dispose();

	releaseTile();
	auto loader = Pasta::TextureLoader::getSingleton();

	if (tex != nullptr)
		free(tex);
	if (texData != nullptr)
		loader->release(texData);
	if (bytes != nullptr)
		Pasta::FileMgr::getSingleton()->release(fileStart);
	if (decoder != nullptr) {
		h264bsdShutdown(decoder);
		h264bsdFree(decoder);
	}
	tex = nullptr;
	texData = nullptr;
	bytes = nullptr;
	decoder = nullptr;
}

Tile* r2::Video::getPrimaryTile() {
	return tile;
}

void Video::setWidth(float _w){
	if (!tile) return;
	scaleX = _w / tile->width;
}

void Video::setHeight(float _h){
	if (!tile) return;
	scaleY = _h / tile->height;
}

Bounds r2::Video::getMyLocalBounds(){
	Bounds b;
	b.empty();
	syncAllMatrix();

	if (!tile) return b;

	Pasta::Matrix44 local = getLocalMatrix();
	
	Pasta::Vector3 topLeft		= local * Pasta::Vector3(tile->dx,					tile->dy, 0.0);
	Pasta::Vector3 topRight		= local * Pasta::Vector3(tile->dx + tile->width,	tile->dy, 0.0);
	Pasta::Vector3 bottomLeft	= local * Pasta::Vector3(tile->dx,					tile->dy + tile->height, 0.0);
	Pasta::Vector3 bottomRight	= local * Pasta::Vector3(tile->dx + tile->width,	tile->dy + tile->height, 0.0);

	b.addPoint(topLeft.x, topLeft.y);
	b.addPoint(topRight.x, topRight.y);
	b.addPoint(bottomLeft.x, bottomLeft.y);
	b.addPoint(bottomRight.x, bottomRight.y);

	return b;
}

void r2::Video::setCenterRatio(double px, double py){
	if (tile)
		tile->setCenterRatio(px, py);
	else
		int p = 0;
}

std::string r2::Video::toString()
{
	return std::string("vid ") + (!tile ? "" : tile->toString());
}


void r2::Video::im() {
	r2::Sprite::im();
	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && ImGui::CollapsingHeader(ICON_MD_VIDEOCAM " Video")) {
		ImGui::Indent(); ImGui::PushItemWidth(124);
		
		if (ImGui::Button("Play")) {
			play();
		}

		ImGui::PopItemWidth();
		ImGui::Unindent();
	}
}
#undef SUPER
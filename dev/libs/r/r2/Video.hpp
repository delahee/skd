#pragma once
#include "Sprite.hpp"
#include "Tile.hpp"
#include "1-graphics/Graphic.h"
extern "C"
{
#include "h264bsd_decoder.h"
}

namespace r2 {
	class Video : public r2::Sprite {
		storage_t*		decoder = nullptr;
		u8*				fileStart;
		u8*				bytes;
		u32				len;
		u32				lastFrameDecoded;

		double			playStartTime = 0;
		double			currentVideoTime = 0;
		u32				frame = 0;
		bool			frameReady = false;

		Pasta::TextureData* texData = nullptr;
		u32*				tex = nullptr;
		u32					width;
		u32					height;
		Tile*				tile = nullptr;

		bool			isPlaying = false;
	public:
		rd::Sig			onEnd;
	public:
						Video( Node * parent = nullptr);
		virtual			~Video();

		static Video*	fromVideoFile(const std::string& path, r2::Node* parent);

		void			play();
		double getTime() { return isPlaying ? rs::Timer::now - playStartTime : 0; }
		virtual void	update(double dt) override;
		bool			decodeFrame(u32 wantedFrame);

		void			releaseTile();
		virtual void	dispose();

		virtual Node *	clone(Node * n) override;

		virtual Tile*	getPrimaryTile() override;

		virtual void	setWidth(float width) override;
		virtual void	setHeight(float height) override;

		virtual Bounds	getMyLocalBounds() override;

		virtual void	setCenterRatio( double px = 0.5, double py = 0.5 );

		std::string		toString();

		virtual	NodeType getType() const override { return NodeType::NT_VIDEO; };
		virtual void	im() override;
	};
}

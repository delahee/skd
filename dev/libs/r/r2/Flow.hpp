#pragma once

#include "r2/Node.hpp"

using std::optional;

namespace r2 {
	
	//port tentative for 
	// https://github.com/HeapsIO/heaps/blob/master/h2d/Flow.hx
	//

	enum class FlowAlign : u32 {
		Top,
		Left,
		Right,
		Middle,
		Bottom,
	};

	enum class FlowOverflow : u32 {
		Expand,
		Limit,
		Hidden,
		//Scroll,
	};

	enum class FlowLayout : u32 {
		Horizontal,
		Vertical,
		Stack,
	};

	struct FlowItemProperty {
		int					paddingLeft		= 0;
		int					paddingTop		= 0;
		int					paddingRight	= 0;
		int					paddingBottom	= 0;

		bool				isAbsolute		= false;

		optional<FlowAlign>	horizontalAlign = std::nullopt;
		optional<FlowAlign>	verticalAlign = std::nullopt;

		int					offsetX=0;
		int					offsetY=0;

		optional<int>		minWidth = std::nullopt;
		optional<int>		minHeight = std::nullopt;

		int					calculatedWidth = 0;
		int					calculatedHeight = 0;

		bool				isBreak = false;
		bool				lineBreak = false;
		bool				constraint = true;

		void setAbsolute(bool onOff) {
			if (onOff)
				isBreak = false;
			isAbsolute = onOff;
		};

		void im();
	};

	class ConstraintManager {
		public:
			std::unordered_map<r2::Node*, Vector2i> objs;
	};

	/**
	 * Warning wip, don't try this at home
	 */
	class Flow : public r2::Node {
	public:
		r2::Bounds					tmp;
		bool						needReflow = true;

		std::optional<FlowAlign>	horizontalAlign = std::nullopt;
		std::optional<FlowAlign>	verticalAlign = std::nullopt;

		std::optional<int>			lineHeight = std::nullopt;
		std::optional<int>			colWidth = std::nullopt;

		bool						debug = true;
		FlowOverflow				overflow = FlowOverflow::Expand;

		bool						posChanged = true;
		int							paddingHorizontal = 0;
		int							paddingVertical = 0;

		int							paddingLeft = 0;
		int							paddingTop = 0;
		int							paddingRight = 0;
		int							paddingBottom = 0;

		int							horizontalSpacing = 0;
		int							verticalSpacing = 0;

		int							borderWidth = 0;
		int							borderHeight = 0;

		int							borderLeft = 0;
		int							borderRight = 0;
		int							borderTop = 0;
		int							borderBottom = 0;

		int							innerWidth = 0;
		int							innerHeight = 0;

		int							outerWidth = 0;
		int							outerHeight = 0;

		FlowLayout					layout = FlowLayout::Horizontal;
		bool						isInline = true;
		bool						multiline = false;
		bool						reverse = false;
		bool						fillWidth = false;
		bool						fillHeight = false;

		eastl::vector<FlowItemProperty> properties;
		float						calculatedWidth = 0.;
		float						calculatedHeight = 0.;
		float						contentWidth = 0.;
		float						contentHeight = 0.;
		float						constraintWidth = -1;
		float						constraintHeight = -1;
		float						realMaxWidth = -1;
		float						realMaxHeight = -1;
		int							realMinWidth = -1;
		int							realMinHeight = -1;
		bool						isConstraint = false;
		bool manualReflow = false;

		optional<int>				getMinWidth() { return minWidth; };
		optional<int>				getMaxWidth() { return maxWidth; };
		optional<int>				getMinHeight() { return minHeight; };
		optional<int>				getMaxHeight() { return maxHeight; };

		void setMinWidth(optional<int> w) {
			needReflow = true;
			minWidth = w;
			updateConstraint();
		}

		void setMinHeight(optional<int> h) {
			needReflow = true;
			minHeight = h;
			updateConstraint();
		}

		void setMaxWidth(optional<int> w) {
			maxWidth = w;
			updateConstraint();
		}

		void setMaxHeight(optional<int> h) {
			maxHeight = h;
			updateConstraint();
		}

		// do not port background, interactive and scrollBar yet ( it should be broken down in extended version anyway )

		void constraintSize(float width, float height) {
			constraintWidth = width;
			constraintHeight = height;
			isConstraint = true;
			updateConstraint();
		};

		inline void					vertical() { layout = FlowLayout::Vertical; needReflow = true; };
		inline void					horizontal() { layout = FlowLayout::Horizontal; needReflow = true; };
		void						updateConstraint();

									Flow(r2::Node* parent);
		virtual						~Flow();

		FlowItemProperty&			getProperties(r2::Node* elem);
		const FlowItemProperty&		getProperties(r2::Node* e) const;

		void						reflow();
		virtual void				im() override;
		virtual void				onAddChild(Node*) override;
		virtual void				onRemoveChild(Node*) override;
		virtual void				update(double dt) override;

		rd::Sig						onReflow;
		rd::Sig						onAfterReflow;

	protected:
		optional<int>				minWidth = std::nullopt;
		optional<int>				maxWidth = std::nullopt;

		optional<int>				minHeight = std::nullopt;
		optional<int>				maxHeight = std::nullopt;
		ConstraintManager			constraints;

		void						addConstraint(r2::Node* n, int x, int y);
	};
}
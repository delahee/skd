#include "stdafx.h"
#include "Flow.hpp"
#include "Scene.hpp"

#define SUPER r2::Node

using namespace std;
using namespace r2;

static auto _id = []() {};

//see https://github.com/HeapsIO/heaps/blob/master/h2d/Flow.hx
Flow::Flow(r2::Node* parent) : SUPER(parent){
	onReflow = _id;
	afterReflow = _id;
	name = "Flow#" + to_string(uid);
}

FlowItemProperty& Flow::getProperties(r2::Node* e) {
	needReflow = true;
	int idx = getChildIndex(e);
	if (idx == -1)
		throw "failure to retrieve element, it's probably not in the children";
	return properties[idx];
}

const FlowItemProperty& Flow::getProperties(r2::Node* e) const {
	return properties[getChildIndex(e)];
}

void Flow::update(double dt) {
	if (!isConstraint && (fillWidth || fillHeight)) {
		auto scene = getScene();
		auto cw = fillWidth ? scene->width() : -1;
		auto ch = fillHeight ? scene->height() : -1;
		if (cw != constraintWidth || ch != constraintHeight) needReflow = true;
	}
	if (needReflow && !manualReflow)
		reflow();
	SUPER::update(dt);
}

void r2::Flow::reflow() {
	syncMatrix();

	onReflow();

	if (!isConstraint && (fillWidth || fillHeight)) {
		auto scene = getScene();
		auto cw = (fillWidth && scene) ? scene->width() : -1;
		auto ch = (fillHeight && scene) ? scene->height() : -1;
		if (cw != constraintWidth || ch != constraintHeight) {
			constraintSize(cw, ch);
			isConstraint = false;
		}
	}

	auto borderTop = 0;
	auto borderBottom = 0;
	auto borderLeft = 0;
	auto borderRight = 0;

	auto getSize = [this](r2::Node* c) -> r2::Bounds {
		r2::Bounds b = c->getBounds(this);
		if (b.isEmpty()) {
			b.zero();
		}
		else
			b.offset(-c->x, -c->y);
		return b;
	};

	auto isConstraintWidth = realMaxWidth >= 0;
	auto isConstraintHeight = realMaxHeight >= 0;
	// outer size
	auto maxTotWidth = realMaxWidth < 0 ? 100000000 : floor(realMaxWidth);
	auto maxTotHeight = realMaxHeight < 0 ? 100000000 : floor(realMaxHeight);
	// inner size
	auto maxInWidth = maxTotWidth - (paddingLeft + paddingRight + (borderLeft + borderRight));
	auto maxInHeight = maxTotHeight - (paddingTop + paddingBottom + (borderTop + borderBottom));
	auto childAt = [this](int i) {
		return children[reverse ? children.size() - i - 1 : i];
	};
	auto propAt = [this](int i) -> FlowItemProperty&{
		return properties[reverse ? children.size() - i - 1 : i];
	};

	float cw = 0.0f, ch = 0.0f;
	switch (layout) {
	case FlowLayout::Horizontal: {
		FlowAlign halign = (!horizontalAlign) ? FlowAlign::Left : *horizontalAlign;
		FlowAlign valign = (!verticalAlign) ? FlowAlign::Bottom : *verticalAlign;

		auto startX = paddingLeft + borderLeft;
		auto x = startX;
		auto y = paddingTop + borderTop;
		cw = x;
		auto maxLineHeight = 0;
		auto minLineHeight = (lineHeight) ? *lineHeight : (realMinHeight >= 0 && !multiline) ? (realMinHeight - (paddingTop + paddingBottom + borderTop + borderBottom)) : 0;
		auto lastIndex = 0;

		auto alignLine = [&](int maxIndex) {
			if (maxLineHeight < minLineHeight)
				maxLineHeight = minLineHeight;
			else if (overflow != FlowOverflow::Expand && minLineHeight != 0)
				maxLineHeight = minLineHeight;
			for (int i = lastIndex; i < maxIndex; ++i) {
				auto& p = propAt(i);
				if (p.isAbsolute && !p.verticalAlign ) continue;
				auto c = childAt(i);
				if (!c->visible) continue;
				auto a = (p.verticalAlign) ? p.verticalAlign : valign;
				c->y = y + p.offsetY + p.paddingTop;

				if(a)
				switch (*a) {
					case  FlowAlign::Bottom:
						c->y += maxLineHeight - int(p.calculatedHeight);
						break;

					case  FlowAlign::Middle:
						c->y += int((maxLineHeight - p.calculatedHeight) * 0.5);
						break;

					default:
						break;
				}
			}
			lastIndex = maxIndex;
		};

		auto remSize = [&](int from) {
			auto size = 0;
			for (int j = from; j < children.size(); ++j) {
				auto& p = propAt(j);
				if (p.isAbsolute || !childAt(j)->visible) continue;
				if (p.isBreak) break;
				size += horizontalSpacing + p.calculatedWidth;
			}
			return size;
		};

		for (int i = 0; i < children.size();++i) {
			auto& p = propAt(i);
			auto isAbs = p.isAbsolute;
			if (isAbs && !p.horizontalAlign && !p.verticalAlign ) continue;//if( isAbs && p.horizontalAlign == null && p.verticalAlign == null ) continue;
			auto c = childAt(i);
			if (!c->visible) continue;

			auto pw = p.paddingLeft + p.paddingRight;
			auto ph = p.paddingTop + p.paddingBottom;
			if (!isAbs) {
				if (isConstraintWidth && p.constraint) {
					//todo add a constraint manager to manage this rather than pollute global object space 
					// todo activate that when we have a text max width
					//addConstraint( c, (maxInWidth - pw) / abs(c->scaleX),
					//				(maxInHeight - ph) / abs(c->scaleX) 
					//);
				}
			}

			auto b = getSize(c);
			auto br = false;
			p.calculatedWidth = ceil(b.xMax) + pw;
			p.calculatedHeight = ceil(b.yMax) + ph;
			if ((p.minWidth==nullopt)  && (p.calculatedWidth < p.minWidth)) 
				p.calculatedWidth = *p.minWidth;
			if ( (p.minHeight==nullopt)  && (p.calculatedHeight < p.minHeight)) 
				p.calculatedHeight = *p.minHeight;

			if (isAbs) continue;

			if (( (multiline && (x - startX + p.calculatedWidth > maxInWidth)) || p.lineBreak) && (x - startX > 0)) {
				br = true;
				alignLine(i);
				y += maxLineHeight + verticalSpacing;
				maxLineHeight = 0;
				x = startX;
			}
			p.isBreak = br;
			x += p.calculatedWidth;
			if (x > cw) cw = x;
			x += horizontalSpacing;
			if (p.calculatedHeight > maxLineHeight) maxLineHeight = p.calculatedHeight;
		}
		alignLine(children.size());
		cw += paddingRight + borderRight;
		ch = y + maxLineHeight + paddingBottom + borderBottom;

		// horizontal align
		if (realMinWidth >= 0 && cw < realMinWidth) cw = realMinWidth;
		auto endX = cw - (paddingRight + borderRight);
		auto xmin = startX;
		auto xmax = endX;
		auto midSpace = 0;
		std::optional<FlowAlign> curAlign = nullopt;
		for (int i = 0; i < children.size();++i) {
			auto& p = propAt(i);
			auto  c = childAt(i);
			if (!c->visible) continue;
			if (p.isAbsolute) {
				if (p.horizontalAlign) {
					switch (*p.horizontalAlign) {
					case  FlowAlign::Right:
						c->x = endX - p.calculatedWidth + p.offsetX;
						break;

					case  FlowAlign::Left:
						c->x = startX + p.offsetX;
						break;

					case  FlowAlign::Middle:
						c->x = startX + int((endX - startX - p.calculatedWidth) * 0.5) + p.offsetX + startX;
						break;

					default:
						break;
					};
				}
				continue;
			}
			if (p.isBreak) {
				xmin = startX;
				xmax = endX;
				midSpace = 0;
			}
			auto px=0;
			auto align = !p.horizontalAlign ? halign : p.horizontalAlign;
			if (curAlign != align) {
				curAlign = align;
				midSpace = 0;
			}
			if(align)
			switch (*align) {
			case  FlowAlign::Right:
				if (midSpace == 0) {
					auto _remSize = p.calculatedWidth + remSize(i + 1);
					midSpace = (xmax - xmin) - _remSize;
					xmin += midSpace;
				}
				px = xmin;
				xmin += p.calculatedWidth + horizontalSpacing;
			case  FlowAlign::Middle:
				if (midSpace == 0) {
					auto _remSize = p.calculatedWidth + remSize(i + 1);
					midSpace = int(((xmax - xmin) - _remSize) * 0.5);
					xmin += midSpace;
				}
				px = xmin;
				xmin += p.calculatedWidth + horizontalSpacing;
			default:
				px = xmin;
				xmin += p.calculatedWidth + horizontalSpacing;
			}
			else {
				px = xmin;
				xmin += p.calculatedWidth + horizontalSpacing;
			}
			c->x = px + p.offsetX + p.paddingLeft;
			if (p.isAbsolute) xmin = px;
		}
	}
	break;
	
	case FlowLayout::Vertical: {
		auto halign = (horizontalAlign == nullopt) ? FlowAlign::Left : horizontalAlign;
		auto valign = (verticalAlign == nullopt) ? FlowAlign::Top : verticalAlign;

		auto startY = paddingTop + borderTop;
		auto y = startY;
		auto x = paddingLeft + borderLeft;
		ch = y;
		auto maxColWidth = 0;
		auto minColWidth = (this->colWidth != nullopt) ? (*colWidth) : (this->realMinWidth >= 0 && !multiline) ? (this->realMinWidth - (paddingLeft + paddingRight + borderLeft + borderRight)) : 0;
		auto lastIndex = 0;

		auto alignLine = [&](int maxIndex) {
			if (maxColWidth < minColWidth)
				maxColWidth = minColWidth;
			else if (overflow != FlowOverflow::Expand && minColWidth != 0)
				maxColWidth = minColWidth;
			for (int i = lastIndex; i < maxIndex; ++i) {
				auto& p = propAt(i);
				if (p.isAbsolute && (p.horizontalAlign == nullopt)) continue;
				auto c = childAt(i);
				if (!c->visible) continue;
				auto a = (p.horizontalAlign != nullopt) ? *p.horizontalAlign : *halign;
				c->x = x + p.offsetX + p.paddingLeft;
				switch (a) {
				case FlowAlign::Right:
					c->x += maxColWidth - p.calculatedWidth;
					break;
				case FlowAlign::Middle:
					c->x += int((maxColWidth - p.calculatedWidth) * 0.5);
					break;
				default:
					break;
				};
			}
			lastIndex = maxIndex;
		};

		auto remSize = [&, this](int from) {
			auto size = 0;
			for (int j = from; j < children.size(); ++j) {
				auto& p = propAt(j);
				if (p.isAbsolute || !childAt(j)->visible) continue;
				if (p.isBreak) break;
				size += verticalSpacing + p.calculatedHeight;
			}
			return size;
		};

		for(int i = 0; i < children.size();++i) {
			auto& p = propAt(i);
			auto isAbs = p.isAbsolute;
			if (isAbs && (p.horizontalAlign == nullopt) && (p.verticalAlign == nullopt)) continue;

			auto c = childAt(i);
			if (!c->visible) continue;

			auto pw = p.paddingLeft + p.paddingRight;
			auto ph = p.paddingTop + p.paddingBottom;
			if (!isAbs) {
				/*
				c.constraintSize(
					isConstraintWidth && p.constraint ? (maxInWidth - pw) / abs(c.scaleX) : -1,
					isConstraintHeight && p.constraint ? (maxInHeight - ph) / abs(c.scaleY) : -1
				);
				*/
			}

			auto b = getSize(c);
			auto br = false;

			p.calculatedWidth = ceil(b.xMax) + pw;
			p.calculatedHeight = ceil(b.yMax) + ph;
			if ( (p.minWidth != nullopt) && (p.calculatedWidth < p.minWidth) ) p.calculatedWidth = *p.minWidth;
			if ( (p.minHeight != nullopt) && (p.calculatedHeight < p.minHeight) ) p.calculatedHeight = *p.minHeight;

			if (isAbs) continue;

			if (((multiline && (y - startY + p.calculatedHeight > maxInHeight)) || p.lineBreak) && (y - startY > 0)) {
				br = true;
				alignLine(i);
				x += maxColWidth + horizontalSpacing;
				maxColWidth = 0;
				y = startY;
			}
			p.isBreak = br;
			c->y = y + p.offsetY + p.paddingTop;
			y += p.calculatedHeight;
			if (y > ch) ch = y;
			y += verticalSpacing;
			if (p.calculatedWidth > maxColWidth) maxColWidth = p.calculatedWidth;
		}
		alignLine(children.size());
		ch += paddingBottom + borderBottom;
		cw = x + maxColWidth + paddingRight + borderRight;

		// vertical align
		if (realMinHeight >= 0 && ch < realMinHeight) ch = realMinHeight;
		int endY = ch - (paddingBottom + borderBottom);
		auto ymin = startY;
		auto ymax = endY;
		auto midSpace = 0; 
		std::optional<FlowAlign> curAlign = nullopt;
		for (int i = 0; i < children.size();++i) {
			auto& p = propAt(i);
			auto c = childAt(i);
			if (!c->visible)
				continue;
			if (p.isAbsolute) {
				if(p.verticalAlign)
				switch (*p.verticalAlign) {
				case FlowAlign::Bottom:
					c->y = endY - p.calculatedHeight + p.offsetY;
					break;
				case FlowAlign::Top:
					c->y = startY + p.offsetY;
					break;
				case FlowAlign::Middle:
					c->y = startY + int((endY - startY - p.calculatedHeight) * 0.5) + p.offsetY + startY;
					break;
				default:
					break;
				}
				continue;
			}
			if (p.isBreak) {
				ymin = startY;
				ymax = endY;
				midSpace = 0;
			}
			auto py = 0;
			auto align = (p.verticalAlign == nullopt) ? valign : p.verticalAlign;
			if (curAlign != align) {
				curAlign = align;
				midSpace = 0;
			}
			if(align)
			switch (*align) {
			case FlowAlign::Bottom:
				if (midSpace == 0) {
					auto _remSize = p.calculatedHeight + remSize(i + 1);
					midSpace = (ymax - ymin) - _remSize;
					ymin += midSpace;
				}
				py = ymin;
				ymin += p.calculatedHeight + verticalSpacing;
				break;
			case FlowAlign::Middle:
				if (midSpace == 0) {
					auto _remSize = p.calculatedHeight + remSize(i + 1);
					midSpace = int(((ymax - ymin) - _remSize) * 0.5);
					ymin += midSpace;
				}
				py = ymin;
				ymin += p.calculatedHeight + verticalSpacing;
				break;
			default:
				py = ymin;
				ymin += p.calculatedHeight + verticalSpacing;
				break;
			}
			c->y = py + p.offsetY + p.paddingTop;
		}
	} 
	break;
	
	case FlowLayout::Stack: {
		auto halign = (!horizontalAlign) ? FlowAlign::Left : horizontalAlign;
		auto valign = (!verticalAlign) ? FlowAlign::Top : verticalAlign;

		auto maxChildW = 0;
		auto maxChildH = 0;

		for (int i = 0; i < children.size();++i) {
			auto c = childAt(i);
			if (!c->visible) continue;
			auto& p = propAt(i);
			auto isAbs = p.isAbsolute;
			if (isAbs && (p.verticalAlign == nullopt) && (p.horizontalAlign == nullopt)) continue;

			auto pw = p.paddingLeft + p.paddingRight;
			auto ph = p.paddingTop + p.paddingBottom;
			if (!isAbs) {
				//c.constraintSize(
				//	isConstraintWidth && p.constraint ? (maxInWidth - pw) / abs(c->scaleX) : -1,
				//	isConstraintHeight && p.constraint ? (maxInHeight - ph) / abs(c->scaleY) : -1
				//);
			}

			auto b = getSize(c);
			p.calculatedWidth = ceil(b.xMax) + pw;
			p.calculatedHeight = ceil(b.yMax) + ph;
			if ((p.minWidth != nullopt) && (p.calculatedWidth < p.minWidth)) p.calculatedWidth = *p.minWidth;
			if ((p.minHeight != nullopt) && (p.calculatedHeight < p.minHeight)) p.calculatedHeight = *p.minHeight;
			if (isAbs) continue;
			if (p.calculatedWidth > maxChildW) maxChildW = p.calculatedWidth;
			if (p.calculatedHeight > maxChildH) maxChildH = p.calculatedHeight;
		}

		auto xmin = paddingLeft + borderLeft;
		auto ymin = paddingTop + borderTop;
		auto xmax = (realMaxWidth > 0 && overflow != FlowOverflow::Expand) ? floor(realMaxWidth - (paddingRight + borderRight))
		: max<int>(xmin + maxChildW, realMinWidth - (paddingRight + borderRight));
		auto ymax = (realMaxWidth > 0 && overflow != FlowOverflow::Expand) ? floor(realMaxHeight - (paddingBottom + borderBottom))
		: max<int>(ymin + maxChildH, realMinHeight - (paddingBottom + borderBottom));
		cw = xmax + paddingRight + borderRight;
		ch = ymax + paddingBottom + borderBottom;

		for (int i = 0; i < children.size();++i) {
			auto c = childAt(i);
			if (!c->visible) continue;
			auto& p = propAt(i);
			auto isAbs = p.isAbsolute;
			if (isAbs && p.verticalAlign == nullopt  && p.horizontalAlign == nullopt ) continue;

			auto rvalign = (p.verticalAlign == nullopt) ? *valign : *p.verticalAlign;
			auto rhalign = (p.horizontalAlign == nullopt) ? *halign : *p.horizontalAlign;

			auto px = xmin;
			switch (rhalign) {
			case FlowAlign::Right:
				px = xmax - p.calculatedWidth;
				break;
			case FlowAlign::Middle:
				px = xmin + int(((xmax - xmin) - p.calculatedWidth) * 0.5);
				break;
			default:
				break;
			}

			auto py = ymin;
			switch (rvalign) {
			case FlowAlign::Bottom:
				py = ymax - p.calculatedHeight;
				break;
			case FlowAlign::Middle:
				py = ymin + int(((ymax - ymin) - p.calculatedHeight) * 0.5);
				break;
			default:
				break;
			}

			if (!isAbs || (p.horizontalAlign != nullopt))
				c->x = px + p.offsetX + p.paddingLeft;
			if (!isAbs || (p.verticalAlign != nullopt))
				c->y = py + p.offsetY + p.paddingTop;
		}
	}
	break;
	}

	/*
	if (scrollPosY != 0) {
		int i = 0;
		int sy = int(scrollPosY);
		for (c in children) {
			auto p = properties[i++];
			if (p.isAbsolute) continue;
			c.y -= sy;
		}
	}
	*/

	if (realMinWidth >= 0 && cw < realMinWidth) cw = realMinWidth;
	if (realMinHeight >= 0 && ch < realMinHeight) ch = realMinHeight;

	contentWidth = cw;
	contentHeight = ch;

	if (overflow != FlowOverflow::Expand) {
		if (isConstraintWidth && cw > maxTotWidth) cw = maxTotWidth;
		if (isConstraintHeight && ch > maxTotHeight) ch = maxTotHeight;
	}

	//if (interactive != null) {
	//	interactive.width = cw;
	//	interactive.height = ch;
	//}

	//if (background != null) {
	//	background.width = Math.ceil(cw);
	//	background.height = Math.ceil(ch);
	//}

	calculatedWidth = cw;
	calculatedHeight = ch;

	/*if (scrollBar != null) {
		if (contentHeight <= calculatedHeight)
			scrollBar.visible = false;
		else {
			scrollBar.visible = true;
			scrollBar.minHeight = Math.ceil(calculatedHeight);
			scrollBarCursor.minHeight = hxd.Math.imax(1, Std.int(calculatedHeight * (1 - (contentHeight - calculatedHeight) / contentHeight)));
			updateScrollCursor();
		}
	}*/

	needReflow = false;
	if ( //overflow == FlowOverflow::Scroll || 
		overflow == FlowOverflow::Hidden) {
		posChanged = true;
	}

	if (debug) {
		
	}

	//this.tmpBounds = tmpBounds;
	afterReflow();
}

void Flow::onAddChild(Node* s) {
	int idx = getChildIndex(s);
	properties.insert( properties.begin() + idx, FlowItemProperty());
	needReflow = true;
}

void Flow::onRemoveChild(Node*s){
	auto index = getChildIndex(s);
	if (index >= 0) {
		needReflow = true;
		properties.erase( properties.begin() + index);

		if (rs::Std::exists(constraints.objs, s)) {
			constraints.objs.erase(constraints.objs.find(s));
		}
	}
}

static const char* FlowLayouts[] = {
		"Horizontal",
		"Vertical",
		"Stack"
};

static const char* FlowAligns[] = {
	"Top",
	"Left",
	"Right",
	"Middle",
	"Bottom",
};

void Flow::im() {
	SUPER::im();
	using namespace ImGui;
	bool rebuild = false;
	if (CollapsingHeader(ICON_MD_DASHBOARD " Flow")) {
		Indent();

		if (TreeNode("Layout")) {
			ImGui::SetNextItemWidth(150);
			rebuild |= Combo("layout", (int*)&layout, FlowLayouts, IM_ARRAYSIZE(FlowLayouts));
			rebuild |= Checkbox("isInline", &isInline);
			rebuild |= Checkbox("multiline", &multiline);
			rebuild |= Checkbox("reverse", &this->reverse);
			//Value("fillWidth", fillWidth);
			//Value("fillHeight", fillHeight);
			TreePop();		
		}

		if (TreeNode("Control")) {
			Checkbox("needReflow", &needReflow			);

			if (horizontalAlign) {
				rebuild|=Combo("halign", (int*)&*horizontalAlign, FlowAligns, IM_ARRAYSIZE(FlowAligns));
				SameLine();
				if (Button(ICON_MD_DELETE"##halign")) {
					horizontalAlign = nullopt;
					rebuild = true;
				}
			}
			else {
				LabelText("halign", "nullopt"); SameLine(); 
				if (Button(ICON_MD_CHECK "##halign")) {
					horizontalAlign = FlowAlign::Left;
					rebuild = true;

				}
			}

			if (verticalAlign) {
				rebuild|=Combo("valign", (int*)&*verticalAlign, FlowAligns, IM_ARRAYSIZE(FlowAligns));
				SameLine();
				if (Button(ICON_MD_DELETE "##valign")) {
					verticalAlign = nullopt;
					rebuild = true;
				}
			}
			else {
				LabelText("valign", "nullopt"); SameLine();
				if (Button(ICON_MD_CHECK "##valign")) {
					verticalAlign = FlowAlign::Top;
					rebuild = true;
				}
			}

			if (rebuild |= DragInt("minWidth", minWidth)) 
				setMinWidth(minWidth);

			if (rebuild |= DragInt("maxWidth", maxWidth))
				setMaxWidth(maxWidth);

			if (rebuild |= DragInt("minHeight", minHeight))
				setMinHeight(minHeight);
			
			if (rebuild |= DragInt("maxHeight", maxHeight))
				setMaxHeight(maxHeight);

			rebuild |= DragInt("lineHeight", lineHeight	);
			rebuild |= DragInt("colWidth", colWidth		);
			TreePop();
		}

		if (TreeNode("Boxing")){
			rebuild |= DragInt("paddingHorizontal ", &paddingHorizontal);
			rebuild |= DragInt("paddingVertical  ", &paddingVertical);
			rebuild |= DragInt("paddingLeft 	 ", &paddingLeft);
			rebuild |= DragInt("paddingTop	 ", &paddingTop);
			rebuild |= DragInt("paddingRight ", &paddingRight);
			rebuild |= DragInt("paddingBottom ", &paddingBottom);
			rebuild |= DragInt("horizontalSpacing ", &horizontalSpacing);
			rebuild |= DragInt("verticalSpacing  ", &verticalSpacing);

			//Value("borderWidth ", borderWidth);
			//Value("borderHeight ", borderHeight);
			//Value("borderLeft 	 ", borderLeft);
			//Value("borderRight 	 ", borderRight);
			//Value("borderTop 	 ", borderTop);
			//Value("borderBottom  ", borderBottom);

			Value("innerWidth 	 ", innerWidth);
			Value("innerHeight 	 ", innerHeight);

			Value("outerWidth 	 ", outerWidth);
			Value("outerHeight 	 ", outerHeight);
			TreePop();
		}
		if (TreeNode("Calcs.")) {
			Value("calculatedWidth	", calculatedWidth);
			Value("calculatedHeight	", calculatedHeight);
			Value("contentWidth 	", contentWidth);
			Value("contentHeight 	", contentHeight);
			Value("constraintWidth 	", constraintWidth);
			Value("constraintHeight	", constraintHeight);
			Value("realMaxWidth 	", realMaxWidth);
			Value("realMaxHeight	", realMaxHeight);
			Value("realMinWidth 	", realMinWidth);
			Value("realMinHeight  	", realMinHeight);
			TreePop();
		}
		Unindent();
	}
	if (rebuild) {
		needReflow = true;
	}
}

void Flow::updateConstraint() {
	auto oldW = realMaxWidth; 
	auto oldH = realMaxHeight;
	realMaxWidth = (!maxWidth.has_value()) ? constraintWidth : ((constraintWidth < 0) ? *maxWidth : min<float>(*maxWidth, constraintWidth));
	realMaxHeight = (!maxHeight.has_value()) ? constraintHeight : ((constraintHeight < 0) ? *maxHeight : min<float>(*maxHeight, constraintHeight));
	if (minWidth.has_value() && (realMaxWidth < minWidth) && (realMaxWidth >= 0))
		realMaxWidth = *minWidth;
	if (minHeight.has_value() && (realMaxHeight < minHeight) && (realMaxWidth >= 0))
		realMaxHeight = *minHeight;
	if (realMaxWidth != oldW || realMaxHeight != oldH)
		needReflow = true;

	auto _oldW = realMinWidth;
	auto _oldH = realMinHeight;
	realMinWidth = (minWidth && fillWidth) ? ceil(constraintWidth) : ((minWidth) ? (*minWidth) : -1);
	realMinHeight = (minHeight && fillHeight) ? ceil(constraintHeight) : ((minHeight) ? (*minHeight) : -1);
	if ((realMinWidth != _oldW) || (realMinHeight != _oldH))
		needReflow = true;
}

void Flow::addConstraint(r2::Node* n, int x, int y) {
	constraints.objs[n] = Vector2i(x, y);
	//don't take into account right now
}


#undef SUPER
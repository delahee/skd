#include "stdafx.h"

#include "../LuaScriptHost.hpp"

using namespace std;
using namespace r2;
using namespace rd;
using namespace Pasta;

#include "rd/PointerWrapper.hpp"

#ifndef HBC_NO_LUA_IMGUI
void LuaScriptHost::injectImGui(sol::state & luaSol) {
	auto imguiLua = luaSol["ImGui"].get_or_create<sol::table>();

#pragma region REGION: Add wrapper around raw pointer for base type	
	luaSol.new_usertype<PointerWrapperChar>("CharBuffer",
		sol::constructors<PointerWrapperChar(), PointerWrapperChar(int)>(),
		"GetPtr", &PointerWrapperChar::GetPtr,
		"GetVoidPtr", &PointerWrapperChar::GetVoidPtr,
		"GetSize", &PointerWrapperChar::GetSize,
		"GetAt", &PointerWrapperChar::GetAt,
		"SetAt", &PointerWrapperChar::SetAt,
		"MemSet", sol::overload(
			sol::resolve<void(char)>(&PointerWrapperChar::MemSet),
			sol::resolve<void(char, int)>(&PointerWrapperChar::MemSet)
		),
		"MemCpy", &PointerWrapperChar::MemCpy,
		"StrCpy", &PointerWrapperChar::StrCpy
		);
	luaSol.new_usertype<PointerWrapper<float>>("FloatPtr",
		sol::constructors<PointerWrapper<float>(), PointerWrapper<float>(int)>(),
		"GetPtr", &PointerWrapper<float>::GetPtr,
		"GetVoidPtr", &PointerWrapper<float>::GetVoidPtr,
		"GetSize", &PointerWrapper<float>::GetSize,
		"Get", &PointerWrapper<float>::Get,
		"GetAt", &PointerWrapper<float>::GetAt,
		"Set", &PointerWrapper<float>::Set,
		"SetAt", &PointerWrapper<float>::SetAt,
		"MemSet", sol::overload(
			sol::resolve<void(float)>(&PointerWrapper<float>::MemSet),
			sol::resolve<void(float, int)>(&PointerWrapper<float>::MemSet)
		),
		"MemCpy", &PointerWrapper<float>::MemCpy
		);
	luaSol.new_usertype<PointerWrapper<int>>("IntPtr",
		sol::constructors<PointerWrapper<int>(), PointerWrapper<int>(int)>(),
		"GetPtr", &PointerWrapper<int>::GetPtr,
		"GetVoidPtr", &PointerWrapper<int>::GetVoidPtr,
		"GetSize", &PointerWrapper<int>::GetSize,
		"Get", &PointerWrapper<int>::Get,
		"GetAt", &PointerWrapper<int>::GetAt,
		"Set", &PointerWrapper<int>::Set,
		"SetAt", &PointerWrapper<int>::SetAt,
		"MemSet", sol::overload(
			sol::resolve<void(int)>(&PointerWrapper<int>::MemSet),
			sol::resolve<void(int, int)>(&PointerWrapper<int>::MemSet)
		),
		"MemCpy", &PointerWrapper<int>::MemCpy
		);
	luaSol.new_usertype<PointerWrapper<bool>>("BoolPtr",
		sol::constructors<PointerWrapper<bool>()>(),
		"GetPtr", &PointerWrapper<bool>::GetPtr,
		"GetVoidPtr", &PointerWrapper<bool>::GetVoidPtr,
		"Get", &PointerWrapper<bool>::Get,
		"Set", &PointerWrapper<bool>::Set
		);
	luaSol.new_usertype<PointerWrapper<Vector2>>("Vector2Ptr",
		sol::constructors<PointerWrapper<Vector2>()>(),
		"GetPtr", [](PointerWrapper<Vector2> *self) { return (float*)self->GetVoidPtr(); },
		"GetVoidPtr", &PointerWrapper<Vector2>::GetVoidPtr,
		"Get", &PointerWrapper<Vector2>::Get,
		"Set", &PointerWrapper<Vector2>::Set
		);
	luaSol.new_usertype<PointerWrapper<Vector3>>("Vector3Ptr",
		sol::constructors<PointerWrapper<Vector3>()>(),
		"GetPtr", [](PointerWrapper<Vector3> *self) { return (float*)self->GetVoidPtr(); },
		"GetVoidPtr", &PointerWrapper<Vector3>::GetVoidPtr,
		"Get", &PointerWrapper<Vector3>::Get,
		"Set", &PointerWrapper<Vector3>::Set
		);
	luaSol.new_usertype<PointerWrapper<Vector4>>("Vector4Ptr",
		sol::constructors<PointerWrapper<Vector4>()>(),
		"GetPtr", [](PointerWrapper<Vector4> *self) { return (float*)self->GetVoidPtr(); },
		"GetVoidPtr", &PointerWrapper<Vector4>::GetVoidPtr,
		"Get", &PointerWrapper<Vector4>::Get,
		"Set", &PointerWrapper<Vector4>::Set
		);
	luaSol.new_usertype<PointerWrapper<r::Color>>("ColorPtr",
		sol::constructors<PointerWrapper<r::Color>()>(),
		"GetPtr", [](PointerWrapper<r::Color> *self) { return (float*)self->GetVoidPtr(); },
		"GetVoidPtr", &PointerWrapper<r::Color>::GetVoidPtr,
		"Get", &PointerWrapper<r::Color>::Get,
		"Set", &PointerWrapper<r::Color>::Set
	);
	luaSol.new_usertype<PointerWrapper<rd::Vars>>("VarsPtr",
		sol::constructors<PointerWrapper<rd::Vars>()>(),
		"GetPtr", &PointerWrapper<rd::Vars>::GetPtr,
		"GetVoidPtr", &PointerWrapper<rd::Vars>::GetVoidPtr,
		"Get", &PointerWrapper<rd::Vars>::Get,
		"Set", &PointerWrapper<rd::Vars>::Set
	);
#pragma endregion

#pragma region [TODO] REGION: Add ImGui types & enums
	imguiLua.new_usertype<ImVec2>("ImVec2",
		sol::constructors<ImVec2(), ImVec2(float, float)>(),
		"x", &ImVec2::x,
		"y", &ImVec2::y
	);

	imguiLua.new_usertype<ImVec4>("ImVec4",
		sol::constructors<ImVec4(), ImVec4(float, float, float, float)>(),
		"x", &ImVec4::x,
		"y", &ImVec4::y,
		"z", &ImVec4::z,
		"w", &ImVec4::w
	);

	//Add missing types

	imguiLua["ImGuiWindowFlags_None"] = ImGuiWindowFlags_None;
	imguiLua["ImGuiWindowFlags_NoTitleBar"] = ImGuiWindowFlags_NoTitleBar;
	imguiLua["ImGuiWindowFlags_NoResize"] = ImGuiWindowFlags_NoResize;
	imguiLua["ImGuiWindowFlags_NoMove"] = ImGuiWindowFlags_NoMove;
	imguiLua["ImGuiWindowFlags_NoScrollbar"] = ImGuiWindowFlags_NoScrollbar;
	imguiLua["ImGuiWindowFlags_NoScrollWithMouse"] = ImGuiWindowFlags_NoScrollWithMouse;
	imguiLua["ImGuiWindowFlags_NoCollapse"] = ImGuiWindowFlags_NoCollapse;
	imguiLua["ImGuiWindowFlags_AlwaysAutoResize"] = ImGuiWindowFlags_AlwaysAutoResize;
	imguiLua["ImGuiWindowFlags_NoBackground"] = ImGuiWindowFlags_NoBackground;
	imguiLua["ImGuiWindowFlags_NoSavedSettings"] = ImGuiWindowFlags_NoSavedSettings;
	imguiLua["ImGuiWindowFlags_NoMouseInputs"] = ImGuiWindowFlags_NoMouseInputs;
	imguiLua["ImGuiWindowFlags_MenuBar"] = ImGuiWindowFlags_MenuBar;
	imguiLua["ImGuiWindowFlags_HorizontalScrollbar"] = ImGuiWindowFlags_HorizontalScrollbar;
	imguiLua["ImGuiWindowFlags_NoFocusOnAppearing"] = ImGuiWindowFlags_NoFocusOnAppearing;
	imguiLua["ImGuiWindowFlags_NoBringToFrontOnFocus"] = ImGuiWindowFlags_NoBringToFrontOnFocus;
	imguiLua["ImGuiWindowFlags_AlwaysVerticalScrollbar"] = ImGuiWindowFlags_AlwaysVerticalScrollbar;
	imguiLua["ImGuiWindowFlags_AlwaysHorizontalScrollbar"] = ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	imguiLua["ImGuiWindowFlags_AlwaysUseWindowPadding"] = ImGuiWindowFlags_AlwaysUseWindowPadding;
	imguiLua["ImGuiWindowFlags_NoNavInputs"] = ImGuiWindowFlags_NoNavInputs;
	imguiLua["ImGuiWindowFlags_NoNavFocus"] = ImGuiWindowFlags_NoNavFocus;
	imguiLua["ImGuiWindowFlags_UnsavedDocument"] = ImGuiWindowFlags_UnsavedDocument;
	imguiLua["ImGuiWindowFlags_NoDocking"] = ImGuiWindowFlags_NoDocking;
	imguiLua["ImGuiWindowFlags_NoNav"] = ImGuiWindowFlags_NoNav;
	imguiLua["ImGuiWindowFlags_NoDecoration"] = ImGuiWindowFlags_NoDecoration;
	imguiLua["ImGuiWindowFlags_NoInputs"] = ImGuiWindowFlags_NoInputs;

	imguiLua["ImGuiInputTextFlags_None"] = ImGuiInputTextFlags_None;
	imguiLua["ImGuiInputTextFlags_CharsDecimal"] = ImGuiInputTextFlags_CharsDecimal;
	imguiLua["ImGuiInputTextFlags_CharsHexadecimal"] = ImGuiInputTextFlags_CharsHexadecimal;
	imguiLua["ImGuiInputTextFlags_CharsUppercase"] = ImGuiInputTextFlags_CharsUppercase;
	imguiLua["ImGuiInputTextFlags_CharsNoBlank"] = ImGuiInputTextFlags_CharsNoBlank;
	imguiLua["ImGuiInputTextFlags_AutoSelectAll"] = ImGuiInputTextFlags_AutoSelectAll;
	imguiLua["ImGuiInputTextFlags_EnterReturnsTrue"] = ImGuiInputTextFlags_EnterReturnsTrue;
	imguiLua["ImGuiInputTextFlags_CallbackCompletion"] = ImGuiInputTextFlags_CallbackCompletion;
	imguiLua["ImGuiInputTextFlags_CallbackHistory"] = ImGuiInputTextFlags_CallbackHistory;
	imguiLua["ImGuiInputTextFlags_CallbackAlways"] = ImGuiInputTextFlags_CallbackAlways;
	imguiLua["ImGuiInputTextFlags_CallbackCharFilter"] = ImGuiInputTextFlags_CallbackCharFilter;
	imguiLua["ImGuiInputTextFlags_AllowTabInput"] = ImGuiInputTextFlags_AllowTabInput;
	imguiLua["ImGuiInputTextFlags_CtrlEnterForNewLine"] = ImGuiInputTextFlags_CtrlEnterForNewLine;
	imguiLua["ImGuiInputTextFlags_NoHorizontalScroll"] = ImGuiInputTextFlags_NoHorizontalScroll;
	imguiLua["ImGuiInputTextFlags_AlwaysOverwrite"] = ImGuiInputTextFlags_AlwaysOverwrite;
	imguiLua["ImGuiInputTextFlags_ReadOnly"] = ImGuiInputTextFlags_ReadOnly;
	imguiLua["ImGuiInputTextFlags_Password"] = ImGuiInputTextFlags_Password;
	imguiLua["ImGuiInputTextFlags_NoUndoRedo"] = ImGuiInputTextFlags_NoUndoRedo;
	imguiLua["ImGuiInputTextFlags_CharsScientific"] = ImGuiInputTextFlags_CharsScientific;
	imguiLua["ImGuiInputTextFlags_CallbackResize"] = ImGuiInputTextFlags_CallbackResize;


	imguiLua["ImGuiTreeNodeFlags_None"] = ImGuiTreeNodeFlags_None;
	imguiLua["ImGuiTreeNodeFlags_Selected"] = ImGuiTreeNodeFlags_Selected;
	imguiLua["ImGuiTreeNodeFlags_Framed"] = ImGuiTreeNodeFlags_Framed;
	imguiLua["ImGuiTreeNodeFlags_AllowItemOverlap"] = ImGuiTreeNodeFlags_AllowItemOverlap;
	imguiLua["ImGuiTreeNodeFlags_NoTreePushOnOpen"] = ImGuiTreeNodeFlags_NoTreePushOnOpen;
	imguiLua["ImGuiTreeNodeFlags_NoAutoOpenOnLog"] = ImGuiTreeNodeFlags_NoAutoOpenOnLog;
	imguiLua["ImGuiTreeNodeFlags_DefaultOpen"] = ImGuiTreeNodeFlags_DefaultOpen;
	imguiLua["ImGuiTreeNodeFlags_OpenOnDoubleClick"] = ImGuiTreeNodeFlags_OpenOnDoubleClick;
	imguiLua["ImGuiTreeNodeFlags_OpenOnArrow"] = ImGuiTreeNodeFlags_OpenOnArrow;
	imguiLua["ImGuiTreeNodeFlags_Leaf"] = ImGuiTreeNodeFlags_Leaf;
	imguiLua["ImGuiTreeNodeFlags_Bullet"] = ImGuiTreeNodeFlags_Bullet;
	imguiLua["ImGuiTreeNodeFlags_FramePadding"] = ImGuiTreeNodeFlags_FramePadding;
	imguiLua["ImGuiTreeNodeFlags_SpanAvailWidth"] = ImGuiTreeNodeFlags_SpanAvailWidth;
	imguiLua["ImGuiTreeNodeFlags_SpanFullWidth"] = ImGuiTreeNodeFlags_SpanFullWidth;
	imguiLua["ImGuiTreeNodeFlags_NavLeftJumpsBackHere"] = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
	imguiLua["ImGuiTreeNodeFlags_CollapsingHeader"] = ImGuiTreeNodeFlags_CollapsingHeader;

	imguiLua["ImGuiSelectableFlags_None"] = ImGuiSelectableFlags_None;
	imguiLua["ImGuiSelectableFlags_DontClosePopups"] = ImGuiSelectableFlags_DontClosePopups;
	imguiLua["ImGuiSelectableFlags_SpanAllColumns"] = ImGuiSelectableFlags_SpanAllColumns;
	imguiLua["ImGuiSelectableFlags_AllowDoubleClick"] = ImGuiSelectableFlags_AllowDoubleClick;
	imguiLua["ImGuiSelectableFlags_Disabled"] = ImGuiSelectableFlags_Disabled;
	imguiLua["ImGuiSelectableFlags_AllowItemOverlap"] = ImGuiSelectableFlags_AllowItemOverlap;

	imguiLua["ImGuiFocusedFlags_None"] = ImGuiFocusedFlags_None;
	imguiLua["ImGuiFocusedFlags_ChildWindows"] = ImGuiFocusedFlags_ChildWindows;
	imguiLua["ImGuiFocusedFlags_RootWindow"] = ImGuiFocusedFlags_RootWindow;
	imguiLua["ImGuiFocusedFlags_AnyWindow"] = ImGuiFocusedFlags_AnyWindow;
	imguiLua["ImGuiFocusedFlags_RootAndChildWindows"] = ImGuiFocusedFlags_RootAndChildWindows;

	imguiLua["ImGuiHoveredFlags_None"] = ImGuiHoveredFlags_None;
	imguiLua["ImGuiHoveredFlags_ChildWindows"] = ImGuiHoveredFlags_ChildWindows;
	imguiLua["ImGuiHoveredFlags_RootWindow"] = ImGuiHoveredFlags_RootWindow;
	imguiLua["ImGuiHoveredFlags_AnyWindow"] = ImGuiHoveredFlags_AnyWindow;
	imguiLua["ImGuiHoveredFlags_AllowWhenBlockedByPopup"] = ImGuiHoveredFlags_AllowWhenBlockedByPopup;
	imguiLua["ImGuiHoveredFlags_AllowWhenBlockedByActiveItem"] = ImGuiHoveredFlags_AllowWhenBlockedByActiveItem;
	imguiLua["ImGuiHoveredFlags_AllowWhenOverlapped"] = ImGuiHoveredFlags_AllowWhenOverlapped;
	imguiLua["ImGuiHoveredFlags_AllowWhenDisabled"] = ImGuiHoveredFlags_AllowWhenDisabled;
	imguiLua["ImGuiHoveredFlags_RectOnly"] = ImGuiHoveredFlags_RectOnly;
	imguiLua["ImGuiHoveredFlags_RootAndChildWindows"] = ImGuiHoveredFlags_RootAndChildWindows;

	imguiLua["ImGuiDataType_S8"] = ImGuiDataType_S8;
	imguiLua["ImGuiDataType_U8"] = ImGuiDataType_U8;
	imguiLua["ImGuiDataType_S16"] = ImGuiDataType_S16;
	imguiLua["ImGuiDataType_U16"] = ImGuiDataType_U16;
	imguiLua["ImGuiDataType_S32"] = ImGuiDataType_S32;
	imguiLua["ImGuiDataType_U32"] = ImGuiDataType_U32;
	imguiLua["ImGuiDataType_S64"] = ImGuiDataType_S64;
	imguiLua["ImGuiDataType_U64"] = ImGuiDataType_U64;
	imguiLua["ImGuiDataType_Float"] = ImGuiDataType_Float;
	imguiLua["ImGuiDataType_Double"] = ImGuiDataType_Double;

	imguiLua["ImGuiDir_None"] = ImGuiDir_None;
	imguiLua["ImGuiDir_Left"] = ImGuiDir_Left;
	imguiLua["ImGuiDir_Right"] = ImGuiDir_Right;
	imguiLua["ImGuiDir_Up"] = ImGuiDir_Up;
	imguiLua["ImGuiDir_Down"] = ImGuiDir_Down;

	imguiLua["ImGuiColorEditFlags_None"] = ImGuiColorEditFlags_None;
	imguiLua["ImGuiColorEditFlags_NoAlpha"] = ImGuiColorEditFlags_NoAlpha;
	imguiLua["ImGuiColorEditFlags_NoPicker"] = ImGuiColorEditFlags_NoPicker;
	imguiLua["ImGuiColorEditFlags_NoOptions"] = ImGuiColorEditFlags_NoOptions;
	imguiLua["ImGuiColorEditFlags_NoSmallPreview"] = ImGuiColorEditFlags_NoSmallPreview;
	imguiLua["ImGuiColorEditFlags_NoInputs"] = ImGuiColorEditFlags_NoInputs;
	imguiLua["ImGuiColorEditFlags_NoTooltip"] = ImGuiColorEditFlags_NoTooltip;
	imguiLua["ImGuiColorEditFlags_NoLabel"] = ImGuiColorEditFlags_NoLabel;
	imguiLua["ImGuiColorEditFlags_NoSidePreview"] = ImGuiColorEditFlags_NoSidePreview;
	imguiLua["ImGuiColorEditFlags_NoDragDrop"] = ImGuiColorEditFlags_NoDragDrop;
	imguiLua["ImGuiColorEditFlags_NoBorder"] = ImGuiColorEditFlags_NoBorder;
	imguiLua["ImGuiColorEditFlags_AlphaBar"] = ImGuiColorEditFlags_AlphaBar;
	imguiLua["ImGuiColorEditFlags_AlphaPreview"] = ImGuiColorEditFlags_AlphaPreview;
	imguiLua["ImGuiColorEditFlags_AlphaPreviewHalf"] = ImGuiColorEditFlags_AlphaPreviewHalf;
	imguiLua["ImGuiColorEditFlags_HDR"] = ImGuiColorEditFlags_HDR;
	imguiLua["ImGuiColorEditFlags_DisplayRGB"] = ImGuiColorEditFlags_DisplayRGB;
	imguiLua["ImGuiColorEditFlags_DisplayHSV"] = ImGuiColorEditFlags_DisplayHSV;
	imguiLua["ImGuiColorEditFlags_DisplayHex"] = ImGuiColorEditFlags_DisplayHex;
	imguiLua["ImGuiColorEditFlags_Uint8"] = ImGuiColorEditFlags_Uint8;
	imguiLua["ImGuiColorEditFlags_Float"] = ImGuiColorEditFlags_Float;
	imguiLua["ImGuiColorEditFlags_PickerHueBar"] = ImGuiColorEditFlags_PickerHueBar;
	imguiLua["ImGuiColorEditFlags_PickerHueWheel"] = ImGuiColorEditFlags_PickerHueWheel;
	imguiLua["ImGuiColorEditFlags_InputRGB"] = ImGuiColorEditFlags_InputRGB;
	imguiLua["ImGuiColorEditFlags_InputHSV"] = ImGuiColorEditFlags_InputHSV;
	imguiLua["ImGuiColorEditFlags_DefaultOptions_"] = ImGuiColorEditFlags_DefaultOptions_;

	imguiLua["ImGuiMouseButton_Left"] = ImGuiMouseButton_Left;
	imguiLua["ImGuiMouseButton_Right"] = ImGuiMouseButton_Right;
	imguiLua["ImGuiMouseButton_Middle"] = ImGuiMouseButton_Middle;

	imguiLua["ImGuiCond_Always"] = ImGuiCond_Always;
	imguiLua["ImGuiCond_Once"] = ImGuiCond_Once;
	imguiLua["ImGuiCond_FirstUseEver"] = ImGuiCond_FirstUseEver;
	imguiLua["ImGuiCond_Appearing"] = ImGuiCond_Appearing;

	imguiLua["ICON_FA_PLAY"] = ICON_FA_PLAY;
	imguiLua["ICON_FA_PAUSE"] = ICON_FA_PAUSE;
#pragma endregion

#pragma region [DONE] REGION: ImGui Windows
	imguiLua.set_function("Begin", sol::overload(
		[](const char* name) { return ImGui::Begin(name); },
		[](const char* name, bool* p_open) { return ImGui::Begin(name, (bool*)p_open); },
		[](const char* name, bool* p_open, ImGuiWindowFlags flags) { return ImGui::Begin(name, (bool*)p_open, flags); }
	));
	imguiLua.set_function("End", ImGui::End);
#pragma endregion

#pragma region [DONE] REGION: Child Windows
	imguiLua.set_function("BeginChild", sol::overload(
		[](const char* str_id) { return ImGui::BeginChild(str_id); },
		[](const char* str_id, const ImVec2& size) { return ImGui::BeginChild(str_id, size); },
		[](const char* str_id, const ImVec2& size, bool border) { return ImGui::BeginChild(str_id, size, border); },
		[](const char* str_id, const ImVec2& size, bool border, ImGuiWindowFlags flags) { return ImGui::BeginChild(str_id, size, border, flags); },
		[](ImGuiID id) { return ImGui::BeginChild(id); },
		[](ImGuiID id, const ImVec2& size) { return ImGui::BeginChild(id, size); },
		[](ImGuiID id, const ImVec2& size, bool border) { return ImGui::BeginChild(id, size, border); },
		[](ImGuiID id, const ImVec2& size, bool border, ImGuiWindowFlags flags) { return ImGui::BeginChild(id, size, border, flags); }
	));
	imguiLua.set_function("EndChild", ImGui::EndChild);
#pragma endregion

#pragma region [TODO] REGION: Windows Utilities
	imguiLua.set_function("IsWindowAppearing", ImGui::IsWindowAppearing);
	imguiLua.set_function("IsWindowCollapsed", ImGui::IsWindowCollapsed);
	imguiLua.set_function("IsWindowFocused", sol::overload(
		[]() { return ImGui::IsWindowFocused(); },
		[](ImGuiFocusedFlags flags) { return ImGui::IsWindowFocused(flags); }
	));
	imguiLua.set_function("IsWindowHovered", sol::overload(
		[]() { return ImGui::IsWindowHovered(); },
		[](ImGuiHoveredFlags flags) { return ImGui::IsWindowHovered(flags); }
	));
	imguiLua.set_function("GetWindowPos", ImGui::GetWindowPos);
	imguiLua.set_function("GetWindowSize", ImGui::GetWindowSize);
	imguiLua.set_function("GetWindowWidth", ImGui::GetWindowWidth);
	imguiLua.set_function("GetWindowHeight", ImGui::GetWindowHeight);
	// TODO: Add Set
#pragma endregion

#pragma region [DONE] REGION: Content region
	imguiLua.set_function("GetContentRegionMax", ImGui::GetContentRegionMax);
	imguiLua.set_function("GetContentRegionAvail", ImGui::GetContentRegionAvail);
	imguiLua.set_function("GetWindowContentRegionMin", ImGui::GetWindowContentRegionMin);
	imguiLua.set_function("GetWindowContentRegionMax", ImGui::GetWindowContentRegionMax);
	imguiLua.set_function("GetWindowContentRegionWidth", ImGui::GetWindowContentRegionWidth);
#pragma endregion

#pragma region [DONE] REGION: Windows Scrolling
	imguiLua.set_function("GetScrollX", ImGui::GetScrollX);
	imguiLua.set_function("GetScrollY", ImGui::GetScrollY);
	imguiLua.set_function("GetScrollMaxX", ImGui::GetScrollMaxX);
	imguiLua.set_function("GetScrollMaxY", ImGui::GetScrollMaxY);
	imguiLua.set_function("SetScrollX", ImGui::SetScrollX);
	imguiLua.set_function("SetScrollY", ImGui::SetScrollY);
	imguiLua.set_function("SetScrollHereX", sol::overload(
		[]() { return ImGui::SetScrollHereX(); },
		[](float center_x_ratio) { return ImGui::SetScrollHereX(center_x_ratio); }
	));
	imguiLua.set_function("SetScrollHereY", sol::overload(
		[]() { return ImGui::SetScrollHereY(); },
		[](float center_y_ratio) { return ImGui::SetScrollHereY(center_y_ratio); }
	));
	imguiLua.set_function("SetScrollFromPosX", sol::overload(
		[](float local_x) { return ImGui::SetScrollFromPosX(local_x); },
		[](float local_x, float center_x_ratio) { return ImGui::SetScrollFromPosX(local_x, center_x_ratio); }
	));
	imguiLua.set_function("SetScrollFromPosY", sol::overload(
		[](float local_y) { return ImGui::SetScrollFromPosY(local_y); },
		[](float local_y, float center_y_ratio) { return ImGui::SetScrollFromPosY(local_y, center_y_ratio); }
	));
#pragma endregion

#pragma region [TODO] REGION: Parameters stacks (shared)
	//TODO: Add Color, Style & Font options
#pragma endregion

#pragma region [DONE] REGION: Parameters stacks (current window)
	imguiLua.set_function("PushItemWidth", ImGui::PushItemWidth);
	imguiLua.set_function("PopItemWidth", ImGui::PopItemWidth);
	imguiLua.set_function("SetNextItemWidth", ImGui::SetNextItemWidth);
	imguiLua.set_function("CalcItemWidth", ImGui::CalcItemWidth);
	imguiLua.set_function("PushTextWrapPos", sol::overload(
		[]() { return ImGui::PushTextWrapPos(); },
		[](float wrap_local_pos_x) { return ImGui::PushTextWrapPos(wrap_local_pos_x); }
	));
	imguiLua.set_function("PopTextWrapPos", ImGui::PopTextWrapPos);
	imguiLua.set_function("PushAllowKeyboardFocus", ImGui::PushAllowKeyboardFocus);
	imguiLua.set_function("PopAllowKeyboardFocus", ImGui::PopAllowKeyboardFocus);
	imguiLua.set_function("PushButtonRepeat", ImGui::PushButtonRepeat);
	imguiLua.set_function("PopButtonRepeat", ImGui::PopButtonRepeat);
#pragma endregion

#pragma region [DONE] REGION: Cursor / Layout
	imguiLua.set_function("Separator", ImGui::Separator);
	imguiLua.set_function("SameLine", sol::overload(
		[]() { return ImGui::SameLine(); },
		[](float offset_from_start_x) { return ImGui::SameLine(offset_from_start_x); },
		[](float offset_from_start_x, float spacing) { return ImGui::SameLine(offset_from_start_x, spacing); }
	));
	imguiLua.set_function("NewLine", ImGui::NewLine);
	imguiLua.set_function("Spacing", ImGui::Spacing);
	imguiLua.set_function("Dummy", ImGui::Dummy);
	imguiLua.set_function("Indent", sol::overload(
		[]() { return ImGui::Indent(); },
		[](float indent_w) { return ImGui::Indent(indent_w); }
	));
	imguiLua.set_function("Unindent", sol::overload(
		[]() { return ImGui::Unindent(); },
		[](float indent_w) { return ImGui::Unindent(indent_w); }
	));
	imguiLua.set_function("BeginGroup", ImGui::BeginGroup);
	imguiLua.set_function("EndGroup", ImGui::EndGroup);
	imguiLua.set_function("GetCursorPos", ImGui::GetCursorPos);
	imguiLua.set_function("GetCursorPosX", ImGui::GetCursorPosX);
	imguiLua.set_function("GetCursorPosY", ImGui::GetCursorPosY);
	imguiLua.set_function("SetCursorPos", ImGui::SetCursorPos);
	imguiLua.set_function("SetCursorPosX", ImGui::SetCursorPosX);
	imguiLua.set_function("SetCursorPosY", ImGui::SetCursorPosY);
	imguiLua.set_function("GetCursorStartPos", ImGui::GetCursorStartPos);
	imguiLua.set_function("GetCursorScreenPos", ImGui::GetCursorScreenPos);
	imguiLua.set_function("SetCursorScreenPos", ImGui::SetCursorScreenPos);
	imguiLua.set_function("AlignTextToFramePadding", ImGui::AlignTextToFramePadding);
	imguiLua.set_function("GetTextLineHeight", ImGui::GetTextLineHeight);
	imguiLua.set_function("GetTextLineHeightWithSpacing", ImGui::GetTextLineHeightWithSpacing);
	imguiLua.set_function("GetFrameHeight", ImGui::GetFrameHeight);
	imguiLua.set_function("GetFrameHeightWithSpacing", ImGui::GetFrameHeightWithSpacing);
#pragma endregion

#pragma region [DONE] REGION: ID stack/scopes
	imguiLua.set_function("PushID", sol::overload(
		[](const char* str_id) { return ImGui::PushID(str_id); },
		[](const char* str_id_begin, const char* str_id_end) { return ImGui::PushID(str_id_begin, str_id_end); },
		[](const void* ptr_id) { return ImGui::PushID(ptr_id); }
	));
	imguiLua.set_function("PopID", ImGui::PopID);
	imguiLua.set_function("GetID", sol::overload(
		[](const char* str_id) { return ImGui::GetID(str_id); },
		[](const char* str_id_begin, const char* str_id_end) { return ImGui::GetID(str_id_begin, str_id_end); },
		[](const void* ptr_id) { return  ImGui::GetID(ptr_id); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Text		[TODO: Maybe add variadic args support]
	imguiLua.set_function("Text", sol::overload(
		[](const char* label)			{ return ImGui::Text(label); },
		[](const std::string& label)	{ return ImGui::Text(label); }
	));
	imguiLua.set_function("TextColored", ImGui::TextColored);
	imguiLua.set_function("TextDisabled", ImGui::TextDisabled);
	imguiLua.set_function("TextWrapped", ImGui::TextWrapped);
	imguiLua.set_function("LabelText", ImGui::LabelText);
	imguiLua.set_function("BulletText", ImGui::BulletText);
#pragma endregion

#pragma region [DONE] REGION: Widgets - Main
	imguiLua.set_function("Button", sol::overload(
		[](const char* text) { return ImGui::Button(text); },
		[](const char* text, const ImVec2& size) { return ImGui::Button(text, size); }
	));
	imguiLua.set_function("SmallButton", ImGui::SmallButton);
	imguiLua.set_function("InvisibleButton", ImGui::InvisibleButton);
	imguiLua.set_function("ArrowButton", ImGui::ArrowButton);
	imguiLua.set_function("Checkbox", ImGui::Checkbox);
	//seems to break on visual15
	//imguiLua.set_function("CheckboxFlags", sol::resolve<bool(const char*, int*, int)>(ImGui::CheckboxFlags));
	imguiLua.set_function("RadioButton", sol::overload(
		[](const char* label, bool active) { return ImGui::RadioButton(label, active); },
		[](const char* label, int* v, int v_button) { return ImGui::RadioButton(label, v, v_button); }
	));
	imguiLua.set_function("ProgressBar", sol::overload(
		[](float fraction) { return ImGui::ProgressBar(fraction); },
		[](float fraction, const ImVec2& size_arg) { return ImGui::ProgressBar(fraction, size_arg); },
		[](float fraction, const ImVec2& size_arg, const char* overlay) { return ImGui::ProgressBar(fraction, size_arg, overlay); }
	));
	imguiLua.set_function("Bullet", ImGui::Bullet);
#pragma endregion

#pragma region [DONE] REGION: Widgets - Combo Box
	imguiLua.set_function("BeginCombo", sol::overload(
		[](const char* label, const char* preview_value) { return ImGui::BeginCombo(label, preview_value); },
		[](const char* label, const char* preview_value, ImGuiComboFlags flags) { return ImGui::BeginCombo(label, preview_value, flags); }
	));
	imguiLua.set_function("EndCombo", ImGui::EndCombo);
	// deprecated Combo() not binded
#pragma endregion

#pragma region [DONE] REGION: Widgets - Drags
	imguiLua.set_function("DragFloat", sol::overload(
		[](const char* label, float* v) { return ImGui::DragFloat(label, v); },
		[](const char* label, float* v, float v_speed) { return ImGui::DragFloat(label, v, v_speed); },
		[](const char* label, float* v, float v_speed, float v_min) { return ImGui::DragFloat(label, v, v_speed, v_min); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max) { return ImGui::DragFloat(label, v, v_speed, v_min, v_max); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max, const char* format) { return ImGui::DragFloat(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragFloat2", sol::overload(
		[](const char* label, float* v) { return ImGui::DragFloat2(label, v); },
		[](const char* label, float* v, float v_speed) { return ImGui::DragFloat2(label, v, v_speed); },
		[](const char* label, float* v, float v_speed, float v_min) { return ImGui::DragFloat2(label, v, v_speed, v_min); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max) { return ImGui::DragFloat2(label, v, v_speed, v_min, v_max); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max, const char* format) { return ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragFloat3", sol::overload(
		[](const char* label, float* v) { return ImGui::DragFloat3(label, v); },
		[](const char* label, float* v, float v_speed) { return ImGui::DragFloat3(label, v, v_speed); },
		[](const char* label, float* v, float v_speed, float v_min) { return ImGui::DragFloat3(label, v, v_speed, v_min); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max) { return ImGui::DragFloat3(label, v, v_speed, v_min, v_max); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max, const char* format) { return ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragFloat4", sol::overload(
		[](const char* label, float* v) { return ImGui::DragFloat4(label, v); },
		[](const char* label, float* v, float v_speed) { return ImGui::DragFloat4(label, v, v_speed); },
		[](const char* label, float* v, float v_speed, float v_min) { return ImGui::DragFloat4(label, v, v_speed, v_min); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max) { return ImGui::DragFloat4(label, v, v_speed, v_min, v_max); },
		[](const char* label, float* v, float v_speed, float v_min, float v_max, const char* format) { return ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragFloatRange2", sol::overload(
		[](const char* label, float* v_current_min, float* v_current_max) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max); },
		[](const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max, float power) { return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, power); }
	));
	imguiLua.set_function("DragInt", sol::overload(
		[](const char* label, int* v) { return ImGui::DragInt(label, v); },
		[](const char* label, int* v, float v_speed) { return ImGui::DragInt(label, v, v_speed); },
		[](const char* label, int* v, float v_speed, int v_min) { return ImGui::DragInt(label, v, v_speed, v_min); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max) { return ImGui::DragInt(label, v, v_speed, v_min, v_max); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max, const char* format) { return ImGui::DragInt(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragInt2", sol::overload(
		[](const char* label, int* v) { return ImGui::DragInt2(label, v); },
		[](const char* label, int* v, float v_speed) { return ImGui::DragInt2(label, v, v_speed); },
		[](const char* label, int* v, float v_speed, int v_min) { return ImGui::DragInt2(label, v, v_speed, v_min); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max) { return ImGui::DragInt2(label, v, v_speed, v_min, v_max); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max, const char* format) { return ImGui::DragInt2(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragInt3", sol::overload(
		[](const char* label, int* v) { return ImGui::DragInt3(label, v); },
		[](const char* label, int* v, float v_speed) { return ImGui::DragInt3(label, v, v_speed); },
		[](const char* label, int* v, float v_speed, int v_min) { return ImGui::DragInt3(label, v, v_speed, v_min); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max) { return ImGui::DragInt3(label, v, v_speed, v_min, v_max); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max, const char* format) { return ImGui::DragInt3(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragInt4", sol::overload(
		[](const char* label, int* v) { return ImGui::DragInt4(label, v); },
		[](const char* label, int* v, float v_speed) { return ImGui::DragInt4(label, v, v_speed); },
		[](const char* label, int* v, float v_speed, int v_min) { return ImGui::DragInt4(label, v, v_speed, v_min); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max) { return ImGui::DragInt4(label, v, v_speed, v_min, v_max); },
		[](const char* label, int* v, float v_speed, int v_min, int v_max, const char* format) { return ImGui::DragInt4(label, v, v_speed, v_min, v_max, format); }
	));
	imguiLua.set_function("DragIntRange2", sol::overload(
		[](const char* label, int* v_current_min, int* v_current_max) { return ImGui::DragIntRange2(label, v_current_min, v_current_max); },
		[](const char* label, int* v_current_min, int* v_current_max, float v_speed) { return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed); },
		[](const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min) { return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min); },
		[](const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max) { return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max); },
		[](const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format) { return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format); },
		[](const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format, const char* format_max) { return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max); }
	));
	imguiLua.set_function("DragScalar", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data, float v_speed) { return ImGui::DragScalar(label, data_type, p_data, v_speed); },
		[](const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min) { return ImGui::DragScalar(label, data_type, p_data, v_speed, p_min); },
		[](const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max) { return ImGui::DragScalar(label, data_type, p_data, v_speed, p_min, p_max); },
		[](const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format) { return ImGui::DragScalar(label, data_type, p_data, v_speed, p_min, p_max, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, float power) { return ImGui::DragScalar(label, data_type, p_data, v_speed, p_min, p_max, format, power); }
	));
	imguiLua.set_function("DragScalarN", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed) { return ImGui::DragScalarN(label, data_type, p_data, components, v_speed); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min) { return ImGui::DragScalarN(label, data_type, p_data, components, v_speed, p_min); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max) { return ImGui::DragScalarN(label, data_type, p_data, components, v_speed, p_min, p_max); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format) { return ImGui::DragScalarN(label, data_type, p_data, components, v_speed, p_min, p_max, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, float power) { return ImGui::DragScalarN(label, data_type, p_data, components, v_speed, p_min, p_max, format, power); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Sliders
	imguiLua.set_function("SliderFloat", sol::overload(
		[](const char* label, float* v, float v_min, float v_max) { return ImGui::SliderFloat(label, v, v_min, v_max); },
		[](const char* label, float* v, float v_min, float v_max, const char* format) { return ImGui::SliderFloat(label, v, v_min, v_max, format); },
		[](const char* label, float* v, float v_min, float v_max, const char* format, float power) { return ImGui::SliderFloat(label, v, v_min, v_max, format, power); }
	));
	imguiLua.set_function("SliderFloat2", sol::overload(
		[](const char* label, float* v, float v_min, float v_max) { return ImGui::SliderFloat2(label, v, v_min, v_max); },
		[](const char* label, float* v, float v_min, float v_max, const char* format) { return ImGui::SliderFloat2(label, v, v_min, v_max, format); },
		[](const char* label, float* v, float v_min, float v_max, const char* format, float power) { return ImGui::SliderFloat2(label, v, v_min, v_max, format, power); }
	));
	imguiLua.set_function("SliderFloat3", sol::overload(
		[](const char* label, float* v, float v_min, float v_max) { return ImGui::SliderFloat3(label, v, v_min, v_max); },
		[](const char* label, float* v, float v_min, float v_max, const char* format) { return ImGui::SliderFloat3(label, v, v_min, v_max, format); },
		[](const char* label, float* v, float v_min, float v_max, const char* format, float power) { return ImGui::SliderFloat3(label, v, v_min, v_max, format, power); }
	));
	imguiLua.set_function("SliderFloat4", sol::overload(
		[](const char* label, float* v, float v_min, float v_max) { return ImGui::SliderFloat4(label, v, v_min, v_max); },
		[](const char* label, float* v, float v_min, float v_max, const char* format) { return ImGui::SliderFloat4(label, v, v_min, v_max, format); },
		[](const char* label, float* v, float v_min, float v_max, const char* format, float power) { return ImGui::SliderFloat4(label, v, v_min, v_max, format, power); }
	));
	imguiLua.set_function("SliderAngle", sol::overload(
		[](const char* label, float* v_rad) { return ImGui::SliderAngle(label, v_rad); },
		[](const char* label, float* v_rad, float v_degrees_min) { return ImGui::SliderAngle(label, v_rad, v_degrees_min); },
		[](const char* label, float* v_rad, float v_degrees_min, float v_degrees_max) { return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max); },
		[](const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format) { return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format); }
	));
	imguiLua.set_function("SliderInt", sol::overload(
		[](const char* label, int* v, int v_min, int v_max) { return ImGui::SliderInt(label, v, v_min, v_max); },
		[](const char* label, int* v, int v_min, int v_max, const char* format) { return ImGui::SliderInt(label, v, v_min, v_max, format); }
	));
	imguiLua.set_function("SliderInt2", sol::overload(
		[](const char* label, int* v, int v_min, int v_max) { return ImGui::SliderInt2(label, v, v_min, v_max); },
		[](const char* label, int* v, int v_min, int v_max, const char* format) { return ImGui::SliderInt2(label, v, v_min, v_max, format); }
	));
	imguiLua.set_function("SliderInt3", sol::overload(
		[](const char* label, int* v, int v_min, int v_max) { return ImGui::SliderInt3(label, v, v_min, v_max); },
		[](const char* label, int* v, int v_min, int v_max, const char* format) { return ImGui::SliderInt3(label, v, v_min, v_max, format); }
	));
	imguiLua.set_function("SliderInt4", sol::overload(
		[](const char* label, int* v, int v_min, int v_max) { return ImGui::SliderInt4(label, v, v_min, v_max); },
		[](const char* label, int* v, int v_min, int v_max, const char* format) { return ImGui::SliderInt4(label, v, v_min, v_max, format); }
	));
	imguiLua.set_function("SliderScalar", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max) { return ImGui::SliderScalar(label, data_type, p_data, p_min, p_max); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format) { return ImGui::SliderScalar(label, data_type, p_data, p_min, p_max, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power) { return ImGui::SliderScalar(label, data_type, p_data, p_min, p_max, format, power); }
	));
	imguiLua.set_function("SliderScalarN", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max) { return ImGui::SliderScalarN(label, data_type, p_data, components, p_min, p_max); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format) { return ImGui::SliderScalarN(label, data_type, p_data, components, p_min, p_max, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format, float power) { return ImGui::SliderScalarN(label, data_type, p_data, components, p_min, p_max, format, power); }
	));
	imguiLua.set_function("VSliderFloat", sol::overload(
		[](const char* label, const ImVec2& size, float* v, float v_min, float v_max) { return ImGui::VSliderFloat(label, size, v, v_min, v_max); },
		[](const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format) { return ImGui::VSliderFloat(label, size, v, v_min, v_max, format); },
		[](const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format, float power) { return ImGui::VSliderFloat(label, size, v, v_min, v_max, format, power); }
	));
	imguiLua.set_function("VSliderInt", sol::overload(
		[](const char* label, const ImVec2& size, int* v, int v_min, int v_max) { return ImGui::VSliderInt(label, size, v, v_min, v_max); },
		[](const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format) { return ImGui::VSliderInt(label, size, v, v_min, v_max, format); }
	));
	imguiLua.set_function("VSliderScalar", sol::overload(
		[](const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max) { return ImGui::VSliderScalar(label, size, data_type, p_data, p_min, p_max); },
		[](const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format) { return ImGui::VSliderScalar(label, size, data_type, p_data, p_min, p_max, format); },
		[](const char* label, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power) { return ImGui::VSliderScalar(label, size, data_type, p_data, p_min, p_max, format, power); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Inputs with Keyboard
	imguiLua.set_function("InputText", sol::overload(
		[](const char* label, void* buf, int buf_size) { return ImGui::InputText(label, (char*)buf, buf_size); },
		[](const char* label, void* buf, int buf_size, ImGuiInputTextFlags flags) { return ImGui::InputText(label, (char*)buf, buf_size, flags); }
	));
	imguiLua.set_function("InputTextMultiline", sol::overload(
		[](const char* label, void* buf, int buf_size) { return ImGui::InputTextMultiline(label, (char*)buf, buf_size); },
		[](const char* label, void* buf, int buf_size, const ImVec2& size) { return ImGui::InputTextMultiline(label, (char*)buf, buf_size, size); },
		[](const char* label, void* buf, int buf_size, const ImVec2& size, ImGuiInputTextFlags flags) { return ImGui::InputTextMultiline(label, (char*)buf, buf_size, size, flags); }
	));
	imguiLua.set_function("InputTextWithHint", sol::overload(
		[](const char* label, const char* hint, void* buf, int buf_size) { return ImGui::InputTextWithHint(label, hint, (char*)buf, buf_size); },
		[](const char* label, const char* hint, void* buf, int buf_size, ImGuiInputTextFlags flags) { return ImGui::InputTextWithHint(label, hint, (char*)buf, buf_size, flags); }
	));
	imguiLua.set_function("InputFloat", sol::overload(
		[](const char* label, float* v) { return ImGui::InputFloat(label, v); },
		[](const char* label, float* v, float step) { return ImGui::InputFloat(label, v, step); },
		[](const char* label, float* v, float step, float step_fast) { return ImGui::InputFloat(label, v, step, step_fast); },
		[](const char* label, float* v, float step, float step_fast, const char* format) { return ImGui::InputFloat(label, v, step, step_fast, format); },
		[](const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputFloat(label, v, step, step_fast, format, flags); }
	));
	imguiLua.set_function("InputFloat2", sol::overload(
		[](const char* label, float* v) { return ImGui::InputFloat2(label, v); },
		[](const char* label, float* v, const char* format) { return ImGui::InputFloat2(label, v, format); },
		[](const char* label, float* v, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputFloat2(label, v, format, flags); }
	));
	imguiLua.set_function("InputFloat3", sol::overload(
		[](const char* label, float* v) { return ImGui::InputFloat3(label, v); },
		[](const char* label, float* v, const char* format) { return ImGui::InputFloat3(label, v, format); },
		[](const char* label, float* v, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputFloat3(label, v, format, flags); }
	));
	imguiLua.set_function("InputFloat4", sol::overload(
		[](const char* label, float* v) { return ImGui::InputFloat4(label, v); },
		[](const char* label, float* v, const char* format) { return ImGui::InputFloat4(label, v, format); },
		[](const char* label, float* v, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputFloat4(label, v, format, flags); }
	));
	imguiLua.set_function("InputInt", sol::overload(
		[](const char* label, int* v) { return ImGui::InputInt(label, v); },
		[](const char* label, int* v, int step) { return ImGui::InputInt(label, v, step); },
		[](const char* label, int* v, int step, int step_fast) { return ImGui::InputInt(label, v, step, step_fast); },
		[](const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags) { return ImGui::InputInt(label, v, step, step_fast, flags); }
	));
	imguiLua.set_function("InputInt2", sol::overload(
		[](const char* label, int* v) { return ImGui::InputInt2(label, v); },
		[](const char* label, int* v, ImGuiInputTextFlags flags) { return ImGui::InputInt2(label, v, flags); }
	));
	imguiLua.set_function("InputInt3", sol::overload(
		[](const char* label, int* v) { return ImGui::InputInt3(label, v); },
		[](const char* label, int* v, ImGuiInputTextFlags flags) { return ImGui::InputInt3(label, v, flags); }
	));
	imguiLua.set_function("InputInt4", sol::overload(
		[](const char* label, int* v) { return ImGui::InputInt4(label, v); },
		[](const char* label, int* v, ImGuiInputTextFlags flags) { return ImGui::InputInt4(label, v, flags); }
	));
	imguiLua.set_function("InputDouble", sol::overload(
		[](const char* label, double* v) { return ImGui::InputDouble(label, v); },
		[](const char* label, double* v, double step) { return ImGui::InputDouble(label, v, step); },
		[](const char* label, double* v, double step, double step_fast) { return ImGui::InputDouble(label, v, step, step_fast); },
		[](const char* label, double* v, double step, double step_fast, const char* format) { return ImGui::InputDouble(label, v, step, step_fast, format); },
		[](const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputDouble(label, v, step, step_fast, format, flags); }
	));
	imguiLua.set_function("InputScalar", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data) { return ImGui::InputScalar(label, data_type, p_data); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_step) { return ImGui::InputScalar(label, data_type, p_data, p_step); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast) { return ImGui::InputScalar(label, data_type, p_data, p_step, p_step_fast); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format) { return ImGui::InputScalar(label, data_type, p_data, p_step, p_step_fast, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputScalar(label, data_type, p_data, p_step, p_step_fast, format, flags); }
	));
	imguiLua.set_function("InputScalarN", sol::overload(
		[](const char* label, ImGuiDataType data_type, void* p_data, int components) { return ImGui::InputScalarN(label, data_type, p_data, components); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step) { return ImGui::InputScalarN(label, data_type, p_data, components, p_step); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast) { return ImGui::InputScalarN(label, data_type, p_data, components, p_step, p_step_fast); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast, const char* format) { return ImGui::InputScalarN(label, data_type, p_data, components, p_step, p_step_fast, format); },
		[](const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags) { return ImGui::InputScalarN(label, data_type, p_data, components, p_step, p_step_fast, format, flags); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Color Editor/Picker
	imguiLua.set_function("ColorEdit3", sol::overload(
		[](const char* label, float* col) { return ImGui::ColorEdit3(label, col); },
		[](const char* label, float* col, ImGuiColorEditFlags flags) { return ImGui::ColorEdit3(label, col, flags); }
	));
	imguiLua.set_function("ColorEdit4", sol::overload(
		[](const char* label, float* col) { return ImGui::ColorEdit4(label, col); },
		[](const char* label, float* col, ImGuiColorEditFlags flags) { return ImGui::ColorEdit4(label, col, flags); }
	));
	imguiLua.set_function("ColorPicker3", sol::overload(
		[](const char* label, float* col) { return ImGui::ColorPicker3(label, col); },
		[](const char* label, float* col, ImGuiColorEditFlags flags) { return ImGui::ColorPicker3(label, col, flags); }
	));
	imguiLua.set_function("ColorPicker4", sol::overload(
		[](const char* label, float* col) { return ImGui::ColorPicker4(label, col); },
		[](const char* label, float* col, ImGuiColorEditFlags flags) { return ImGui::ColorPicker4(label, col, flags); },
		[](const char* label, float* col, ImGuiColorEditFlags flags, const float* ref_col) { return ImGui::ColorPicker4(label, col, flags, ref_col); }
	));
	imguiLua.set_function("ColorButton", sol::overload(
		[](const char* desc_id, const ImVec4& col) { return ImGui::ColorButton(desc_id, col); },
		[](const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags) { return ImGui::ColorButton(desc_id, col, flags); },
		[](const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, ImVec2 size) { return ImGui::ColorButton(desc_id, col, flags, size); }
	));
	imguiLua.set_function("SetColorEditOptions", ImGui::SetColorEditOptions);
#pragma endregion

#pragma region [DONE] REGION: Widgets - Trees		[TODO: Maybe add variadic args support]
	imguiLua.set_function("TreeNode", sol::overload(
		[](const char* label) { return ImGui::TreeNode(label); },
		[](const char* str_id, const char* fmt) { return ImGui::TreeNode(str_id, fmt); },
		[](const void* ptr_id, const char* fmt) { return ImGui::TreeNode(ptr_id, fmt); }
	));
	imguiLua.set_function("TreeNodeEx", sol::overload(
		[](const char* label) { return ImGui::TreeNodeEx(label); },
		[](const char* label, ImGuiTreeNodeFlags flags) { return ImGui::TreeNodeEx(label, flags); },
		[](const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt) { return ImGui::TreeNodeEx(str_id, flags, fmt); },
		[](const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt) { return ImGui::TreeNodeEx(ptr_id, flags, fmt); }
	));
	imguiLua.set_function("TreePush", sol::overload(
		[](const char* str_id) { return ImGui::TreePush(str_id); },
		[](const void* ptr_id) { return ImGui::TreePush(ptr_id); }
	));
	imguiLua.set_function("TreePop", ImGui::TreePop);
	imguiLua.set_function("GetTreeNodeToLabelSpacing", ImGui::GetTreeNodeToLabelSpacing);
	imguiLua.set_function("CollapsingHeader", sol::overload(
		[](const char* label) { return ImGui::CollapsingHeader(label); },
		[](const char* label, ImGuiTreeNodeFlags flags) { return ImGui::CollapsingHeader(label, flags); },
		[](const char* label, bool* p_open, ImGuiTreeNodeFlags flags) { return ImGui::CollapsingHeader(label, p_open, flags); }
	));
	imguiLua.set_function("SetNextItemOpen", sol::overload(
		[](bool is_open) { return ImGui::SetNextItemOpen(is_open); },
		[](bool is_open, ImGuiCond cond) { return ImGui::SetNextItemOpen(is_open, cond); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Selectables
	imguiLua.set_function("Selectable", sol::overload(
		[](const char* label) { return ImGui::Selectable(label); },
		[](const char* label, bool selected) { return ImGui::Selectable(label, selected); },
		[](const char* label, bool selected, ImGuiSelectableFlags flags) { return ImGui::Selectable(label, selected, flags); },
		[](const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size) { return ImGui::Selectable(label, selected, flags, size); },
		[](const char* label, bool* p_selected) { return ImGui::Selectable(label, p_selected); },
		[](const char* label, bool* p_selected, ImGuiSelectableFlags flags) { return ImGui::Selectable(label, p_selected, flags); },
		[](const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size) { return ImGui::Selectable(label, p_selected, flags, size); }
	));
#pragma endregion

#pragma region [TODO] REGION: Widgets - List Boxes
#pragma endregion

#pragma region [DONE] REGION: Widgets - Data Plotting	[TODO: Maybe add void* variant with values_getter callback]
	imguiLua.set_function("PlotLines", sol::overload(
		[](const char* label, const float* values, int values_count) { return ImGui::PlotLines(label, values, values_count); },
		[](const char* label, const float* values, int values_count, int values_offset) { return ImGui::PlotLines(label, values, values_count, values_offset); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text) { return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min) { return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text, scale_min); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max) { return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text, scale_min, scale_max); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size) { return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride) { return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size, stride); }
	)); 
	imguiLua.set_function("PlotHistogram", sol::overload(
		[](const char* label, const float* values, int values_count) { return ImGui::PlotHistogram(label, values, values_count); },
		[](const char* label, const float* values, int values_count, int values_offset) { return ImGui::PlotHistogram(label, values, values_count, values_offset); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text) { return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min) { return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text, scale_min); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max) { return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text, scale_min, scale_max); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size) { return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size); },
		[](const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride) { return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size, stride); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Value
	imguiLua.set_function("Value", sol::overload(
		[](const char* prefix, bool b) { return ImGui::Value(prefix, b); },
		[](const char* prefix, int v) { return ImGui::Value(prefix, v); },
		[](const char* prefix, unsigned int v) { return ImGui::Value(prefix, v); },
		[](const char* prefix, float v) { return ImGui::Value(prefix, v); },
		[](const char* prefix, float v, const char* float_format) { return ImGui::Value(prefix, v, float_format); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Menus
	imguiLua.set_function("BeginMenuBar", ImGui::BeginMenuBar);
	imguiLua.set_function("EndMenuBar", ImGui::EndMenuBar);
	imguiLua.set_function("BeginMainMenuBar", ImGui::BeginMainMenuBar);
	imguiLua.set_function("EndMainMenuBar", ImGui::EndMainMenuBar);
	imguiLua.set_function("BeginMenu", sol::overload(
		[](const char* label) { return ImGui::BeginMenu(label); },
		[](const char* label, bool enabled) { return ImGui::BeginMenu(label, enabled); }
	));
	imguiLua.set_function("EndMenu", ImGui::EndMenu);
	imguiLua.set_function("MenuItem", sol::overload(
		[](const char* label) { return ImGui::MenuItem(label); },
		[](const char* label, const char* shortcut) { return ImGui::MenuItem(label, shortcut); },
		[](const char* label, const char* shortcut, bool selected) { return ImGui::MenuItem(label, shortcut, selected); },
		[](const char* label, const char* shortcut, bool selected, bool enabled) { return ImGui::MenuItem(label, shortcut, selected, enabled); },
		[](const char* label, const char* shortcut, bool* p_selected) { return ImGui::MenuItem(label, shortcut, p_selected); },
		[](const char* label, const char* shortcut, bool* p_selected, bool enabled) { return ImGui::MenuItem(label, shortcut, p_selected, enabled); }
	));
#pragma endregion

#pragma region [DONE] REGION: Widgets - Tooltips
	imguiLua.set_function("BeginTooltip", ImGui::BeginTooltip);
	imguiLua.set_function("EndTooltip", ImGui::EndTooltip);
	//imguiLua.set_function("SetTooltip", ImGui::SetTooltip);
#pragma endregion

#pragma region [DONE] REGION: Popups, Modals
  imguiLua.set_function("OpenPopup", sol::overload(
    [](const char* str_id) { return ImGui::OpenPopup(str_id); },
    [](const char* str_id, ImGuiPopupFlags popup_flags) { return ImGui::OpenPopup(str_id, popup_flags); },
    [](ImGuiID id) { return ImGui::OpenPopup(id); },
    [](ImGuiID id, ImGuiPopupFlags popup_flags) { return ImGui::OpenPopup(id, popup_flags); }
  ));
	imguiLua.set_function("BeginPopup", sol::overload(
		[](const char* str_id) { return ImGui::BeginPopup(str_id); },
		[](const char* str_id, ImGuiWindowFlags flags) { return ImGui::BeginPopup(str_id, flags); }
	));
	imguiLua.set_function("BeginPopupContextItem", sol::overload(
		[]() { return ImGui::BeginPopupContextItem(); },
		[](const char* str_id) { return ImGui::BeginPopupContextItem(str_id); },
		[](const char* str_id, ImGuiMouseButton mouse_button) { return ImGui::BeginPopupContextItem(str_id, mouse_button); }
	));
	imguiLua.set_function("BeginPopupContextWindow", sol::overload(
		[]() { return ImGui::BeginPopupContextWindow(); },
		[](const char* str_id) { return ImGui::BeginPopupContextWindow(str_id); },
		[](const char* str_id, ImGuiMouseButton mouse_button) { return ImGui::BeginPopupContextWindow(str_id, mouse_button); }
	));
	imguiLua.set_function("BeginPopupContextVoid", sol::overload(
		[]() { return ImGui::BeginPopupContextVoid(); },
		[](const char* str_id) { return ImGui::BeginPopupContextVoid(str_id); },
		[](const char* str_id, ImGuiMouseButton mouse_button) { return ImGui::BeginPopupContextVoid(str_id, mouse_button); }
	));
	imguiLua.set_function("BeginPopupModal", ImGui::BeginPopupModal);
	imguiLua.set_function("EndPopup", ImGui::EndPopup);
	imguiLua.set_function("OpenPopupOnItemClick", sol::overload(
		[]() { return ImGui::OpenPopupOnItemClick(); },
		[](const char* str_id) { return ImGui::OpenPopupOnItemClick(str_id); },
		[](const char* str_id, ImGuiMouseButton mouse_button) { return ImGui::OpenPopupOnItemClick(str_id, mouse_button); }
	));
	imguiLua.set_function("IsPopupOpen", ImGui::IsPopupOpen);
	imguiLua.set_function("CloseCurrentPopup", ImGui::CloseCurrentPopup);
#pragma endregion

#pragma region [DONE] REGION: Columns
	imguiLua.set_function("Columns", sol::overload(
		[]() { return ImGui::Columns(); },
		[](int count) { return ImGui::Columns(count); },
		[](int count, const char* id) { return ImGui::Columns(count, id); },
		[](int count, const char* id, bool border) { return ImGui::Columns(count, id, border); }
	));
	imguiLua.set_function("NextColumn", ImGui::NextColumn);
	imguiLua.set_function("GetColumnIndex", ImGui::GetColumnIndex);
	imguiLua.set_function("GetColumnWidth", sol::overload(
		[]() { return ImGui::GetColumnWidth(); },
		[](int column_index) { return ImGui::GetColumnWidth(column_index); }
	));
	imguiLua.set_function("SetColumnWidth", ImGui::SetColumnWidth);
	imguiLua.set_function("GetColumnOffset", sol::overload(
		[]() { return ImGui::GetColumnOffset(); },
		[](int column_index) { return ImGui::GetColumnOffset(column_index); }
	));
	imguiLua.set_function("SetColumnOffset", ImGui::SetColumnOffset);
	imguiLua.set_function("GetColumnsCount", ImGui::GetColumnsCount);
#pragma endregion

#pragma region [DONE] REGION: Clipping
	imguiLua.set_function("PushClipRect", ImGui::PushClipRect);
	imguiLua.set_function("PopClipRect", ImGui::PopClipRect);
#pragma endregion

#pragma region [DONE] REGION: Focus, Activation
	imguiLua.set_function("SetItemDefaultFocus", ImGui::SetItemDefaultFocus);
	imguiLua.set_function("SetKeyboardFocusHere", sol::overload(
		[]() { return ImGui::SetKeyboardFocusHere(); },
		[](int offset) { return ImGui::SetKeyboardFocusHere(offset); }
	));
#pragma endregion

	imguiLua.set_function("Warning", sol::resolve<void(const char*)>(&ImGui::Warning));
	imguiLua.set_function("Error", sol::resolve<void(const char*)>(&ImGui::Error));
	

	//TODO Utilities
}
#else
void LuaScriptHost::injectImGui(sol::state& luaSol) {
}
#endif
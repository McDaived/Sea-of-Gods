#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "blur_dx11.hpp"
#include "animations.hpp"

#include <string>
#include <vector>
#include <functional>


#define font( n ) ImGui::GetIO( ).Fonts->Fonts[ n ]

#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

struct tab_t {
	const char* m_icon;
	const char* m_name;

	int cur_subtab = 0;
	int next_subtab = 0;
	std::vector< const char* > m_subtabs;
	std::vector< std::function< void( ) > > pages;

	tab_t( const char* icon, const char* name, std::vector< const char* > subtabs ) {
		m_icon = icon;
		m_name = name;
		m_subtabs = subtabs;
	}
};

namespace ui {
	inline ImVec2 size{ 984, 620 };

	inline float anim = 0.f;
	inline float anim_dst = 1.f;
	inline float content_anim = 0.f;
	inline float content_anim_dst = 1.f;
	void handle_anims( );

	void render_page( );
	void add_page( int tab_idx, std::function< void( ) > c );

	void rotate_start( );
    ImVec2 rotation_center( );
    void rotate_end( float rad, ImVec2 center );

	inline int cur_tab = 0;
	inline int next_tab = 0;
	inline std::vector< tab_t > tabs;
	int add_tab( const char* icon, const char* name, std::vector< const char* > subtabs = { } );

	void tab( int num );
	void subtab( int num, int tab_num );

	void begin_child( const char* name, ImVec2 size );
	void end_child( );

	bool color( const char* name, float col[4], bool alpha = true );
}

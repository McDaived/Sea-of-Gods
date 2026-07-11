#pragma once

#include <d3d11.h>
#include "../ImGui/imgui.h"


namespace imgui_blur
{

    void set_device( ID3D11Device* device, ID3D11DeviceContext* context ) noexcept;

    void new_frame( ID3D11RenderTargetView* backbuffer_rtv ) noexcept;

    void create_blur( ImDrawList* d, ImVec2 min, ImVec2 max, ImColor col = ImColor( 255, 255, 255 ), float rounding = 0.f, ImDrawFlags round_flags = ImDrawFlags_RoundCornersAll ) noexcept;
}

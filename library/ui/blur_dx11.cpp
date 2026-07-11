#include "blur_dx11.hpp"
#include "../ImGui/imgui_internal.h"

#include <d3dcompiler.h>
#include <wrl/client.h>
#include <cstring>

#pragma comment(lib, "d3dcompiler")

using Microsoft::WRL::ComPtr;

namespace {

    const char* kShaderSource = R"(
struct VS_INPUT { float2 pos : POSITION; float2 uv : TEXCOORD0; float4 col : COLOR0; };
struct PS_INPUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output;
    output.pos = float4( input.pos.x, input.pos.y, 0.0f, 1.0f );
    output.uv = input.uv;
    return output;
}

Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);

cbuffer BlurConstants : register(b0)
{
    float2 step_uv; // one-texel step in the blur direction, in UV units (0 in the other axis)
    float2 _pad;
};

float4 PSBlur( PS_INPUT input ) : SV_TARGET
{
    float2 o1 = step_uv * 1.3846153846f;
    float2 o2 = step_uv * 3.2307692308f;
    float3 color  = tex0.Sample( samp0, input.uv ).rgb           * 0.2270270270f;
    color += tex0.Sample( samp0, input.uv + o1 ).rgb * 0.3162162162f;
    color += tex0.Sample( samp0, input.uv - o1 ).rgb * 0.3162162162f;
    color += tex0.Sample( samp0, input.uv + o2 ).rgb * 0.0702702703f;
    color += tex0.Sample( samp0, input.uv - o2 ).rgb * 0.0702702703f;
    return float4( color, 1.0f );
}

float4 PSCopy( PS_INPUT input ) : SV_TARGET
{
    return float4( tex0.Sample( samp0, input.uv ).rgb, 1.0f );
}
)";

    struct BlurConstants { float step_uv[2]; float pad[2]; };

    class c_blur_effect {
    public:
        static c_blur_effect& instance( ) noexcept {
            static c_blur_effect inst;
            return inst;
        }

        void set_device( ID3D11Device* dev, ID3D11DeviceContext* ctx ) noexcept {
            if ( device == dev && context == ctx )
                return;
            release_textures( );
            release_shaders( );
            device = dev;
            context = ctx;
        }

        void new_frame( ID3D11RenderTargetView* rtv ) noexcept {
            backbuffer_rtv = rtv;
        }

        void create_blur( ImDrawList* d, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawFlags round_flags ) noexcept {
            if ( !device || !context || !backbuffer_rtv )
                return;

            ensure_shaders( );
            if ( !vs || !ps_blur || !ps_copy )
                return;

            ImVec2 display_size = ImGui::GetIO( ).DisplaySize;
            if ( display_size.x <= 0 || display_size.y <= 0 )
                return;

            if ( !ensure_textures( ) )
                return;

            static constexpr int kBlurIterations = 4;
            static constexpr int kRefreshInterval = 2;
            bool doRefresh = force_refresh || ( ( refresh_counter++ % kRefreshInterval ) == 0 );

            if ( doRefresh ) {
                force_refresh = false;
                d->AddCallback( &s_begin, nullptr );
                d->AddCallback( &s_downsample, nullptr );
                d->AddImage( reinterpret_cast<ImTextureID>( srv_full.Get( ) ), { -1.f, -1.f }, { 1.f, 1.f } );
                for ( int i = 0; i < kBlurIterations; ++i ) {
                    d->AddCallback( &s_pass_x, nullptr );
                    d->AddImage( reinterpret_cast<ImTextureID>( srv_small[0].Get( ) ), { -1.f, -1.f }, { 1.f, 1.f } );
                    d->AddCallback( &s_pass_y, nullptr );
                    d->AddImage( reinterpret_cast<ImTextureID>( srv_small[1].Get( ) ), { -1.f, -1.f }, { 1.f, 1.f } );
                }
                d->AddCallback( &s_end, nullptr );
                d->AddCallback( ImDrawCallback_ResetRenderState, nullptr );
            }

            ImVec2 uv0 = { min.x / display_size.x, min.y / display_size.y };
            ImVec2 uv1 = { max.x / display_size.x, max.y / display_size.y };
            float alpha = GImGui ? GImGui->Style.Alpha : 1.f;
            ImColor tint( col.Value.x, col.Value.y, col.Value.z, col.Value.w * alpha );
            d->AddImageRounded( reinterpret_cast<ImTextureID>( srv_small[0].Get( ) ), min, max, uv0, uv1, tint, rounding, round_flags );
        }

    private:
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        ID3D11RenderTargetView* backbuffer_rtv = nullptr;

        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11PixelShader> ps_blur;
        ComPtr<ID3D11PixelShader> ps_copy;
        ComPtr<ID3D11InputLayout> input_layout;
        ComPtr<ID3D11Buffer> constants;
        ComPtr<ID3D11SamplerState> sampler;
        ComPtr<ID3D11RasterizerState> rasterizer_noscissor;
        ComPtr<ID3D11BlendState> blend_opaque;

        ComPtr<ID3D11Texture2D> tex_full;
        ComPtr<ID3D11ShaderResourceView> srv_full;

        ComPtr<ID3D11Texture2D> tex_small[2];
        ComPtr<ID3D11RenderTargetView> rtv_small[2];
        ComPtr<ID3D11ShaderResourceView> srv_small[2];

        UINT full_w = 0, full_h = 0;
        DXGI_FORMAT full_fmt = DXGI_FORMAT_UNKNOWN;
        UINT small_w = 0, small_h = 0;
        static constexpr UINT kDownsample = 9;

        int refresh_counter = 0;
        bool force_refresh = true;

        ComPtr<ID3D11RasterizerState> saved_rs;
        ComPtr<ID3D11BlendState> saved_bs;
        FLOAT saved_blend_factor[4] = {};
        UINT saved_sample_mask = 0;
        D3D11_VIEWPORT saved_viewport = {};
        UINT saved_viewport_count = 1;

        c_blur_effect( ) = default;

        void release_textures( ) noexcept {
            tex_full.Reset( ); srv_full.Reset( );
            for ( int i = 0; i < 2; ++i ) { tex_small[i].Reset( ); rtv_small[i].Reset( ); srv_small[i].Reset( ); }
            full_w = full_h = small_w = small_h = 0;
            full_fmt = DXGI_FORMAT_UNKNOWN;
        }

        void release_shaders( ) noexcept {
            vs.Reset( ); ps_blur.Reset( ); ps_copy.Reset( ); input_layout.Reset( );
            constants.Reset( ); sampler.Reset( ); rasterizer_noscissor.Reset( ); blend_opaque.Reset( );
        }

        void ensure_shaders( ) noexcept {
            if ( vs && ps_blur && ps_copy )
                return;

            static ComPtr<ID3DBlob> vs_blob, ps_blur_blob, ps_copy_blob;
            if ( !vs_blob || !ps_blur_blob || !ps_copy_blob ) {
                ComPtr<ID3DBlob> err;
                UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

                if ( FAILED( D3DCompile( kShaderSource, strlen( kShaderSource ), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", flags, 0, &vs_blob, &err ) ) )
                    return;
                if ( FAILED( D3DCompile( kShaderSource, strlen( kShaderSource ), nullptr, nullptr, nullptr, "PSBlur", "ps_4_0", flags, 0, &ps_blur_blob, &err ) ) )
                    return;
                if ( FAILED( D3DCompile( kShaderSource, strlen( kShaderSource ), nullptr, nullptr, nullptr, "PSCopy", "ps_4_0", flags, 0, &ps_copy_blob, &err ) ) )
                    return;
            }

            if ( FAILED( device->CreateVertexShader( vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), nullptr, &vs ) ) )
                return;
            if ( FAILED( device->CreatePixelShader( ps_blur_blob->GetBufferPointer( ), ps_blur_blob->GetBufferSize( ), nullptr, &ps_blur ) ) )
                return;
            if ( FAILED( device->CreatePixelShader( ps_copy_blob->GetBufferPointer( ), ps_copy_blob->GetBufferSize( ), nullptr, &ps_copy ) ) )
                return;

            D3D11_INPUT_ELEMENT_DESC layout[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF( ImDrawVert, pos ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF( ImDrawVert, uv ),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF( ImDrawVert, col ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            device->CreateInputLayout( layout, 3, vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), &input_layout );

            D3D11_BUFFER_DESC cb_desc = {};
            cb_desc.ByteWidth = sizeof( BlurConstants );
            cb_desc.Usage = D3D11_USAGE_DYNAMIC;
            cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            device->CreateBuffer( &cb_desc, nullptr, &constants );

            D3D11_SAMPLER_DESC samp_desc = {};
            samp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samp_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            device->CreateSamplerState( &samp_desc, &sampler );

            D3D11_RASTERIZER_DESC rs_desc = {};
            rs_desc.FillMode = D3D11_FILL_SOLID;
            rs_desc.CullMode = D3D11_CULL_NONE;
            rs_desc.DepthClipEnable = TRUE;
            rs_desc.ScissorEnable = FALSE;
            device->CreateRasterizerState( &rs_desc, &rasterizer_noscissor );

            D3D11_BLEND_DESC bs_desc = {};
            bs_desc.RenderTarget[0].BlendEnable = FALSE;
            bs_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            device->CreateBlendState( &bs_desc, &blend_opaque );
        }

        bool ensure_textures( ) noexcept {
            ComPtr<ID3D11Resource> backbuffer_res;
            backbuffer_rtv->GetResource( &backbuffer_res );
            ComPtr<ID3D11Texture2D> backbuffer_tex;
            if ( FAILED( backbuffer_res.As( &backbuffer_tex ) ) )
                return false;

            D3D11_TEXTURE2D_DESC bb_desc = {};
            backbuffer_tex->GetDesc( &bb_desc );
            if ( bb_desc.SampleDesc.Count != 1 )
                return false; // MSAA backbuffers aren't supported by this simple CopyResource-based path.

            UINT sw = ( bb_desc.Width / kDownsample ) < 1 ? 1 : bb_desc.Width / kDownsample;
            UINT sh = ( bb_desc.Height / kDownsample ) < 1 ? 1 : bb_desc.Height / kDownsample;

            if ( tex_full && full_w == bb_desc.Width && full_h == bb_desc.Height && full_fmt == bb_desc.Format
                 && tex_small[0] && small_w == sw && small_h == sh )
                return true;

            release_textures( );

            D3D11_TEXTURE2D_DESC full_desc = {};
            full_desc.Width = bb_desc.Width;
            full_desc.Height = bb_desc.Height;
            full_desc.MipLevels = 1;
            full_desc.ArraySize = 1;
            full_desc.Format = bb_desc.Format;
            full_desc.SampleDesc.Count = 1;
            full_desc.Usage = D3D11_USAGE_DEFAULT;
            full_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            if ( FAILED( device->CreateTexture2D( &full_desc, nullptr, &tex_full ) ) )
                return false;
            if ( FAILED( device->CreateShaderResourceView( tex_full.Get( ), nullptr, &srv_full ) ) )
                return false;

            D3D11_TEXTURE2D_DESC small_desc = full_desc;
            small_desc.Width = sw;
            small_desc.Height = sh;
            small_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            for ( int i = 0; i < 2; ++i ) {
                if ( FAILED( device->CreateTexture2D( &small_desc, nullptr, &tex_small[i] ) ) )
                    return false;
                if ( FAILED( device->CreateShaderResourceView( tex_small[i].Get( ), nullptr, &srv_small[i] ) ) )
                    return false;
                if ( FAILED( device->CreateRenderTargetView( tex_small[i].Get( ), nullptr, &rtv_small[i] ) ) )
                    return false;
            }

            full_w = bb_desc.Width; full_h = bb_desc.Height; full_fmt = bb_desc.Format;
            small_w = sw; small_h = sh;
            force_refresh = true;
            return true;
        }

        void set_step( float x, float y ) noexcept {
            D3D11_MAPPED_SUBRESOURCE mapped;
            if ( SUCCEEDED( context->Map( constants.Get( ), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) ) {
                BlurConstants* c = reinterpret_cast<BlurConstants*>( mapped.pData );
                c->step_uv[0] = x; c->step_uv[1] = y;
                c->pad[0] = c->pad[1] = 0.f;
                context->Unmap( constants.Get( ), 0 );
            }
        }

        void bind_common( ) noexcept {
            context->IASetInputLayout( input_layout.Get( ) );
            context->VSSetShader( vs.Get( ), nullptr, 0 );
            context->PSSetSamplers( 0, 1, sampler.GetAddressOf( ) );
            context->RSSetState( rasterizer_noscissor.Get( ) );
            const float blend_factor[4] = { 0, 0, 0, 0 };
            context->OMSetBlendState( blend_opaque.Get( ), blend_factor, 0xffffffff );
            context->PSSetConstantBuffers( 0, 1, constants.GetAddressOf( ) );
        }

        void _begin( ) noexcept {
            saved_viewport_count = 1;
            context->RSGetViewports( &saved_viewport_count, &saved_viewport );
            context->RSGetState( &saved_rs );
            context->OMGetBlendState( &saved_bs, saved_blend_factor, &saved_sample_mask );

            ComPtr<ID3D11Resource> backbuffer_res;
            backbuffer_rtv->GetResource( &backbuffer_res );
            ComPtr<ID3D11Texture2D> backbuffer_tex;
            if ( SUCCEEDED( backbuffer_res.As( &backbuffer_tex ) ) )
                context->CopyResource( tex_full.Get( ), backbuffer_tex.Get( ) );

            bind_common( );
        }

        void _downsample( ) noexcept {
            D3D11_VIEWPORT vp = { 0.f, 0.f, (float)small_w, (float)small_h, 0.f, 1.f };
            context->RSSetViewports( 1, &vp );
            ID3D11RenderTargetView* rt = rtv_small[0].Get( );
            context->OMSetRenderTargets( 1, &rt, nullptr );
            ID3D11ShaderResourceView* srv = srv_full.Get( );
            context->PSSetShaderResources( 0, 1, &srv );
            context->PSSetShader( ps_copy.Get( ), nullptr, 0 );
        }

        void _pass_x( ) noexcept {
            D3D11_VIEWPORT vp = { 0.f, 0.f, (float)small_w, (float)small_h, 0.f, 1.f };
            context->RSSetViewports( 1, &vp );
            ID3D11RenderTargetView* rt = rtv_small[1].Get( );
            context->OMSetRenderTargets( 1, &rt, nullptr );
            ID3D11ShaderResourceView* srv = srv_small[0].Get( );
            context->PSSetShaderResources( 0, 1, &srv );
            context->PSSetShader( ps_blur.Get( ), nullptr, 0 );
            set_step( 1.0f / (float)small_w, 0.0f );
        }

        void _pass_y( ) noexcept {
            D3D11_VIEWPORT vp = { 0.f, 0.f, (float)small_w, (float)small_h, 0.f, 1.f };
            context->RSSetViewports( 1, &vp );
            ID3D11RenderTargetView* rt = rtv_small[0].Get( );
            context->OMSetRenderTargets( 1, &rt, nullptr );
            ID3D11ShaderResourceView* srv = srv_small[1].Get( );
            context->PSSetShaderResources( 0, 1, &srv );
            context->PSSetShader( ps_blur.Get( ), nullptr, 0 );
            set_step( 0.0f, 1.0f / (float)small_h );
        }

        void _end( ) noexcept {
            ID3D11ShaderResourceView* null_srv = nullptr;
            context->PSSetShaderResources( 0, 1, &null_srv );
            context->OMSetRenderTargets( 1, &backbuffer_rtv, nullptr );
            context->RSSetViewports( saved_viewport_count ? saved_viewport_count : 1, &saved_viewport );
            context->RSSetState( saved_rs.Get( ) );
            context->OMSetBlendState( saved_bs.Get( ), saved_blend_factor, saved_sample_mask );
            saved_rs.Reset( );
            saved_bs.Reset( );
        }

        static void s_begin( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._begin( ); }
        static void s_downsample( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._downsample( ); }
        static void s_pass_x( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._pass_x( ); }
        static void s_pass_y( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._pass_y( ); }
        static void s_end( const ImDrawList*, const ImDrawCmd* ) noexcept { instance( )._end( ); }
    };

} 

void imgui_blur::set_device( ID3D11Device* device, ID3D11DeviceContext* context ) noexcept {
    c_blur_effect::instance( ).set_device( device, context );
}

void imgui_blur::new_frame( ID3D11RenderTargetView* backbuffer_rtv ) noexcept {
    c_blur_effect::instance( ).new_frame( backbuffer_rtv );
}

void imgui_blur::create_blur( ImDrawList* d, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawFlags round_flags ) noexcept {
    c_blur_effect::instance( ).create_blur( d, min, max, col, rounding, round_flags );
}

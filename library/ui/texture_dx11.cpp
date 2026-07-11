#include "texture_dx11.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "../stb/stb_image.h"

bool CreateTextureFromMemory(
    ID3D11Device* device,
    const unsigned char* data,
    size_t data_size,
    ID3D11ShaderResourceView** out_srv,
    int* out_width,
    int* out_height )
{
    if ( !device || !data || data_size == 0 || !out_srv )
        return false;

    int width = 0, height = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory( data, static_cast<int>( data_size ), &width, &height, &channels, 4 );
    if ( !pixels )
        return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = static_cast<UINT>( width );
    desc.Height = static_cast<UINT>( height );
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem = pixels;
    sub.SysMemPitch = static_cast<UINT>( width ) * 4;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D( &desc, &sub, &texture );
    stbi_image_free( pixels );
    if ( FAILED( hr ) || !texture )
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView( texture, &srv_desc, out_srv );
    texture->Release( );
    if ( FAILED( hr ) )
        return false;

    if ( out_width ) *out_width = width;
    if ( out_height ) *out_height = height;
    return true;
}

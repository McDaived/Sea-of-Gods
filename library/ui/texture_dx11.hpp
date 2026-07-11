#pragma once

#include <d3d11.h>
#include <cstddef>


bool CreateTextureFromMemory(
    ID3D11Device* device,
    const unsigned char* data,
    size_t data_size,
    ID3D11ShaderResourceView** out_srv,
    int* out_width = nullptr,
    int* out_height = nullptr );

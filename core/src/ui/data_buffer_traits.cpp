#include "ui/data_buffer_traits.hpp"

#include "ui/context.hpp"
#include "ui/vertex.hpp"

#include <gsl/gsl>

#include <cstdint>

namespace windower::ui
{

data_buffer_traits<vertex>::com_pointer data_buffer_traits<vertex>::allocate(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, std::size_t size) noexcept
{
    com_pointer result;
    d3d_device->CreateVertexBuffer(
        size * sizeof(vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, ::D3DPOOL_DEFAULT,
        result.put());
    return result;
}

void data_buffer_traits<vertex>::set_buffer(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept
{
    d3d_device->SetStreamSource(0, ptr, sizeof(vertex));
}

data_buffer_traits<std::uint16_t>::com_pointer
data_buffer_traits<std::uint16_t>::allocate(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, std::size_t size) noexcept
{
    com_pointer result;
    d3d_device->CreateIndexBuffer(
        size * sizeof(std::uint16_t), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        ::D3DFMT_INDEX16, ::D3DPOOL_DEFAULT, result.put());
    return result;
}

void data_buffer_traits<std::uint16_t>::set_buffer(
    gsl::not_null<::IDirect3DDevice8*> d3d_device, pointer ptr) noexcept
{
    d3d_device->SetIndices(ptr, 0);
}

}
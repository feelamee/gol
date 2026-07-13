#pragma once

#include <engine/fundamental.hpp>
#include <engine/mesh.hpp>

#include <glad/glad.h>

#include <string_view>

namespace gol::gl
{

namespace detail
{

struct resource
{
    // TODO! assume that gl methods always return non-zero id,
    // so we use 0 here as "invalid" state
    GLuint id = 0;

    bool has_value() const;

    operator GLuint() const;
};

}

struct shader : detail::resource {};

enum class stage : GLenum
{
    fragment = GL_FRAGMENT_SHADER,
    vertex = GL_VERTEX_SHADER,
};

void link(shader &);
void attach_source(shader &, stage, std::string_view source);
void attach_file(shader &, stage, std::filesystem::path const& path);

struct buffer : detail::resource
{
    GLenum target;
};
struct texture : detail::resource
{
    GLenum type;
};
struct vertex_array : detail::resource {};

struct model
{
    buffer vbo;
    buffer ebo;
    vertex_array vao;
    std::vector<texture> textures;
    sz indices_count;

    bool has_value() const;
};

bool from_file(model & m, u32 location, std::filesystem::path const& path);
bool from_file(texture &, GLenum type, std::filesystem::path const& path);

// TODO! split allocating buffer and uploading data to it
void from_mem(
    buffer &,
    GLenum usage,
    std::span<byte const> data,
    sz padding_size = 0 ///< allocate padding_size bytes more
);

template<sz Extent>
void from_mem(
    buffer & b,
    GLenum usage,
    std::span<byte const, Extent> data,
    sz padding_size = 0 ///< allocate padding_size bytes more
)
{
    return from_mem(b, usage, std::span<byte const>{ data }, padding_size);
}

void from_mem(
    buffer & b,
    GLenum usage,
    rng::range auto const& data,
    sz padding_size = 0 ///< allocate padding_size bytes more
)
{
    return from_mem(b, usage, std::as_bytes(std::span{ data }), padding_size);
}

void bind(shader const&);
void bind(vertex_array const&);
void bind(buffer const&);
void bind(u32 location, u32 unit, texture const&);
void bind(u32 location, model const&);
void bind(u32 location, mat4 const&);
void bind(u32 location, vec4 const&);
void bind(u32 location, uvec2 const&);
void bind(u32 binding, buffer const&);
void bind(vertex_array & vao, buffer const& b);

void unbind(shader const&);
void unbind(vertex_array const&);
void unbind(buffer const&);
void unbind(u32 binding, buffer const&);
void unbind(u32 unit, texture const&);
void unbind(model const&);
void unbind(buffer const&);

template<typename T>
struct bind_guard
{
    bind_guard(u32 location, T const & r)
        : resource(r)
    {
        bind(location, r);
    }

    bind_guard(T const & r)
        : resource(r)
    {
        bind(r);
    }

    ~bind_guard()
    {
        unbind(resource);
    }

    T const& resource;
};

void destroy(shader &);
void destroy(model &);
void destroy(buffer &);
void destroy(texture &);
void destroy(vertex_array &);

}

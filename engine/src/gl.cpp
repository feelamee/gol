#include <engine/gl.hpp>

#include <engine/sdl.hpp>
#include <engine/mesh.hpp>
#include <engine/image.hpp>
#include <engine/log.hpp>

#include <fstream>

namespace gol::gl
{

namespace
{

struct vao_info
{
    vertex_array vao;
    buffer vbo;
    buffer ebo;
};

static vao_info make_vao(mesh const& mesh, u32 location)
{
    vao_info m;

    glGenBuffers(1, &m.ebo.id);
    m.ebo.target = GL_ELEMENT_ARRAY_BUFFER;
    from_mem(m.ebo, GL_STATIC_DRAW, mesh.indices);

    glGenBuffers(1, &m.vbo.id);
    m.vbo.target = GL_ARRAY_BUFFER;
    from_mem(m.vbo, GL_STATIC_DRAW, mesh.vertices);

    glGenVertexArrays(1, &m.vao.id);
    glBindVertexArray(m.vao);

    bind(m.ebo);
    bind(m.vbo);

    sz offset = 0;
    for (auto [i, attr] : vs::enumerate_with<GLuint>(mesh.attribs))
    {
        glVertexAttribPointer(
            location + i,
            GLint(attrib_size(attr)),
            GL_FLOAT,
            GL_FALSE,
            GLsizei(mesh.bytestride()),
            (void*)offset
        );
        offset += attrib_bytesize(attr);

        glEnableVertexAttribArray(location + i);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return m;
}

static void make_vao(model & m, mesh const& mesh, u32 location)
{
    auto info = make_vao(mesh, location);
    m.vao = info.vao;
    m.vbo = info.vbo;
    m.ebo = info.ebo;
    m.indices_count = mesh.indices.size();
}

static texture make_tex(image const& img)
{
    texture tex{ .type = GL_TEXTURE_2D };
    glGenTextures(1, &tex.id);

    glBindTexture(tex.type, tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        GLsizei(img.size.x), GLsizei(img.size.y), 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    return tex;
}

static texture make_tex(cubemap const& cm)
{
    texture tex{ .type = GL_TEXTURE_CUBE_MAP };
    glGenTextures(1, &tex.id);

    glBindTexture(tex.type, tex);
    for(auto const& [i, img] : vs::enumerate_with<GLenum>(cm.data))
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  0, GL_RGB,
            GLsizei(img.size.x), GLsizei(img.size.y), 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data()
        );
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return tex;
}

}

bool detail::resource::has_value() const
{
    return id != 0;
}

detail::resource::operator GLuint() const
{
    assert(has_value());
    return id;
}

void link(shader & s)
{
    glLinkProgram(s);

    // for some reason linkage errors is not passed into glDebugMessageCallback
    GLint success;
    glGetProgramiv(s, GL_LINK_STATUS, &success);
    if (!success)
    {
        static GLchar log[1024];
        glGetProgramInfoLog(s, 1024, nullptr, log);
        log::err(log::category::gl, "program linkage error: {}\n", log);
    }
}

void attach_source(shader & s, stage stage, std::string_view source)
{
    auto data{ (GLchar const*)source.data() };
    auto const size{ GLint(source.size()) };

    GLuint shader{ glCreateShader(to_underlying(stage)) };
    glShaderSource(shader, 1, &data, &size);
    glCompileShader(shader);

    if (0)
    {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            log::err(log::category::gl, "shader compilation error: {}\n", log);
        }
    }

    glAttachShader(s, shader);
    glDeleteShader(shader);
}

void attach_file(shader & s, stage stage, std::filesystem::path const& path)
{
    sz size;
    void * data{ SDL_LoadFile(path.string().c_str(), &size) };
    if (!data)
    {
        sdl::log_error();
        return;
    }

    attach_source(s, stage, { (char *)data, size });
    SDL_free(data);
}


bool model::has_value() const
{
    return vao.has_value();
}

bool from_file(model & m, u32 location, std::filesystem::path const& path)
{
    namespace fs = std::filesystem;

    bool has_mesh{ false };
    image img;
    cubemap cubemap;
    mesh mesh;

    std::ifstream in{ path };
    std::string line;
    while (std::getline(in, line))
    {
        auto pos = line.find(' ');
        if (pos == line.npos)
            continue;

        std::string_view var{ line.data(), pos };
        std::string_view value{ line.data() + pos + 1 };
        fs::path abspath{ path.parent_path() / value };
        if (var == "mesh")
        {
            if (from_file(mesh, abspath))
            {
                make_vao(m, mesh, location);
                has_mesh = true;
            }
        }
        else if (var == "texture")
        {
            if (from_file(img, abspath))
                m.textures.push_back(make_tex(img));
        }
        else if (var == "cubemap")
        {
            if (from_file(cubemap, abspath))
                m.textures.push_back(make_tex(cubemap));
        }
    }

    if (!has_mesh)
    {
        for (texture t : m.textures)
            destroy(t);
    }

    // TODO! if have no textures insert default
    return has_mesh;
}

bool from_file(texture & tex, GLenum type, std::filesystem::path const& path)
{
    if (type == GL_TEXTURE_2D)
    {
        image img;
        if (!from_file(img, path))
            return false;

        tex = make_tex(img);
    }
    else if (type == GL_TEXTURE_CUBE_MAP)
    {
        cubemap cm;
        if (!from_file(cm, path))
            return false;

        tex = make_tex(cm);
    }

    return true;
}

void from_mem(
    buffer & buf,
    GLenum usage,
    std::span<byte const> data,
    sz padding_size
)
{
    bind_guard g(buf);
    glBufferData(
        buf.target,
        GLsizeiptr(data.size() * sizeof(byte) + padding_size),
        nullptr,
        usage
    );
    glBufferSubData(
        buf.target,
        0,
        GLsizeiptr(data.size() * sizeof(byte)),
        data.data()
    );
}

void bind(shader const& m)
{
    glUseProgram(m);
}

void bind(vertex_array const& vao)
{
    glBindVertexArray(vao);
}

void bind(buffer const& b)
{
    glBindBuffer(b.target, b);
}

void bind(u32 location, u32 unit, texture const& tex)
{
    glUniform1i(GLint(location), GLint(unit));
    glActiveTexture(GL_TEXTURE0 + GLenum(unit));

    glBindTexture(tex.type, tex);
}

void bind(u32 location, model const& m)
{
    bind(m.vao);
    for (auto [i, tex] : vs::enumerate_with<u32>(m.textures))
        bind(location + i, i, tex);
}

void bind(u32 location, mat4 const& m)
{
    glUniformMatrix4fv(GLint(location), 1, GL_FALSE, value_ptr(m));
}

void bind(u32 location, vec4 const& v)
{
    glUniform4f(GLint(location), v.x, v.y, v.z, v.w);
}

void bind(u32 location, uvec2 const& v)
{
    glUniform2ui(GLint(location), v.x, v.y);
}

void bind(u32 binding, buffer const& b)
{
    glBindBufferBase(b.target, binding, b);
}

void bind(vertex_array & vao, buffer const& b)
{
    bind(vao);
    bind(b);
    unbind(vao);
    unbind(b);
}

void unbind(shader const&)
{
    glUseProgram(0);
}

void unbind(vertex_array const&)
{
    glBindVertexArray(0);
}

void unbind(buffer const& b)
{
    glBindBuffer(b.target, 0);
}

void unbind(u32 unit, texture const& tex)
{
    glActiveTexture(GL_TEXTURE0 + GLenum(unit));
    glBindTexture(tex.type, 0);
}

void unbind(model const& m)
{
    unbind(m.vao);
    for (auto [i, tex] : vs::enumerate_with<u32>(m.textures))
        unbind(i, tex);
}

void unbind(u32 binding, buffer const& b)
{
    glBindBufferBase(b.target, binding, 0);
}

void destroy(shader & s)
{
    glDeleteProgram(s);
}

void destroy(model & m)
{
    destroy(m.vbo);
    destroy(m.ebo);
    destroy(m.vao);
    for (texture t : m.textures)
        destroy(t);
}

void destroy(buffer & b)
{
    glDeleteBuffers(1, &b.id);
}

void destroy(texture & t)
{
    glDeleteTextures(1, &t.id);
}

void destroy(vertex_array & a)
{
    glDeleteVertexArrays(1, &a.id);
}

}

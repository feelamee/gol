#include <engine/context.hpp>

#include <engine/error.hpp>
#include <engine/sdl.hpp>
#include <engine/log.hpp>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_opengl3.h>

namespace gol
{

static void APIENTRY gl_debug_message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    GLchar const* msg,
    void const*
);

static context * g_context;

context::context()
{
    if (g_context)
        throw error("[ERROR][engine] context must be created only once");

    if (SDL_VERSION != SDL_GetVersion())
    {
        throw error{
            "[ERROR][ENGINE] compiled SDL version is different from linked:"
            "compiled {}, linked {}",
            SDL_VERSION, SDL_GetVersion()
        };
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        sdl::throw_error();
    GOL_SCOPE_FAIL { SDL_Quit(); };

    window = SDL_CreateWindow(
        "gol simulation",
        960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        sdl::throw_error();
    GOL_SCOPE_FAIL { SDL_DestroyWindow(window); };

    constexpr auto profile =
#if defined(__WIN32__)
        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
#elif defined(__ANDROID__)
        SDL_GL_CONTEXT_PROFILE_ES
#else
        SDL_GL_CONTEXT_PROFILE_CORE
#endif
        ;

    sdl::set_attr(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    sdl::set_attr(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    sdl::set_attr(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    sdl::set_attr(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    glcontext = SDL_GL_CreateContext(window);
    if (!glcontext)
        sdl::throw_error();
    GOL_SCOPE_FAIL { SDL_GL_DestroyContext(glcontext); };

    auto const load_gl_fn = [](char const* fn)
    {
        return reinterpret_cast<void *>(SDL_GL_GetProcAddress(fn));
    };

    int errc = gladLoadGLES2Loader(load_gl_fn);
    if (!errc)
        throw error("[ERROR][engine] gladLoadGLES2Loader failed");

    std::string_view real_version{ reinterpret_cast<char const*>(glGetString(GL_VERSION)) };
    log::info(log::category::gl, "OpenGL is initialized: {}\n", real_version);

    if (int w, h; SDL_GetWindowSize(window, &w, &h))
        glViewport(0, 0, w, h);
    else
        sdl::log_error();

#ifndef NDEBUG
    glDebugMessageCallback(&gl_debug_message_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    if (!IMGUI_CHECKVERSION())
    {
        throw error{
            "[ERROR][ENGINE] compiled ImGui version is different from linked:"
            "compiled '{}', linked '{}'",
            IMGUI_VERSION, ImGui::GetVersion()
        };
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (!ImGui_ImplSDL3_InitForOpenGL(window, glcontext))
        throw error{ "[ERROR][ENGINE] can't initalize ImGui SDL3 backend for OpenGL" };
    GOL_SCOPE_FAIL { ImGui_ImplSDL3_Shutdown(); };

    if (!ImGui_ImplOpenGL3_Init())
        throw error{ "[ERROR][ENGINE] can't initalize ImGui OpenGL3 backend" };
    GOL_SCOPE_FAIL { ImGui_ImplOpenGL3_Shutdown(); };

    log::info("Dear ImGui is initialized: {}\n", std::string_view(ImGui::GetVersion()));

    g_context = this;
}

context::~context()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    g_context = nullptr;
}

context & ctx()
{
    if (!g_context)
        throw error("[ERROR][engine] gol::context must be created first");

    return *g_context;
}

static log::priority from_gl_severity(GLenum s)
{
    return log::priority::err;
    unimplemented();
    switch (s)
    {
    }

    unreachable();
}

void APIENTRY gl_debug_message_callback(
    GLenum source,
    GLenum /* type */,
    GLuint id,
    GLenum severity,
    GLsizei length,
    GLchar const* msg,
    void const* /* user */
)
{
    static GLuint last_id = -1u;
    static unsigned message_strick = 0;
    constexpr unsigned const max_message_strick = 5;

    if (last_id == id)
        ++message_strick;
    else
    {
        last_id = id;
        message_strick = 0;
    }

    if (message_strick == max_message_strick)
    {
        log::info(
            log::category::gl,
            "Last message was repeated {} times. Now it will be suppressed\n",
            max_message_strick
        );
    }

    if (message_strick >= max_message_strick)
        return;

    log::log(
        log::category::gl, from_gl_severity(severity),
        "ID {}: {}\n",
        id, std::string_view{ msg, sz(length) }
    );
}


}

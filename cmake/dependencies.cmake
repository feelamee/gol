include_guard()

if (GOL_USE_SYSTEM_LIBRARIES)
    set(CPM_USE_LOCAL_PACKAGES ON)
    set(CPM_LOCAL_PACKAGES_ONLY ON)
endif()

include(CPM)
CPMAddPackage(
  NAME SDL3
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
  GIT_TAG release-3.4.12
  OPTIONS "SDL_ASAN ${GOL_SANITIZERS}" "SDL_CCACHE ${GOL_CCACHE}"
)
CPMAddPackage(
  NAME glm
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG 1.0.3
  OPTIONS "GLM_ENABLE_CXX_20 ON"
)


if (NOT GOL_USE_SYSTEM_LIBRARIES)
    if (TARGET SDL3-shared)
        install(
            TARGETS SDL3-shared
            EXPORT SDL3Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    elseif(TARGET SDL3-static)
        install(
            TARGETS SDL3-shared
            EXPORT SDL3Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    endif()

    install(
        TARGETS glm
        EXPORT glmTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

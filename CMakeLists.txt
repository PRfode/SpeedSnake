cmake_minimum_required(VERSION 3.16)
project(SpeedSnake)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL路径配置
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3-3.2.10/x86_64-w64-mingw32")
set(SDL3_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image-3.2.4/x86_64-w64-mingw32")
set(SDL3_TTF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf-3.2.2/x86_64-w64-mingw32")
set(SDL2_MIXER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2_mixer-2.8.1/x86_64-w64-mingw32")

# 检查路径存在性
foreach(LIB_DIR IN ITEMS SDL3_DIR SDL3_IMAGE_DIR SDL3_TTF_DIR SDL2_MIXER_DIR)
    if(NOT EXISTS ${${LIB_DIR}})
        message(FATAL_ERROR "${LIB_DIR} not found: ${${LIB_DIR}}")
    endif()
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 添加utils库
add_library(utils src/utils.cpp src/utils.h)

# 设置utils的传递依赖
target_include_directories(utils PUBLIC
    "${SDL3_DIR}/include"
    "${SDL3_IMAGE_DIR}/include"
    "${SDL3_TTF_DIR}/include"
    "${SDL2_MIXER_DIR}/include"
)

# 添加库文件目录
target_link_directories(utils PUBLIC
    "${SDL3_DIR}/lib"
    "${SDL3_IMAGE_DIR}/lib"
    "${SDL3_TTF_DIR}/lib"
    "${SDL2_MIXER_DIR}/lib"
)



target_link_libraries(utils PUBLIC
    SDL3
    SDL3_image
    SDL3_ttf
    SDL2_mixer
)

# 添加可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 链接utils库（自动获取其所有依赖）
target_link_libraries(${PROJECT_NAME} PRIVATE utils)

# Windows特定设置
if(WIN32)
    # 复制运行时DLL
    set(DLL_FILES
        "${SDL3_DIR}/bin/SDL3.dll"
        "${SDL3_IMAGE_DIR}/bin/SDL3_image.dll"
        "${SDL3_TTF_DIR}/bin/SDL3_ttf.dll"
        "${SDL2_MIXER_DIR}/bin/SDL2_mixer.dll"
    )
    
    foreach(DLL ${DLL_FILES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endforeach()
endif()
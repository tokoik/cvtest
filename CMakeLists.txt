cmake_minimum_required(VERSION 3.22)
project(cvtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

# ソースファイル
set(SOURCES
    main.cpp
    gg.cpp
)

# ヘッダファイル
set(HEADERS
    gg.h
)

# シェーダファイル
set(SHADER_FILES
    simple.vert
    simple.frag
)

# リソースファイル
set(RESOURCE_FILES
)

# ソリューションエクスプローラー用のグループ分け
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Shader Files" FILES ${SHADER_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})

# 外部ライブラリをプロジェクト内の libs/ フォルダへ自動ダウンロードする設定
set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(INCLUDE_DIR "${FETCHCONTENT_BASE_DIR}/include")
set(GL_DIR "${INCLUDE_DIR}/GL")
set(KHR_DIR "${INCLUDE_DIR}/KHR")

file(MAKE_DIRECTORY "${INCLUDE_DIR}")
file(MAKE_DIRECTORY "${GL_DIR}")
file(MAKE_DIRECTORY "${KHR_DIR}")

# OpenGL & KHR ヘッダのダウンロード
if(NOT EXISTS "${GL_DIR}/glext.h")
    message(STATUS "Downloading OpenGL & KHR headers...")
    set(UA_HEADER "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glext.h" "${GL_DIR}/glext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/wglext.h" "${GL_DIR}/wglext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glxext.h" "${GL_DIR}/glxext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/EGL/api/KHR/khrplatform.h" "${KHR_DIR}/khrplatform.h" HTTPHEADER "${UA_HEADER}")
endif()

if(APPLE)

    # macOS の設定
    set(ICON_FILE "opengl.icns")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES} ${ICON_FILE})

    find_package(OpenCV REQUIRED)
    find_library(GLUT_LIBRARY GLUT)
    find_library(OPENGL_LIBRARY OpenGL)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBS}
        ${GLUT_LIBRARY}
        ${OPENGL_LIBRARY}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_ICON_FILE "${ICON_FILE}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2011-2026 Kohe Tokoi. All rights reserved."
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        XCODE_GENERATE_SCHEME ON
    )

    set_source_files_properties(${SHADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY XCODE_GENERATE_SCHEME ON)

elseif(WIN32)

    # Windows (Visual Studio) の設定
    set(OPENCV_DIR "${FETCHCONTENT_BASE_DIR}/opencv")
    set(OPENCV_BUILD_DIR "${OPENCV_DIR}/build")

    if(NOT EXISTS "${OPENCV_BUILD_DIR}/include/opencv2/opencv.hpp")
        # 既存リポジトリにキャッシュがあればコピー
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/opencv/build/include/opencv2/opencv.hpp")
            message(STATUS "Copying cached OpenCV build from calib/libs/opencv/build...")
            file(MAKE_DIRECTORY "${OPENCV_DIR}")
            file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/opencv/build" DESTINATION "${OPENCV_DIR}")
        else()
            message(STATUS "Downloading OpenCV 4.13.0 for Windows...")
            file(DOWNLOAD "https://github.com/opencv/opencv/releases/download/4.13.0/opencv-4.13.0-windows.exe" "${FETCHCONTENT_BASE_DIR}/opencv.exe" SHOW_PROGRESS)
            message(STATUS "Extracting OpenCV...")
            execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf opencv.exe WORKING_DIRECTORY "${FETCHCONTENT_BASE_DIR}")
            file(REMOVE "${FETCHCONTENT_BASE_DIR}/opencv.exe")
        endif()
    endif()

    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES})

    include(FetchContent)
    FetchContent_Declare(
        freeglut
        GIT_REPOSITORY https://github.com/FreeGLUTProject/freeglut.git
        GIT_TAG master
    )

    set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(freeglut)

    find_package(OpenGL REQUIRED)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        "${OPENCV_BUILD_DIR}/include"
        "${freeglut_SOURCE_DIR}/include"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        debug "${OPENCV_BUILD_DIR}/x64/vc16/lib/opencv_world4130d.lib"
        optimized "${OPENCV_BUILD_DIR}/x64/vc16/lib/opencv_world4130.lib"
        freeglut_static
        OpenGL::GL
        OpenGL::GLU
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE FREEGLUT_STATIC)

    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endforeach()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${OPENCV_BUILD_DIR}/x64/vc16/bin/opencv_world4130d.dll,${OPENCV_BUILD_DIR}/x64/vc16/bin/opencv_world4130.dll>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    set_source_files_properties(${SHADER_FILES} ${RESOURCE_FILES} PROPERTIES
        VS_COPY_TO_OUT_DIR PreserveNewest
    )

    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

else()

    # Ubuntu Linux (Makefile) の設定
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES})

    find_package(OpenCV REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${OpenCV_LIBS}
        OpenGL::GL
        OpenGL::GLU
        GLUT::GLUT
        pthread
        m
    )

    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()

endif()

get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# these files are copied only once
file(GLOB TEXTURES
    quad/quad_texture.tga
    grid/rock_texture.tga
    grid/sand_texture.tga
    grid/snow_texture.tga
    grid/grass_texture.tga
    sky/sky_texture.tga
    grid/grid_texture.tga)
copy_files_once(${TEXTURES})

# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
    grid/*glsl
    quad/*.glsl
    sky/*.glsl
    screenquad/*.glsl
    particles/*.glsl
    water/*.glsl)
deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})

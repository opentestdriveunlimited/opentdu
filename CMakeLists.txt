cmake_minimum_required( VERSION 3.15 )
cmake_policy( VERSION 3.15 )

project( OpenTestDriveUnlimited )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set( OTDU_BASE_FOLDER "${CMAKE_SOURCE_DIR}" )
set( OTDU_BIN_FOLDER "${OTDU_BASE_FOLDER}/build/bin" )
set( OTDU_SRC_FOLDER "${OTDU_BASE_FOLDER}/source" )
set( OTDU_THIRDPARTY_FOLDER "${OTDU_BASE_FOLDER}/third_party" )

set( CMAKE_CXX_STANDARD 17 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${OTDU_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OTDU_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OTDU_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OTDU_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OTDU_BIN_FOLDER} )

set( OTDU_DEVBUILD TRUE CACHE BOOL "Build with developer features (verbose logging, devmenu, etc.)" )
set( OTDU_GFX_API "OTDU_VULKAN" CACHE STRING "Graphics backend to target" )
set_property(CACHE OTDU_GFX_API PROPERTY STRINGS OTDU_VULKAN)

add_definitions( -D"${OTDU_GFX_API}" )
if ( ${OTDU_DEVBUILD} )
    add_definitions( -DOTDU_DEVBUILD )
endif()

include( ${OTDU_BASE_FOLDER}/cmake_modules/ide_filters.cmake )

file( GLOB OTDU_SRC "${OTDU_SRC_FOLDER}/tdu_instance.cpp"
                    "${OTDU_SRC_FOLDER}/audio/gs_audio.cpp"
                    "${OTDU_SRC_FOLDER}/core/arg_parser.cpp"
                    "${OTDU_SRC_FOLDER}/core/logger.cpp"
                    "${OTDU_SRC_FOLDER}/core/event.cpp"
                    "${OTDU_SRC_FOLDER}/core/mutex.cpp"
                    "${OTDU_SRC_FOLDER}/core/thread.cpp"
                    "${OTDU_SRC_FOLDER}/core/notifier.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/gs_playerdata.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/user_profile.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/save_game.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/saved_car_reserve.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/saved_ebay_state.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/saved_garage.cpp"
                    "${OTDU_SRC_FOLDER}/player_data/saved_profile.cpp"
                    "${OTDU_SRC_FOLDER}/flash/gs_flash.cpp"
                    "${OTDU_SRC_FOLDER}/flash/bank_common_flash.cpp"
                    "${OTDU_SRC_FOLDER}/flash/bank_flash.cpp"
                    "${OTDU_SRC_FOLDER}/flash/mng_fade.cpp"
                    "${OTDU_SRC_FOLDER}/flash/mng_flash.cpp"
                    "${OTDU_SRC_FOLDER}/flash/modal_message_box.cpp"
                    "${OTDU_SRC_FOLDER}/flash/flash_player.cpp"
                    "${OTDU_SRC_FOLDER}/flash/flash_list.cpp"
                    "${OTDU_SRC_FOLDER}/game/mng_hud.cpp"
                    "${OTDU_SRC_FOLDER}/game/mng_bookmark.cpp"
                    "${OTDU_SRC_FOLDER}/game/mng_base.cpp"
                    "${OTDU_SRC_FOLDER}/game/gm_base.cpp"
                    "${OTDU_SRC_FOLDER}/game/gs_base.cpp"
                    "${OTDU_SRC_FOLDER}/game/gs_intro_pool.cpp"
                    "${OTDU_SRC_FOLDER}/game/cinematic_entity.cpp"
                    "${OTDU_SRC_FOLDER}/game/game_mechanism.cpp"
                    "${OTDU_SRC_FOLDER}/game/game_object.cpp"
                    "${OTDU_SRC_FOLDER}/game/scenaric/gs_scenaric.cpp"
                    "${OTDU_SRC_FOLDER}/game/movie/gm_video_bumper.cpp"
                    "${OTDU_SRC_FOLDER}/game/movie/mng_movie.cpp"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/flash_list_car_showcase.cpp"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/mng_flash_car_showcase.cpp"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/gm_car_showcase.cpp"
                    "${OTDU_SRC_FOLDER}/game/login/gm_login.cpp"
                    "${OTDU_SRC_FOLDER}/game/scene/base_entity.cpp"
                    "${OTDU_SRC_FOLDER}/game/scene/entity_actor.cpp"
                    "${OTDU_SRC_FOLDER}/game/vehicle/bank_vehicle.cpp"
                    "${OTDU_SRC_FOLDER}/game/vehicle/car_desc.cpp"
                    "${OTDU_SRC_FOLDER}/game/vehicle/car_state.cpp"
                    "${OTDU_SRC_FOLDER}/game/vehicle/vehicle_base.cpp"
                    "${OTDU_SRC_FOLDER}/game/vehicle/gs_car_colors.cpp"
                    "${OTDU_SRC_FOLDER}/input/gs_device.cpp"
                    "${OTDU_SRC_FOLDER}/database/gs_database.cpp"
                    "${OTDU_SRC_FOLDER}/system/gs_system.cpp"
                    "${OTDU_SRC_FOLDER}/system/system_win32.cpp"
                    "${OTDU_SRC_FOLDER}/console/gs_console.cpp"
                    "${OTDU_SRC_FOLDER}/config/gs_config.cpp"
                    "${OTDU_SRC_FOLDER}/online/gs_online.cpp"
                    "${OTDU_SRC_FOLDER}/render/gs_render.cpp"
                    "${OTDU_SRC_FOLDER}/render/camera/gs_camera.cpp"
                    "${OTDU_SRC_FOLDER}/render/camera/camera_base.cpp"
                    "${OTDU_SRC_FOLDER}/render/camera/camera_game.cpp"
                    "${OTDU_SRC_FOLDER}/render/camera/vehicle_cameras.cpp"
                    "${OTDU_SRC_FOLDER}/render/2dm.cpp"
                    "${OTDU_SRC_FOLDER}/render/2db.cpp"
                    "${OTDU_SRC_FOLDER}/render/3dd.cpp"
                    "${OTDU_SRC_FOLDER}/render/3dg.cpp"
                    "${OTDU_SRC_FOLDER}/render/uva.cpp"
                    "${OTDU_SRC_FOLDER}/render/frustum.cpp"
                    "${OTDU_SRC_FOLDER}/render/material.cpp"
                    "${OTDU_SRC_FOLDER}/render/mng_number.cpp"
                    "${OTDU_SRC_FOLDER}/render/mng_shadow.cpp"
                    "${OTDU_SRC_FOLDER}/render/mng_reflection.cpp"
                    "${OTDU_SRC_FOLDER}/render/render_scene.cpp"
                    "${OTDU_SRC_FOLDER}/render/render_file.cpp"
                    "${OTDU_SRC_FOLDER}/render/draw_list.cpp"
                    "${OTDU_SRC_FOLDER}/render/shader_register.cpp"
                    "${OTDU_SRC_FOLDER}/render/vulkan/render_device.cpp"
                    "${OTDU_SRC_FOLDER}/win32/entry_point.cpp"
                    "${OTDU_SRC_FOLDER}/unix/entry_point.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_file.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/bank.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_boot_datas.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/file_handle.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/virtual_file_system.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_dirty_disk.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/virtual_file_system.cpp"
                    "${OTDU_SRC_FOLDER}/filesystem/file_path_register.cpp" 
                    "${OTDU_SRC_FOLDER}/world/gs_world.cpp"
                    "${OTDU_SRC_FOLDER}/world/world_listener.cpp" 
                    "${OTDU_SRC_FOLDER}/world/weather/weather_desc.cpp"
                    "${OTDU_SRC_FOLDER}/world/weather/weather_config.cpp"
                    "${OTDU_SRC_FOLDER}/world/weather/gs_weather.cpp" )

file( GLOB OTDU_INC "${OTDU_SRC_FOLDER}/tdu_instance.h"
                    "${OTDU_SRC_FOLDER}/shared.h"
                    "${OTDU_SRC_FOLDER}/audio/gs_audio.h"
                    "${OTDU_SRC_FOLDER}/core/arg_parser.h"
                    "${OTDU_SRC_FOLDER}/core/logger.h"
                    "${OTDU_SRC_FOLDER}/core/event.h"
                    "${OTDU_SRC_FOLDER}/core/mutex.h"
                    "${OTDU_SRC_FOLDER}/core/thread.h"
                    "${OTDU_SRC_FOLDER}/core/notifier.h"
                    "${OTDU_SRC_FOLDER}/core/assert.h"
                    "${OTDU_SRC_FOLDER}/core/locale.h"
                    "${OTDU_SRC_FOLDER}/core/hash/crc32.h"
                    "${OTDU_SRC_FOLDER}/core/hash/crc64.h"
                    "${OTDU_SRC_FOLDER}/core/hash/tdu_concat.h"
                    "${OTDU_SRC_FOLDER}/core/crypto/tea.h"
                    "${OTDU_SRC_FOLDER}/core/random/bcpl.h"
                    "${OTDU_SRC_FOLDER}/core/time.h"
                    "${OTDU_SRC_FOLDER}/core/color.h"
                    "${OTDU_SRC_FOLDER}/core/popup_callback.h"
                    "${OTDU_SRC_FOLDER}/player_data/gs_playerdata.h"
                    "${OTDU_SRC_FOLDER}/player_data/save_game.h"
                    "${OTDU_SRC_FOLDER}/player_data/saved_car_reserve.h"
                    "${OTDU_SRC_FOLDER}/player_data/saved_ebay_state.h"
                    "${OTDU_SRC_FOLDER}/player_data/saved_garage.h"
                    "${OTDU_SRC_FOLDER}/player_data/saved_profile.h"
                    "${OTDU_SRC_FOLDER}/flash/gs_flash.h"
                    "${OTDU_SRC_FOLDER}/flash/flash_cmds.h"
                    "${OTDU_SRC_FOLDER}/flash/flash_resources.h"
                    "${OTDU_SRC_FOLDER}/flash/movie_player.h"
                    "${OTDU_SRC_FOLDER}/flash/modal_message_box.h"
                    "${OTDU_SRC_FOLDER}/flash/mng_flash.h"
                    "${OTDU_SRC_FOLDER}/flash/mng_fade.h"
                    "${OTDU_SRC_FOLDER}/flash/flash_player.h"
                    "${OTDU_SRC_FOLDER}/flash/flash_list.h"
                    "${OTDU_SRC_FOLDER}/flash/flash_entity.h"
                    "${OTDU_SRC_FOLDER}/flash/bank_flash.h"
                    "${OTDU_SRC_FOLDER}/flash/bank_common_flash.h"
                    "${OTDU_SRC_FOLDER}/game/cinematic_entity.h"
                    "${OTDU_SRC_FOLDER}/game/game_mechanism.h"
                    "${OTDU_SRC_FOLDER}/game/game_object.h"
                    "${OTDU_SRC_FOLDER}/game/gm_base.h"
                    "${OTDU_SRC_FOLDER}/game/gs_base.h"
                    "${OTDU_SRC_FOLDER}/game/material_types.h"
                    "${OTDU_SRC_FOLDER}/game/gs_intro_pool.h"
                    "${OTDU_SRC_FOLDER}/game/mng_base.h"
                    "${OTDU_SRC_FOLDER}/game/mng_hud.h"
                    "${OTDU_SRC_FOLDER}/game/mng_bookmark.h"
                    "${OTDU_SRC_FOLDER}/game/scenaric/gs_scenaric.h"
                    "${OTDU_SRC_FOLDER}/game/movie/gm_video_bumper.h"
                    "${OTDU_SRC_FOLDER}/game/movie/mng_movie.h"
                    "${OTDU_SRC_FOLDER}/game/freeride/gm_freeride.h"
                    "${OTDU_SRC_FOLDER}/game/game_cheats.h"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/flash_list_car_showcase.h"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/gm_car_showcase.h"
                    "${OTDU_SRC_FOLDER}/game/car_showcase/mng_flash_car_showcase.h"
                    "${OTDU_SRC_FOLDER}/game/login/gm_login.h"
                    "${OTDU_SRC_FOLDER}/game/scene/base_entity.h"
                    "${OTDU_SRC_FOLDER}/game/scene/entity_actor.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/bank_vehicle.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/car_desc.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/car_state.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/vehicle_base.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/paint.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/vehicle_resource.h"
                    "${OTDU_SRC_FOLDER}/game/vehicle/gs_car_colors.h"
                    "${OTDU_SRC_FOLDER}/input/gs_device.h"
                    "${OTDU_SRC_FOLDER}/database/db_achievement.h"
                    "${OTDU_SRC_FOLDER}/database/db_aftermarket_pack.h"
                    "${OTDU_SRC_FOLDER}/database/db_bot.h"
                    "${OTDU_SRC_FOLDER}/database/db_brand.h"
                    "${OTDU_SRC_FOLDER}/database/db_car_color.h"
                    "${OTDU_SRC_FOLDER}/database/db_car_pack.h"
                    "${OTDU_SRC_FOLDER}/database/db_car_physics_data.h"
                    "${OTDU_SRC_FOLDER}/database/db_car_rim.h"
                    "${OTDU_SRC_FOLDER}/database/db_car_shop.h"
                    "${OTDU_SRC_FOLDER}/database/db_clothe.h"
                    "${OTDU_SRC_FOLDER}/database/db_hair.h"
                    "${OTDU_SRC_FOLDER}/database/db_house.h"
                    "${OTDU_SRC_FOLDER}/database/db_interior.h"
                    "${OTDU_SRC_FOLDER}/database/db_menu.h"
                    "${OTDU_SRC_FOLDER}/database/db_pnj.h"
                    "${OTDU_SRC_FOLDER}/database/db_rim.h"
                    "${OTDU_SRC_FOLDER}/database/db_subtitle.h"
                    "${OTDU_SRC_FOLDER}/database/db_tutorial.h"
                    "${OTDU_SRC_FOLDER}/database/gs_database.h"
                    "${OTDU_SRC_FOLDER}/system/system_win32.h"
                    "${OTDU_SRC_FOLDER}/system/gs_system.h"
                    "${OTDU_SRC_FOLDER}/system/system_shared.h"
                    "${OTDU_SRC_FOLDER}/console/gs_console.h"
                    "${OTDU_SRC_FOLDER}/config/gs_config.h"
                    "${OTDU_SRC_FOLDER}/online/gs_online.h"
                    "${OTDU_SRC_FOLDER}/online/packets_crc.h"
                    "${OTDU_SRC_FOLDER}/physics/collision_event.h"
                    "${OTDU_SRC_FOLDER}/physics/dynamic_rigid_body.h"
                    "${OTDU_SRC_FOLDER}/physics/engine_simulation.h"
                    "${OTDU_SRC_FOLDER}/physics/physics_factory.h"
                    "${OTDU_SRC_FOLDER}/render/gs_render.h"
                    "${OTDU_SRC_FOLDER}/render/camera/gs_camera.h"
                    "${OTDU_SRC_FOLDER}/render/camera/camera_base.h"
                    "${OTDU_SRC_FOLDER}/render/camera/camera_game.h"
                    "${OTDU_SRC_FOLDER}/render/camera/vehicle_cameras.h"
                    "${OTDU_SRC_FOLDER}/render/particles/particle_instance.h"
                    "${OTDU_SRC_FOLDER}/render/gs_render_helper.h"
                    "${OTDU_SRC_FOLDER}/render/2dm.h"
                    "${OTDU_SRC_FOLDER}/render/2db.h"
                    "${OTDU_SRC_FOLDER}/render/3dd.h"
                    "${OTDU_SRC_FOLDER}/render/3dg.h"
                    "${OTDU_SRC_FOLDER}/render/uva.h"
                    "${OTDU_SRC_FOLDER}/render/mng_number.h"
                    "${OTDU_SRC_FOLDER}/render/mng_reflection.h"
                    "${OTDU_SRC_FOLDER}/render/mng_shadow.h"
                    "${OTDU_SRC_FOLDER}/render/render_file.h"
                    "${OTDU_SRC_FOLDER}/render/material.h"
                    "${OTDU_SRC_FOLDER}/render/draw_list.h"
                    "${OTDU_SRC_FOLDER}/render/shader_register.h"
                    "${OTDU_SRC_FOLDER}/render/file_collection_2d.h"
                    "${OTDU_SRC_FOLDER}/render/file_collection_3d.h"
                    "${OTDU_SRC_FOLDER}/render/frustum.h"
                    "${OTDU_SRC_FOLDER}/render/hierarchy_node.h"
                    "${OTDU_SRC_FOLDER}/render/instance.h"
                    "${OTDU_SRC_FOLDER}/render/light.h"
                    "${OTDU_SRC_FOLDER}/render/lod.h"
                    "${OTDU_SRC_FOLDER}/render/render_scene.h"
                    "${OTDU_SRC_FOLDER}/render/render_target.h"
                    "${OTDU_SRC_FOLDER}/render/viewport.h"
                    "${OTDU_SRC_FOLDER}/render/vulkan/render_device.h"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_file.h"
                    "${OTDU_SRC_FOLDER}/filesystem/bank.h"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_boot_datas.h"
                    "${OTDU_SRC_FOLDER}/filesystem/gs_dirty_disk.h"
                    "${OTDU_SRC_FOLDER}/filesystem/virtual_file_system.h"
                    "${OTDU_SRC_FOLDER}/filesystem/file_path_register.h" 
                    "${OTDU_SRC_FOLDER}/filesystem/streamed_resource.h" 
                    "${OTDU_SRC_FOLDER}/filesystem/file_handle.h" 
                    "${OTDU_SRC_FOLDER}/world/gs_world.h"
                    "${OTDU_SRC_FOLDER}/world/world_listener.h" 
                    "${OTDU_SRC_FOLDER}/world/weather/weather_desc.h"
                    "${OTDU_SRC_FOLDER}/world/weather/weather_config.h"
                    "${OTDU_SRC_FOLDER}/world/weather/time_of_day.h"
                    "${OTDU_SRC_FOLDER}/world/weather/hdri_config.h"
                    "${OTDU_SRC_FOLDER}/world/weather/gs_weather.h"
                    "${OTDU_SRC_FOLDER}/gs_timer.h"  )

file( GLOB OTDU_THIRDPARTY_SRC "${OTDU_THIRDPARTY_FOLDER}/cityhash/src/city.cc"
                               "${OTDU_THIRDPARTY_FOLDER}/cityhash/src/city.h"
                               "${OTDU_THIRDPARTY_FOLDER}/cityhash/config.h"
                               "${OTDU_THIRDPARTY_FOLDER}/cityhash/src/citycrc.h" )

set( OTDU_SOURCES ${OTDU_SRC} ${OTDU_INC} ${OTDU_THIRDPARTY_SRC} )
add_ide_filters( "${OTDU_SOURCES}" )
set( OTDU_SOURCES ${OTDU_SOURCES} )

add_executable( opentdu ${GUI_TYPE} ${OTDU_SOURCES} )
set_property(TARGET opentdu PROPERTY FOLDER "Projects")

include_directories( "${OTDU_SRC_FOLDER}" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/cityhash/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/cityhash/src/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/glfw/include/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/Vulkan-Headers/include/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/volk/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/VulkanMemoryAllocator/include" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/eigen" )

if ( WIN32 )
    target_link_libraries( opentdu winmm Pathcch Shlwapi )
    set_target_properties( opentdu PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
    add_definitions( -DOTDU_WIN32 )
elseif ( APPLE )
    add_definitions( -DOTDU_MACOS )
    add_definitions( -DOTDU_UNIX )
endif ( WIN32 )

if( MSVC )
    set_property( TARGET opentdu PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${OTDU_BIN_FOLDER} )
    target_compile_options( opentdu PRIVATE /W3 /WX )
    add_definitions( -DOTDU_MSVC -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING )
elseif( APPLE )
    target_compile_options( opentdu PRIVATE -Wall -Wextra )
    target_compile_options(opentdu PUBLIC -Wno-unused-variable)
    target_compile_options(opentdu PUBLIC -Wno-unused-private-field)
    target_compile_options(opentdu PUBLIC -Wno-unused-function)
    target_compile_options(opentdu PUBLIC -Wno-unused-parameter)
    add_definitions( -DOTDU_CLANG )
endif()

if ( WIN32 )
target_link_libraries( opentdu debug "${OTDU_THIRDPARTY_FOLDER}/glfw/src/Debug/glfw3.lib" )
target_link_libraries( opentdu optimized "${OTDU_THIRDPARTY_FOLDER}/glfw/src/MinSizeRel/glfw3.lib" )
elseif( APPLE )
target_link_libraries( opentdu debug "${OTDU_THIRDPARTY_FOLDER}/glfw/src/libglfw3.a" )
target_link_libraries( opentdu optimized "${OTDU_THIRDPARTY_FOLDER}/glfw/src/libglfw3.a" )
#target_link_libraries( opentdu optimized "${OTDU_THIRDPARTY_FOLDER}/MoltenVK/static/MoltenVK.xcframework" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/MoltenVK/include" )

target_link_libraries( opentdu "-framework QuartzCore -framework Metal -framework Cocoa -framework IOKit -framework IOSurface -framework CoreVideo" )
endif( WIN32 )

add_subdirectory( tools/asset_extractor )
add_subdirectory( tools/save_decryptor )
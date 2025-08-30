file( GLOB OTDU_EAE_SRC "${OTDU_BASE_FOLDER}/tools/asset_extractor/main.cpp" 
                        "${OTDU_SRC_FOLDER}/core/arg_parser.cpp"
                        "${OTDU_SRC_FOLDER}/core/logger.cpp")

file( GLOB OTDU_DXVK_SRC "${OTDU_THIRDPARTY_FOLDER}/dxvk/src/dxso/*.cpp" 
                         "${OTDU_THIRDPARTY_FOLDER}/dxvk/src/dxso/*.h"
                         "${OTDU_THIRDPARTY_FOLDER}/dxvk/src/spirv/*.cpp" 
                         "${OTDU_THIRDPARTY_FOLDER}/dxvk/src/spirv/*.h")
                                 
file( GLOB OTDU_SPVC_SRC "${OTDU_THIRDPARTY_FOLDER}/spirv_cross/spirv_*.cpp" 
                         "${OTDU_THIRDPARTY_FOLDER}/spirv_cross/spirv_*.hpp" )

add_executable( opentdu_assetExtractor ${OTDU_EAE_SRC} ${OTDU_DXVK_SRC} ${OTDU_SPVC_SRC} )
set_property(TARGET opentdu_assetExtractor PROPERTY FOLDER "Tools")

include_directories( "${OTDU_THIRDPARTY_FOLDER}/dxvk/include" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/Vulkan-Headers/include" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/spirv/include" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/spirv_cross/" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/spirv_cross/include" )

if ( WIN32 )
    set_target_properties( opentdu_assetExtractor PROPERTIES OUTPUT_NAME "opentdu_assetextractor_${CMAKE_GENERATOR_PLATFORM}")
elseif ( APPLE )
    set_target_properties( opentdu_assetExtractor PROPERTIES OUTPUT_NAME "opentdu_assetextractor_darwin")
else()
    set_target_properties( opentdu_assetExtractor PROPERTIES OUTPUT_NAME "opentdu_assetextractor_unix")
endif()

add_definitions( -DOTDU_LOG_STDOUT )

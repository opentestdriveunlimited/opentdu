file( GLOB OTDU_EAE_SRC "${OTDU_BASE_FOLDER}/tools/asset_extractor/main.cpp" 
                        "${OTDU_SRC_FOLDER}/core/arg_parser.cpp"
                        "${OTDU_SRC_FOLDER}/core/logger.cpp")

add_executable( opentdu_assetExtractor ${OTDU_EAE_SRC} )
set_property(TARGET opentdu_assetExtractor PROPERTY FOLDER "Tools")

if ( WIN32 )
    target_link_libraries( opentdu_assetExtractor winmm Pathcch Shlwapi )
endif ( WIN32 )

target_link_libraries( opentdu_assetExtractor "${OTDU_THIRDPARTY_FOLDER}/HLSLCrossCompiler/lib64/RelWithDebInfo/libHLSLcc.lib" )
include_directories( "${OTDU_THIRDPARTY_FOLDER}/HLSLCrossCompiler/include" )

add_definitions( -DOTDU_LOG_STDOUT )

file( GLOB OTDU_EAE_SRC "${OTDU_BASE_FOLDER}/tools/save_decryptor/main.cpp" 
                        "${OTDU_SRC_FOLDER}/core/arg_parser.cpp"
                        "${OTDU_SRC_FOLDER}/core/logger.cpp")

add_executable( opentdu_savedecryptor ${OTDU_EAE_SRC} )
set_property(TARGET opentdu_savedecryptor PROPERTY FOLDER "Tools")

if ( WIN32 )
    target_link_libraries( opentdu_savedecryptor winmm Pathcch Shlwapi )
endif ( WIN32 )

add_definitions( -DOTDU_LOG_STDOUT )

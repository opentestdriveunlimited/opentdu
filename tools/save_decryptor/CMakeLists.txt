file( GLOB OTDU_EAE_SRC "${OTDU_BASE_FOLDER}/tools/save_decryptor/main.cpp" 
                        "${OTDU_SRC_FOLDER}/core/arg_parser.cpp"
                        "${OTDU_SRC_FOLDER}/core/logger.cpp")

add_executable( opentdu_savedecryptor ${OTDU_EAE_SRC} )
set_property(TARGET opentdu_savedecryptor PROPERTY FOLDER "Tools")

if ( WIN32 )
    target_link_libraries( opentdu_savedecryptor winmm Pathcch Shlwapi )
endif ( WIN32 )

if ( WIN32 )
    set_target_properties( opentdu_savedecryptor PROPERTIES OUTPUT_NAME "opentdu_savedecryptor_win32")
elseif ( APPLE )
    set_target_properties( opentdu_savedecryptor PROPERTIES OUTPUT_NAME "opentdu_savedecryptor_darwin")
else()
    set_target_properties( opentdu_savedecryptor PROPERTIES OUTPUT_NAME "opentdu_savedecryptor_unix")
endif()

add_definitions( -DOTDU_LOG_STDOUT )

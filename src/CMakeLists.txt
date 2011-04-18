configure_file(${CMAKE_CURRENT_SOURCE_DIR}/broccoli.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/broccoli.h)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/compat)

########################################################################
## libbroccoli target

if (BRO_PCAP_SUPPORT)
    set(PACKET_FILES bro_packet.c bro_packet.h)
endif ()

bison_target(Bro_Parser bro_parser.y ${CMAKE_CURRENT_BINARY_DIR}/bro_parser.c
             VERBOSE ${CMAKE_CURRENT_BINARY-DIR}/bro_parser.output
             COMPILE_FLAGS "-p bro")
flex_target(Bro_Lexer bro_lexer.l ${CMAKE_CURRENT_BINARY_DIR}/bro_lexer.c
             COMPILE_FLAGS "-Pbro")
add_flex_bison_dependency(Bro_Lexer Bro_Parser)

set(broccoli_SRCS
    ${PACKET_FILES}
    ${FLEX_Bro_Lexer_INPUT}
    ${BISON_Bro_Parser_INPUT}
    ${FLEX_Bro_Lexer_OUTPUTS}
    ${BISON_Bro_Parser_OUTPUTS}
    broccoli.h
    bro.c
    bro_attr.c
    bro_attr.h
    bro_attrs.c
    bro_attrs.h
    bro_buf.h
    bro_buf.c
    bro_config.h
    bro_config.c
    bro_debug.h
    bro_debug.c
    bro_event.h
    bro_event.c
    bro_event_reg.h
    bro_event_reg.c
    bro_hashtable.h
    bro_hashtable.c
    bro_id.h
    bro_id.c
    bro_io.h
    bro_io.c
    bro_list.h
    bro_list.c
    bro_location.h
    bro_location.c
    bro_object.h
    bro_object.c
    bro_openssl.h
    bro_openssl.c
    bro_record.h
    bro_record.c
    bro_sobject.h
    bro_sobject.c
    bro_table.h
    bro_table.c
    bro_type.h
    bro_type.c
    bro_type_decl.h
    bro_type_decl.c
    bro_types.h
    bro_util.h
    bro_util.c
    bro_val.h
    bro_val.c
)

add_library(broccoli ${broccoli_SRCS})

target_link_libraries(broccoli ${OpenSSL_LIBRARIES}
                               ${BRO_LIBADD}
)

set_target_properties(broccoli PROPERTIES
                      VERSION ${SOVersion}.${SOVersionMinor}.${SOVersionRelease}
                      SOVERSION ${SOVersion})

if (APPLE)
    # Mac OS X needs the directory portion of the install_name shared lib field
    set_target_properties(broccoli PROPERTIES
                          INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif ()

install(TARGETS broccoli DESTINATION lib)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/broccoli.h DESTINATION include)

# Set as a convenience for projects that depend on broccoli, but want the
# option of building it from source
set(Broccoli_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}
    CACHE STRING "Broccoli header directory" FORCE)

set(Broccoli_LIBRARY broccoli
    CACHE STRING "Broccoli library" FORCE)

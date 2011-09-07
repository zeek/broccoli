configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
               @ONLY)

add_custom_target(broccolidoc
                  COMMAND doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                  COMMENT "[Doxygen] Generating HTML Broccoli API docs")

add_custom_target(broccolidocclean
                  COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/html)

if (TARGET doc)
    add_dependencies(doc broccolidoc)
else ()
    add_custom_target(doc DEPENDS broccolidoc)
endif ()

if (TARGET docclean)
    add_dependencies(docclean broccolidocclean)
else ()
    add_custom_target(docclean DEPENDS broccolidocclean)
endif ()

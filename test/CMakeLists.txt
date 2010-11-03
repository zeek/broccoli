include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

add_executable(broping broping.c)
target_link_libraries(broping broccoli)

add_executable(broconn broconn.c)
target_link_libraries(broconn broccoli)

add_executable(brohose brohose.c)
target_link_libraries(brohose broccoli)

add_executable(broconftest broconftest.c)
target_link_libraries(broconftest broccoli)

add_executable(broenum broenum.c)
target_link_libraries(broenum broccoli)

add_executable(brotable brotable.c)
target_link_libraries(brotable broccoli)

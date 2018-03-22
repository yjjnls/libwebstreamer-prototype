project(rtsp-test-server)


file(GLOB_RECURSE RTSPSERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
                      ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(rtsp-test-server ${RTSPSERVER_SOURCES} )
target_link_libraries(rtsp-test-server ${GST_MODULES_LIBRARIES} )

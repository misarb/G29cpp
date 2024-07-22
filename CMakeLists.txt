cmake_minimum_required(VERSION 3.10)
project(G29Driver CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the hidapi library
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)

# Add the G29 library
add_library(G29
    src/G29.cpp
    src/G29.hpp
)

target_include_directories(G29
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        ${HIDAPI_INCLUDE_DIRS}
)

target_link_libraries(G29
    PRIVATE
        ${HIDAPI_LIBRARIES}
)

# Add the example executable
add_executable(g29_text_example
    main.cpp
)

target_link_libraries(g29_text_example
    PRIVATE
        G29
        ${HIDAPI_LIBRARIES}
)

# Install rules
# install(TARGETS G29 g29_example
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# )

# install(FILES include/G29.h DESTINATION include)
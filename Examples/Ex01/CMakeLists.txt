include_directories(${Utils_INCLUDE} ${FreeImage_INCLUDE} ../../)
add_executable(Example1 Main.cpp)
target_link_libraries(Example1 Utils FreeImage Image)
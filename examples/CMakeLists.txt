include_directories (${pcomb_SOURCE_DIR}/include) 
link_directories (${pcomb_BINARY_DIR}/lib)

# Add executable that is built from the source files.
add_executable (calc calc.cc)

# Link the executable to the library. 
target_link_libraries (calc ParserCombinator) 
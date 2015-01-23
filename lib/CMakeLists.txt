# Make sure the compiler can find include files from our library. 
include_directories (${pcomb_SOURCE_DIR}/include)

set (SourceFiles StringRef/StringRef.cc)

# Create the StringRef library
add_library (ParserCombinator ${SourceFiles})

# Build the google test library for unit test
add_subdirectory (gtest-minimal) 
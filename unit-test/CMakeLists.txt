# Where Google Test's .h files can be found.
include_directories(
  ${gtest_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR}
  ${pcomb_SOURCE_DIR}/include)

# Where Google Test's libraries can be found.
link_directories (${pcomb_BINARY_DIR}/lib)

# We don't want to put those test programs in /bin. 
set (EXECUTABLE_OUTPUT_PATH ${pcomb_BINARY_DIR}/unit-test)

cxx_executable(UnitTest UnitTest "gtest_main;ParserCombinator")
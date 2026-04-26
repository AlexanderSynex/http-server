include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(create_test)
  set(TARGET_NAME tests)
  set(multiValueArgs SOURCES TESTS DEPENDS)
  # Parse the arguments
  cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

  add_executable(${TARGET_NAME})
 foreach(TEST_DIR ${ARG_TESTS})
    list(APPEND TEST_FILES ${TEST_DIR}/test.cxx)
  endforeach()
  target_sources(${TARGET_NAME} PRIVATE ${TEST_FILES})
  target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main)

  if (ARG_DEPENDS)
    target_link_libraries(${TARGET_NAME} PUBLIC "${ARG_DEPENDS}")
  endif()

  gtest_discover_tests(${TARGET_NAME})
endfunction()

create_test(TESTS test_folder)

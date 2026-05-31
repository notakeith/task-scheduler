include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
  container_ut.cpp
  corner_ut.cpp
  datatypes_ut.cpp
  exception_ut.cpp
  future_ut.cpp
  invoke_ut.cpp
  obj_reference_ut.cpp
  order_ut.cpp
  simple_ut.cpp
)

target_link_libraries(tests
    GTest::gtest_main
    GTest::gmock_main
)


target_include_directories(tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

gtest_discover_tests(tests)

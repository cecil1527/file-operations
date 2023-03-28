# only compile test exe if we're actually working on the project
if(NOT PROJECT_IS_TOP_LEVEL)
	return()
endif()


# https://google.github.io/googletest/quickstart-cmake.html

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip # Oct2022
)
# i'm assuming you could also use cmake's fetch content declaration for a git repository with the hash tag to specify
# the version. there are examples of that on cmake's website.

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# make a name for our testing executable. it has to be different than our library name (otherwise i guess they're
# considered the same cmake variable - because it doesn't differentiate between file extensions)
set(test_exe "${PROJECT_NAME}_test")

add_executable(${test_exe} "test.cpp")
set_property(TARGET ${test_exe} PROPERTY CXX_STANDARD 20)

target_link_libraries(${test_exe}
  PUBLIC ${PROJECT_NAME}
  PUBLIC GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${test_exe})
# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/egor/Рабочий стол/afina"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/egor/Рабочий стол/afina"

# Include any dependencies generated for this target.
include test/storage/CMakeFiles/runStorageTests.dir/depend.make

# Include the progress variables for this target.
include test/storage/CMakeFiles/runStorageTests.dir/progress.make

# Include the compile flags for this target's objects.
include test/storage/CMakeFiles/runStorageTests.dir/flags.make

test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.o: test/storage/CMakeFiles/runStorageTests.dir/flags.make
test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.o: test/storage/StorageTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/egor/Рабочий стол/afina/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.o"
	cd "/home/egor/Рабочий стол/afina/test/storage" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/runStorageTests.dir/StorageTest.cpp.o -c "/home/egor/Рабочий стол/afina/test/storage/StorageTest.cpp"

test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/runStorageTests.dir/StorageTest.cpp.i"
	cd "/home/egor/Рабочий стол/afina/test/storage" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/egor/Рабочий стол/afina/test/storage/StorageTest.cpp" > CMakeFiles/runStorageTests.dir/StorageTest.cpp.i

test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/runStorageTests.dir/StorageTest.cpp.s"
	cd "/home/egor/Рабочий стол/afina/test/storage" && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/egor/Рабочий стол/afina/test/storage/StorageTest.cpp" -o CMakeFiles/runStorageTests.dir/StorageTest.cpp.s

# Object files for target runStorageTests
runStorageTests_OBJECTS = \
"CMakeFiles/runStorageTests.dir/StorageTest.cpp.o"

# External object files for target runStorageTests
runStorageTests_EXTERNAL_OBJECTS = \
"/home/egor/Рабочий стол/afina/third-party/backward-cpp/CMakeFiles/backward_object.dir/backward.cpp.o"

test/storage/runStorageTests: test/storage/CMakeFiles/runStorageTests.dir/StorageTest.cpp.o
test/storage/runStorageTests: third-party/backward-cpp/CMakeFiles/backward_object.dir/backward.cpp.o
test/storage/runStorageTests: test/storage/CMakeFiles/runStorageTests.dir/build.make
test/storage/runStorageTests: src/storage/libStorage.a
test/storage/runStorageTests: third-party/googletest-release-1.8.0/googlemock/gtest/libgtest.a
test/storage/runStorageTests: third-party/googletest-release-1.8.0/googlemock/gtest/libgtest_main.a
test/storage/runStorageTests: third-party/googletest-release-1.8.0/googlemock/gtest/libgtest.a
test/storage/runStorageTests: test/storage/CMakeFiles/runStorageTests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/egor/Рабочий стол/afina/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable runStorageTests"
	cd "/home/egor/Рабочий стол/afina/test/storage" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/runStorageTests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/storage/CMakeFiles/runStorageTests.dir/build: test/storage/runStorageTests

.PHONY : test/storage/CMakeFiles/runStorageTests.dir/build

test/storage/CMakeFiles/runStorageTests.dir/clean:
	cd "/home/egor/Рабочий стол/afina/test/storage" && $(CMAKE_COMMAND) -P CMakeFiles/runStorageTests.dir/cmake_clean.cmake
.PHONY : test/storage/CMakeFiles/runStorageTests.dir/clean

test/storage/CMakeFiles/runStorageTests.dir/depend:
	cd "/home/egor/Рабочий стол/afina" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/egor/Рабочий стол/afina" "/home/egor/Рабочий стол/afina/test/storage" "/home/egor/Рабочий стол/afina" "/home/egor/Рабочий стол/afina/test/storage" "/home/egor/Рабочий стол/afina/test/storage/CMakeFiles/runStorageTests.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : test/storage/CMakeFiles/runStorageTests.dir/depend


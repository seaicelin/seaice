# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/seaicelin/Desktop/seaice

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/seaicelin/Desktop/seaice/build

# Include any dependencies generated for this target.
include CMakeFiles/testIOM.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testIOM.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testIOM.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testIOM.dir/flags.make

CMakeFiles/testIOM.dir/test/testIOM.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/test/testIOM.cpp.o: ../test/testIOM.cpp
CMakeFiles/testIOM.dir/test/testIOM.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testIOM.dir/test/testIOM.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/test/testIOM.cpp.o -MF CMakeFiles/testIOM.dir/test/testIOM.cpp.o.d -o CMakeFiles/testIOM.dir/test/testIOM.cpp.o -c /Users/seaicelin/Desktop/seaice/test/testIOM.cpp

CMakeFiles/testIOM.dir/test/testIOM.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/test/testIOM.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/test/testIOM.cpp > CMakeFiles/testIOM.dir/test/testIOM.cpp.i

CMakeFiles/testIOM.dir/test/testIOM.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/test/testIOM.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/test/testIOM.cpp -o CMakeFiles/testIOM.dir/test/testIOM.cpp.s

CMakeFiles/testIOM.dir/seaice/config.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/config.cpp.o: ../seaice/config.cpp
CMakeFiles/testIOM.dir/seaice/config.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/testIOM.dir/seaice/config.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/config.cpp.o -MF CMakeFiles/testIOM.dir/seaice/config.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/config.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/config.cpp

CMakeFiles/testIOM.dir/seaice/config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/config.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/config.cpp > CMakeFiles/testIOM.dir/seaice/config.cpp.i

CMakeFiles/testIOM.dir/seaice/config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/config.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/config.cpp -o CMakeFiles/testIOM.dir/seaice/config.cpp.s

CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o: ../seaice/fd_manager.cpp
CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o -MF CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/fd_manager.cpp

CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/fd_manager.cpp > CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.i

CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/fd_manager.cpp -o CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.s

CMakeFiles/testIOM.dir/seaice/fiber.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/fiber.cpp.o: ../seaice/fiber.cpp
CMakeFiles/testIOM.dir/seaice/fiber.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/testIOM.dir/seaice/fiber.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/fiber.cpp.o -MF CMakeFiles/testIOM.dir/seaice/fiber.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/fiber.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/fiber.cpp

CMakeFiles/testIOM.dir/seaice/fiber.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/fiber.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/fiber.cpp > CMakeFiles/testIOM.dir/seaice/fiber.cpp.i

CMakeFiles/testIOM.dir/seaice/fiber.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/fiber.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/fiber.cpp -o CMakeFiles/testIOM.dir/seaice/fiber.cpp.s

CMakeFiles/testIOM.dir/seaice/hook.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/hook.cpp.o: ../seaice/hook.cpp
CMakeFiles/testIOM.dir/seaice/hook.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/testIOM.dir/seaice/hook.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/hook.cpp.o -MF CMakeFiles/testIOM.dir/seaice/hook.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/hook.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/hook.cpp

CMakeFiles/testIOM.dir/seaice/hook.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/hook.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/hook.cpp > CMakeFiles/testIOM.dir/seaice/hook.cpp.i

CMakeFiles/testIOM.dir/seaice/hook.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/hook.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/hook.cpp -o CMakeFiles/testIOM.dir/seaice/hook.cpp.s

CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o: ../seaice/iomanager.cpp
CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o -MF CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/iomanager.cpp

CMakeFiles/testIOM.dir/seaice/iomanager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/iomanager.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/iomanager.cpp > CMakeFiles/testIOM.dir/seaice/iomanager.cpp.i

CMakeFiles/testIOM.dir/seaice/iomanager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/iomanager.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/iomanager.cpp -o CMakeFiles/testIOM.dir/seaice/iomanager.cpp.s

CMakeFiles/testIOM.dir/seaice/log.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/log.cpp.o: ../seaice/log.cpp
CMakeFiles/testIOM.dir/seaice/log.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/testIOM.dir/seaice/log.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/log.cpp.o -MF CMakeFiles/testIOM.dir/seaice/log.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/log.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/log.cpp

CMakeFiles/testIOM.dir/seaice/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/log.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/log.cpp > CMakeFiles/testIOM.dir/seaice/log.cpp.i

CMakeFiles/testIOM.dir/seaice/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/log.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/log.cpp -o CMakeFiles/testIOM.dir/seaice/log.cpp.s

CMakeFiles/testIOM.dir/seaice/mutex.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/mutex.cpp.o: ../seaice/mutex.cpp
CMakeFiles/testIOM.dir/seaice/mutex.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/testIOM.dir/seaice/mutex.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/mutex.cpp.o -MF CMakeFiles/testIOM.dir/seaice/mutex.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/mutex.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/mutex.cpp

CMakeFiles/testIOM.dir/seaice/mutex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/mutex.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/mutex.cpp > CMakeFiles/testIOM.dir/seaice/mutex.cpp.i

CMakeFiles/testIOM.dir/seaice/mutex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/mutex.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/mutex.cpp -o CMakeFiles/testIOM.dir/seaice/mutex.cpp.s

CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o: ../seaice/scheduler.cpp
CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o -MF CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/scheduler.cpp

CMakeFiles/testIOM.dir/seaice/scheduler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/scheduler.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/scheduler.cpp > CMakeFiles/testIOM.dir/seaice/scheduler.cpp.i

CMakeFiles/testIOM.dir/seaice/scheduler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/scheduler.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/scheduler.cpp -o CMakeFiles/testIOM.dir/seaice/scheduler.cpp.s

CMakeFiles/testIOM.dir/seaice/thread.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/thread.cpp.o: ../seaice/thread.cpp
CMakeFiles/testIOM.dir/seaice/thread.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/testIOM.dir/seaice/thread.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/thread.cpp.o -MF CMakeFiles/testIOM.dir/seaice/thread.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/thread.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/thread.cpp

CMakeFiles/testIOM.dir/seaice/thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/thread.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/thread.cpp > CMakeFiles/testIOM.dir/seaice/thread.cpp.i

CMakeFiles/testIOM.dir/seaice/thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/thread.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/thread.cpp -o CMakeFiles/testIOM.dir/seaice/thread.cpp.s

CMakeFiles/testIOM.dir/seaice/timer.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/timer.cpp.o: ../seaice/timer.cpp
CMakeFiles/testIOM.dir/seaice/timer.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/testIOM.dir/seaice/timer.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/timer.cpp.o -MF CMakeFiles/testIOM.dir/seaice/timer.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/timer.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/timer.cpp

CMakeFiles/testIOM.dir/seaice/timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/timer.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/timer.cpp > CMakeFiles/testIOM.dir/seaice/timer.cpp.i

CMakeFiles/testIOM.dir/seaice/timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/timer.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/timer.cpp -o CMakeFiles/testIOM.dir/seaice/timer.cpp.s

CMakeFiles/testIOM.dir/seaice/utils.cpp.o: CMakeFiles/testIOM.dir/flags.make
CMakeFiles/testIOM.dir/seaice/utils.cpp.o: ../seaice/utils.cpp
CMakeFiles/testIOM.dir/seaice/utils.cpp.o: CMakeFiles/testIOM.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/testIOM.dir/seaice/utils.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testIOM.dir/seaice/utils.cpp.o -MF CMakeFiles/testIOM.dir/seaice/utils.cpp.o.d -o CMakeFiles/testIOM.dir/seaice/utils.cpp.o -c /Users/seaicelin/Desktop/seaice/seaice/utils.cpp

CMakeFiles/testIOM.dir/seaice/utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testIOM.dir/seaice/utils.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/seaicelin/Desktop/seaice/seaice/utils.cpp > CMakeFiles/testIOM.dir/seaice/utils.cpp.i

CMakeFiles/testIOM.dir/seaice/utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testIOM.dir/seaice/utils.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/seaicelin/Desktop/seaice/seaice/utils.cpp -o CMakeFiles/testIOM.dir/seaice/utils.cpp.s

# Object files for target testIOM
testIOM_OBJECTS = \
"CMakeFiles/testIOM.dir/test/testIOM.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/config.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/fiber.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/hook.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/log.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/mutex.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/thread.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/timer.cpp.o" \
"CMakeFiles/testIOM.dir/seaice/utils.cpp.o"

# External object files for target testIOM
testIOM_EXTERNAL_OBJECTS =

bin/testIOM: CMakeFiles/testIOM.dir/test/testIOM.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/config.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/fd_manager.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/fiber.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/hook.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/iomanager.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/log.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/mutex.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/scheduler.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/thread.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/timer.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/seaice/utils.cpp.o
bin/testIOM: CMakeFiles/testIOM.dir/build.make
bin/testIOM: /usr/local/lib/libyaml-cpp.dylib
bin/testIOM: CMakeFiles/testIOM.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/seaicelin/Desktop/seaice/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Linking CXX executable bin/testIOM"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testIOM.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testIOM.dir/build: bin/testIOM
.PHONY : CMakeFiles/testIOM.dir/build

CMakeFiles/testIOM.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testIOM.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testIOM.dir/clean

CMakeFiles/testIOM.dir/depend:
	cd /Users/seaicelin/Desktop/seaice/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/seaicelin/Desktop/seaice /Users/seaicelin/Desktop/seaice /Users/seaicelin/Desktop/seaice/build /Users/seaicelin/Desktop/seaice/build /Users/seaicelin/Desktop/seaice/build/CMakeFiles/testIOM.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testIOM.dir/depend

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
CMAKE_SOURCE_DIR = /home/bobbywu/Desktop/Developer/libnet

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bobbywu/Desktop/Developer/libnet/build

# Include any dependencies generated for this target.
include example/CMakeFiles/echo_server.dir/depend.make

# Include the progress variables for this target.
include example/CMakeFiles/echo_server.dir/progress.make

# Include the compile flags for this target's objects.
include example/CMakeFiles/echo_server.dir/flags.make

example/CMakeFiles/echo_server.dir/EchoServer.cpp.o: example/CMakeFiles/echo_server.dir/flags.make
example/CMakeFiles/echo_server.dir/EchoServer.cpp.o: ../example/EchoServer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bobbywu/Desktop/Developer/libnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object example/CMakeFiles/echo_server.dir/EchoServer.cpp.o"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example && /bin/x86_64-linux-gnu-g++-9  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/echo_server.dir/EchoServer.cpp.o -c /home/bobbywu/Desktop/Developer/libnet/example/EchoServer.cpp

example/CMakeFiles/echo_server.dir/EchoServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/echo_server.dir/EchoServer.cpp.i"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bobbywu/Desktop/Developer/libnet/example/EchoServer.cpp > CMakeFiles/echo_server.dir/EchoServer.cpp.i

example/CMakeFiles/echo_server.dir/EchoServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/echo_server.dir/EchoServer.cpp.s"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bobbywu/Desktop/Developer/libnet/example/EchoServer.cpp -o CMakeFiles/echo_server.dir/EchoServer.cpp.s

# Object files for target echo_server
echo_server_OBJECTS = \
"CMakeFiles/echo_server.dir/EchoServer.cpp.o"

# External object files for target echo_server
echo_server_EXTERNAL_OBJECTS =

bin/echo_server: example/CMakeFiles/echo_server.dir/EchoServer.cpp.o
bin/echo_server: example/CMakeFiles/echo_server.dir/build.make
bin/echo_server: lib/liblibnet.a
bin/echo_server: example/CMakeFiles/echo_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bobbywu/Desktop/Developer/libnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/echo_server"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/echo_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
example/CMakeFiles/echo_server.dir/build: bin/echo_server

.PHONY : example/CMakeFiles/echo_server.dir/build

example/CMakeFiles/echo_server.dir/clean:
	cd /home/bobbywu/Desktop/Developer/libnet/build/example && $(CMAKE_COMMAND) -P CMakeFiles/echo_server.dir/cmake_clean.cmake
.PHONY : example/CMakeFiles/echo_server.dir/clean

example/CMakeFiles/echo_server.dir/depend:
	cd /home/bobbywu/Desktop/Developer/libnet/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bobbywu/Desktop/Developer/libnet /home/bobbywu/Desktop/Developer/libnet/example /home/bobbywu/Desktop/Developer/libnet/build /home/bobbywu/Desktop/Developer/libnet/build/example /home/bobbywu/Desktop/Developer/libnet/build/example/CMakeFiles/echo_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : example/CMakeFiles/echo_server.dir/depend


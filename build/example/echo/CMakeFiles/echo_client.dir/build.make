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
include example/echo/CMakeFiles/echo_client.dir/depend.make

# Include the progress variables for this target.
include example/echo/CMakeFiles/echo_client.dir/progress.make

# Include the compile flags for this target's objects.
include example/echo/CMakeFiles/echo_client.dir/flags.make

example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.o: example/echo/CMakeFiles/echo_client.dir/flags.make
example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.o: ../example/echo/EchoClient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bobbywu/Desktop/Developer/libnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.o"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example/echo && /bin/x86_64-linux-gnu-g++-9  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/echo_client.dir/EchoClient.cpp.o -c /home/bobbywu/Desktop/Developer/libnet/example/echo/EchoClient.cpp

example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/echo_client.dir/EchoClient.cpp.i"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example/echo && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bobbywu/Desktop/Developer/libnet/example/echo/EchoClient.cpp > CMakeFiles/echo_client.dir/EchoClient.cpp.i

example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/echo_client.dir/EchoClient.cpp.s"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example/echo && /bin/x86_64-linux-gnu-g++-9 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bobbywu/Desktop/Developer/libnet/example/echo/EchoClient.cpp -o CMakeFiles/echo_client.dir/EchoClient.cpp.s

# Object files for target echo_client
echo_client_OBJECTS = \
"CMakeFiles/echo_client.dir/EchoClient.cpp.o"

# External object files for target echo_client
echo_client_EXTERNAL_OBJECTS =

bin/echo_client: example/echo/CMakeFiles/echo_client.dir/EchoClient.cpp.o
bin/echo_client: example/echo/CMakeFiles/echo_client.dir/build.make
bin/echo_client: lib/liblibnet.a
bin/echo_client: example/echo/CMakeFiles/echo_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bobbywu/Desktop/Developer/libnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/echo_client"
	cd /home/bobbywu/Desktop/Developer/libnet/build/example/echo && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/echo_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
example/echo/CMakeFiles/echo_client.dir/build: bin/echo_client

.PHONY : example/echo/CMakeFiles/echo_client.dir/build

example/echo/CMakeFiles/echo_client.dir/clean:
	cd /home/bobbywu/Desktop/Developer/libnet/build/example/echo && $(CMAKE_COMMAND) -P CMakeFiles/echo_client.dir/cmake_clean.cmake
.PHONY : example/echo/CMakeFiles/echo_client.dir/clean

example/echo/CMakeFiles/echo_client.dir/depend:
	cd /home/bobbywu/Desktop/Developer/libnet/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bobbywu/Desktop/Developer/libnet /home/bobbywu/Desktop/Developer/libnet/example/echo /home/bobbywu/Desktop/Developer/libnet/build /home/bobbywu/Desktop/Developer/libnet/build/example/echo /home/bobbywu/Desktop/Developer/libnet/build/example/echo/CMakeFiles/echo_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : example/echo/CMakeFiles/echo_client.dir/depend

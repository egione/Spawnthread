/*
Spawn Library
Copyright 2016 Russell Leidich
http://spawnthread.blogspot.com

This collection of files constitutes the Spawn Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Spawn Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Spawn Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Spawn Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
Getting Started:

  See notes under linux_build64.sh below.

Files included:

  constant.h: Some boring constants for documentation value.

  COPYING: Licensing info.

  demo.c: Demo code. You can read the comments and watch it work. It serves as
  a template for how to build your own applications using Spawn.

  flag.h: Contains the build number and inspects commandline switches.

  linux64_build.sh: is for building on any flavor of GCC under 64-bit Linux. It
  will build 2 applications: "monothread_demo" and "multithread_demo". (You may
  see an error message about "cannot remove blah blah", because it tries to
  delete any previous instance  before compiling a new one.) You can then run
  the demos to see a fake global maximization problem being solved using single
  and multiple threads. (Note that you might need to run this script as:
  "/bin/sh ./linux64_build.sh".)

  linux32_build.sh: The 32-bit equivalent of linux64_build.sh

  unix_include.h: The place where we bring in UNIX primitives which are
  necessary to Spawn and/or the demo.

  spawn.c: Code which implents the multithreading macros, and their
  monothreaded aliases.

  spawn.h: Spawn include file.

  spawn_xtrn.h: C (extern)s required for using Spawn as an object file
  (makefile not included).

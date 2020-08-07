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
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PTHREAD
  #include <pthread.h>
#endif

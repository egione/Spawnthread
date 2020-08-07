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
/*
Build control. If possible, change the build using gcc command switches, and not by changing this file.
*/
#define SPAWN_BUILD_ID 14
#if !(defined(_32_)||defined(_64_))
  #error "Use 'gcc -D_64_' for 64-bit or 'gcc -D_32_' for 32-bit code."
#elif defined(_32_)&&defined(_64_)
  #error "You have defined both _32_ and _64_. Chose one only."
#endif
#if !(defined(PTHREAD)||defined(PTHREAD_OFF))
  #error "Use 'gcc -DPTHREAD' for multithreading or 'gcc -DPTHREAD_OFF' for monothreading."
#elif defined(PTHREAD)&&defined(PTHREAD_OFF)
  #error "You have defined both PTHREAD and PTHREAD_OFF. Chose one only."
#endif
#ifndef O_BINARY
  #define O_BINARY 0
#endif
#if defined(WINDOWS)
  #undef WINDOWS
#endif
#if defined(_WIN32)||defined(__WIN32__)||defined(_WIN64)||defined(__WIN64__)
  #define WINDOWS
#endif

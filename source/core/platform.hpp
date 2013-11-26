// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __CAESARIA_PLATFORM_H_INCLUDED__
#define __CAESARIA_PLATFORM_H_INCLUDED__

#if defined(__WIN32__) || defined(_WIN32)
  #define CAESARIA_PLATFORM_WIN
  #define CAESARIA_PLATFORM_WIN32
  #define CAESARIA_PLATFORM_NAME "win32"
#elif defined(WIN64) || defined(_WIN64)
  #define CAESARIA_PLATFORM_WIN
  #define CAESARIA_PLATFORM_WIN64
  #define CAESARIA_PLATFORM_NAME "win64"
#elif defined(__APPLE_CC__)
  #define CAESARIA_PLATFORM_UNIX
  #define CAESARIA_PLATFORM_MACOSX
  #define CAESARIA_PLATFORM_NAME "macosx"
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
  #define CAESARIA_PLATFORM_UNIX
  #define CAESARIA_PLATFORM_XBSD
  #define CAESARIA_PLATFORM_NAME "freebsd"
#else
  #define CAESARIA_PLATFORM_UNIX
  #define CAESARIA_PLATFORM_LINUX
  #define CAESARIA_PLATFORM_NAME "linux"
#endif

#endif //__CAESARIA_PLATFORM_H_INCLUDED__
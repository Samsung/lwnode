/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef AudioPlayer_H_
#define AudioPlayer_H_

// typedef int (*AudioPacketFinalizeCB)(void* user_data);
#include <iostream>
#include <cassert>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#ifdef NDEBUG
#define AUDIOPLAYER_DLOG_INFO(...)
#define AUDIOPLAYER_DLOG_WARN(...)
#define AUDIOPLAYER_DLOG_ERROR(...)

#define AUDIOPLAYER_ASSERT(assertion) ((void)0)

#else

#include <cassert>

#define AUDIOPLAYER_DLOG_INFO(...) fprintf(stdout, __VA_ARGS__);

#define AUDIOPLAYER_DLOG_WARN(...) fprintf(stdout, __VA_ARGS__);

#define AUDIOPLAYER_DLOG_ERROR(...) fprintf(stderr, __VA_ARGS__);

#define AUDIOPLAYER_ASSERT(assertion) assert(assertion);

#endif  // NDEBBUG

class AudioPlayer {
 public:
  virtual ~AudioPlayer(){};

  static AudioPlayer* GetInstance();
  static void RemoveInstance();

  virtual int Start() = 0;
  virtual int Write(const int16_t* samples, size_t samples_count) = 0;
  virtual int Close() = 0;

 protected:
  AudioPlayer(){};

  static AudioPlayer* CreateTizenAudioIO();
  static AudioPlayer* CreateTizenAudioPlayer();
};

#endif

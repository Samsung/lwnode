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

#include <stdio.h>
#include <assert.h>

#include "audio_player.h"
#include "audio_player_sample.h"

static AudioPlayer* g_audioPlayer;

int audio_player_start() {
  g_audioPlayer = AudioPlayer::GetInstance();
  g_audioPlayer->Start();
  return 0;
}

int audio_player_write(const int16_t* samples, size_t samples_count,
                       int* written_samples_count) {
  if (g_audioPlayer == nullptr) {
    return -1;
  }

  if (samples_count == 0) {
    return 0;
  }

  int ret = g_audioPlayer->Write(samples, samples_count);

  if (written_samples_count) {
    *written_samples_count = ret;
  }
  return ret;
}

int audio_player_close() {
  g_audioPlayer->Close();
  AudioPlayer::RemoveInstance();
  return 0;
}

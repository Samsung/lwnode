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

#include "audio_player.h"

static AudioPlayer* g_audioPlayerInstance;

AudioPlayer* AudioPlayer::GetInstance() {
  if (g_audioPlayerInstance == nullptr) {
    g_audioPlayerInstance = AudioPlayer::CreateTizenAudioIO();
  }
  return g_audioPlayerInstance;
}

void AudioPlayer::RemoveInstance() {
  if (g_audioPlayerInstance) {
    delete g_audioPlayerInstance;
    g_audioPlayerInstance = nullptr;
  }
}

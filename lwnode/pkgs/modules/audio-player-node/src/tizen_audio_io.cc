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

#include <audio_io.h>

#include "audio_player.h"

static const char* state_str[] = {"IDLE", "RUNNING", "PAUSED"};

class TizenAudioIO : public AudioPlayer {
 public:
  TizenAudioIO(){};
  virtual ~TizenAudioIO(){};

  int Start() override;
  int Write(const int16_t* samples, size_t samples_count) override;
  int Close() override;

 private:
  audio_out_h m_audioOut{nullptr};
  int m_buffer_size{0};
};

static void audio_out_state_cb(audio_out_h handle, audio_io_state_e previous,
                               audio_io_state_e current, bool by_policy,
                               void* user_data) {
  AUDIOPLAYER_DLOG_INFO(
      ">>> _audio_out_state_cb() : handle(%p), (%d,%s) => (%d,%s), "
      "by_policy(%d), user_data(%p)\n",
      handle, previous, state_str[previous], current, state_str[current],
      by_policy, user_data);
}

int TizenAudioIO::Start() {
  // 44.1Khz , 2 channel, 16bit PCM data
  audio_out_create_new(44100, AUDIO_CHANNEL_STEREO, AUDIO_SAMPLE_TYPE_S16_LE,
                       &m_audioOut);
  AUDIOPLAYER_ASSERT(m_audioOut);
  audio_out_set_state_changed_cb(m_audioOut, audio_out_state_cb, NULL);
  audio_out_prepare(m_audioOut);
  audio_out_get_buffer_size(m_audioOut, &m_buffer_size);
  return 0;
}

int TizenAudioIO::Write(const int16_t* samples, size_t samples_count) {
  AUDIOPLAYER_ASSERT(m_audioOut);
  AUDIOPLAYER_ASSERT(m_buffer_size >= 0);

  int ret = 0;

  const size_t buffer_bytes_size = samples_count * 2;
  auto adjusted_buffer_size = buffer_bytes_size;

  if ((size_t)m_buffer_size < buffer_bytes_size) {
    adjusted_buffer_size = m_buffer_size;
  } else {
    adjusted_buffer_size = buffer_bytes_size;
  }

  if ((ret = audio_out_write(m_audioOut, (void*)samples,
                             adjusted_buffer_size)) < 0) {
    AUDIOPLAYER_DLOG_ERROR("Error!\n");
    return -1;
  }
  return ret / 2;
}

int TizenAudioIO::Close() {
  if (m_audioOut) {
    audio_out_unprepare(m_audioOut);
    audio_out_destroy(m_audioOut);
    m_audioOut = nullptr;
  }
  return 0;
}

AudioPlayer* AudioPlayer::CreateTizenAudioIO() { return new TizenAudioIO(); }

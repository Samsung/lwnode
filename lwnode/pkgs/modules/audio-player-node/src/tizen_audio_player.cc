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

#include <media/player.h>

#include "audio_player.h"

static inline bool CheckPlayerAPIError(int err, std::string msg) {
  if (err == PLAYER_ERROR_NONE) {
    return true;
  } else if (err == PLAYER_ERROR_INVALID_PARAMETER) {
    AUDIOPLAYER_DLOG_ERROR("Invalid params: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_INVALID_STATE) {
    AUDIOPLAYER_DLOG_ERROR("Invalid state: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_NOT_SUPPORTED_FILE) {
    AUDIOPLAYER_DLOG_ERROR("Not supported file: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_BUFFER_SPACE) {
    AUDIOPLAYER_DLOG_ERROR("Buffer space: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_OUT_OF_MEMORY) {
    AUDIOPLAYER_DLOG_ERROR("Out of memory: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_INVALID_OPERATION) {
    AUDIOPLAYER_DLOG_ERROR("Invalid operation: %s\n", msg.data());
  } else if (err == PLAYER_ERROR_RESOURCE_LIMIT) {
    AUDIOPLAYER_DLOG_ERROR("Resource limited: %s\n", msg.data());
  } else {
    AUDIOPLAYER_DLOG_ERROR("Unknown error: %s\n", msg.data());
  }
  return false;
}

static inline bool CheckFormatAPIError(int err, std::string msg) {
  if (err == MEDIA_FORMAT_ERROR_NONE) {
    return true;
  } else if (err == MEDIA_FORMAT_ERROR_NONE) {
    AUDIOPLAYER_DLOG_ERROR("Invalid params: %s\n", msg.data());
  } else if (err == MEDIA_FORMAT_ERROR_NONE) {
    AUDIOPLAYER_DLOG_ERROR("Invalid operation: %s\n", msg.data());
  } else {
    AUDIOPLAYER_DLOG_ERROR("Unknown error: %s\n", msg.data());
  }
  return false;
}

static inline bool CheckPacketAPIError(int err, std::string msg) {
  if (err == MEDIA_PACKET_ERROR_NONE) {
    return true;
  } else if (err == MEDIA_PACKET_ERROR_INVALID_PARAMETER) {
    AUDIOPLAYER_DLOG_ERROR("Invalid Params: %s\n", msg.data());
  } else if (err == MEDIA_PACKET_ERROR_OUT_OF_MEMORY) {
    AUDIOPLAYER_DLOG_ERROR("Out of memory: %s\n", msg.data());
  } else if (err == MEDIA_PACKET_ERROR_INVALID_OPERATION) {
    AUDIOPLAYER_DLOG_ERROR("Invalid operation: %s\n", msg.data());
  } else {
    AUDIOPLAYER_DLOG_ERROR("Unknown error: %s\n", msg.data());
  }

  return false;
}

static const size_t s_mediaPlayerAudioBufferSize = 2048;

class TizenAudioPlayer : public AudioPlayer {
 public:
  TizenAudioPlayer();
  virtual ~TizenAudioPlayer();

  int Prepare();
  int Play();

  int Start() override;
  int Write(const int16_t* samples, size_t samples_count) override;
  int Close() override;

 private:
  player_h m_player{nullptr};
  media_format_h m_format{nullptr};
};

TizenAudioPlayer::TizenAudioPlayer() : AudioPlayer() {
  CheckPlayerAPIError(player_create(&m_player), "Cannot create player!");

  CheckPlayerAPIError(
      player_set_media_stream_buffer_max_size(
          m_player, PLAYER_STREAM_TYPE_AUDIO, s_mediaPlayerAudioBufferSize),
      "Cannot set buffer size");

  CheckFormatAPIError(media_format_create(&m_format),
                      "Cannot create media format!");
  CheckFormatAPIError(
      media_format_set_audio_mime(m_format, MEDIA_FORMAT_PCM_S16LE),
      "Cannot set mime\n");
  CheckFormatAPIError(media_format_set_audio_channel(m_format, 2),
                      "Cannot set audio channel");
  CheckFormatAPIError(media_format_set_audio_samplerate(m_format, 44100),
                      "Cannot set audio samplerate");
  CheckPlayerAPIError(player_set_media_stream_info(
                          m_player, PLAYER_STREAM_TYPE_AUDIO, m_format),
                      "Cannot set media stream info");
}

TizenAudioPlayer::~TizenAudioPlayer() {}

int TizenAudioPlayer::Prepare() {
  if (!CheckPlayerAPIError(player_prepare_async(
                               m_player,
                               [](void* user_data) {
                                 AUDIOPLAYER_DLOG_INFO("Done prepare\n");
                                 TizenAudioPlayer* self =
                                     static_cast<TizenAudioPlayer*>(user_data);
                                 self->Play();
                               },
                               this),
                           "Cannot prepare")) {
    return -1;
  }
  return 0;
}

int TizenAudioPlayer::Play() {
  player_state_e state;
  if (!CheckPlayerAPIError(player_get_state(m_player, &state),
                           "Cannot get state")) {
    return -1;
  }
  if (state == PLAYER_STATE_PLAYING) {
    AUDIOPLAYER_DLOG_INFO("Already play\n");
    return -1;
  }

  if (!CheckPlayerAPIError(player_start(m_player), "Cannot play")) {
    return -1;
  }
  AUDIOPLAYER_DLOG_INFO("Success play\n");
  return 0;
}

int TizenAudioPlayer::Start() { return Prepare(); }

int TizenAudioPlayer::Write(const int16_t* samples, size_t samples_count) {
  player_state_e state;
  player_get_state(m_player, &state);
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    AUDIOPLAYER_DLOG_INFO("Not ready player\n");
    return 0;
  }
  media_packet_h mediaPacket = nullptr;

  if (!CheckPacketAPIError(
          media_packet_create_from_external_memory(
              m_format, (void*)samples, (uint64_t)(samples_count * 2), nullptr,
              nullptr, &mediaPacket),
          "Cannot create media packet")) {
    return 0;
  }

  if (!CheckPlayerAPIError(player_push_media_stream(m_player, mediaPacket),
                           "Cannot push media stream")) {
    return 0;
  }

  media_packet_destroy(mediaPacket);
  return (int)samples_count;
}

int TizenAudioPlayer::Close() {
  if (m_player) {
    player_state_e state;
    player_get_state(m_player, &state);
    if (state == PLAYER_STATE_PLAYING) {
      player_stop(m_player);
    }

    CheckPlayerAPIError(player_unprepare(m_player), "playerUnprepare");
    CheckPlayerAPIError(player_destroy(m_player), "playerDestroy");
    CheckFormatAPIError(media_format_unref(m_format), "mediaFormatUnref");

    m_player = nullptr;
  }
  return 0;
}

AudioPlayer* AudioPlayer::CreateTizenAudioPlayer() {
  return new TizenAudioPlayer();
}

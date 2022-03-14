#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <assert.h>
#include <cstring>

#include "bridge.h"
#include "service.h"

#include "audio_player_sample.h"

void wrapper_on_event(const char* evt, const char* ns, void* userdata) {
  std::cout << "TvBrdige OnEvent ns:" << (ns ? ns : "(null)")
            << " event: " << evt << std::endl;
}

void wrapper_on_playback_event(sp_bridge_playback_event evt,
                               sp_bridge_pb_param* param, void* userdata) {
  std::cout << "TvBrdige OnPlaybackEvent: " << static_cast<int>(evt)
            << std::endl;
}

size_t wrapper_music_delivery(const int16_t* samples, size_t sample_count,
                              const struct sp_bridge_audioformat* sample_format,
                              uint32_t* samples_buffered, void* userdata) {
  std::cout << "TvBridge OnMusicDelivery samples:" << sample_count << std::endl;

  int ret, written_sample_count = 0;

  if (sample_count > 0) {
    if ((ret = audio_player_write(samples, sample_count,
                                  &written_sample_count)) < 0) {
      return 0;
    }
  }

  if (samples_buffered) {
    *samples_buffered = 0;
  }

  return written_sample_count;
}

void wrapper_on_log_print(const char* message, void* userdata) {
  std::cout << message << std::flush;
}

void assignCallbacks(sp_bridge_callbacks& cb) {
  memset(&cb, 0, sizeof(cb));
  cb.on_event = wrapper_on_event;
  cb.on_playback_event = wrapper_on_playback_event;
  cb.music_delivery = wrapper_music_delivery;
  cb.log_print = wrapper_on_log_print;
}

int service_start(const char* in_username, const char* in_password) {
  // Start audio player
  audio_player_start();

  // config
  sp_bridge_config m_cfg;
  sp_bridge_callbacks m_callbacks;
  const char* output;

  memset(&m_cfg, 0, sizeof(m_cfg));
  m_cfg.unique_id = "sample-unique-id";
  m_cfg.brand_name = "sample-brand-name";
  m_cfg.model_name = "sample-model-name";
  m_cfg.display_name = "sample-display-name";
  m_cfg.api_version = BRIDGE_API_VERSION;
  m_cfg.device_type = 5;
  m_cfg.userdata = NULL;
  m_cfg.volume = 65535;
  m_cfg.client_id = "a533a2c29aec473f992ba9c28ed3fea0";  // Mock TV Platform
  m_cfg.zeroconf_enabled = 1;

  assignCallbacks(m_callbacks);
  m_cfg.callbacks = &m_callbacks;

  // create bridge
  auto result = sp_bridge_create(&m_cfg, NULL);
  assert(result == SP_BRIDGE_ERROR_OK);

  // get bridge version
  std::string version_cmd = "{\"func\":21, \"args\":[], \"msgId\":0}";
  result = sp_bridge_execute(version_cmd.c_str(), &output);
  assert(result == SP_BRIDGE_ERROR_OK);
  std::cout << "sp_bridge_execute output: " << output << std::endl;
  std::cout << "sp_bridge_execute output: " << output << std::endl;
  std::cout << "sp_bridge_execute output: " << output << std::endl;

  // login to bridge
  std::string username = in_username;
  std::string password = in_password;
  std::string login_cmd = "{\"func\":0, \"args\":[\"" + username + "\", \"" +
                          password + "\"], \"msgId\":0}";

  result = sp_bridge_execute(login_cmd.c_str(), &output);
  assert(result == SP_BRIDGE_ERROR_OK);
  std::cout << "sp_bridge_execute output: " << output << std::endl;
  std::cout << "sp_bridge_execute output: " << output << std::endl;
  std::cout << "sp_bridge_execute output: " << output << std::endl;

  // let bridge run for some time
  // std::this_thread::sleep_for(std::chrono::seconds(300));

  // release TvBridge
  // result = sp_bridge_release();
  // assert(result == SP_BRIDGE_ERROR_OK);

  return 0;
}

int service_stop() {
  // release TvBridge
  auto result = sp_bridge_release();
  assert(result == SP_BRIDGE_ERROR_OK);

  audio_player_close();
  return 0;
}

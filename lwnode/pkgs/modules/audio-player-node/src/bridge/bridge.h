/*
 * Copyright (c) 2006-2014 Spotify AB
 *
 */

/**
 * \file   bridge.h    Public bridge API for eSDK.
 *
 * \note   All input strings are expected to be in JSON format
 * \note   All output strings are in JSON format
 *
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SP_LIBEXPORT
#define SP_LIBEXPORT(x) x
#endif //SP_LIBEXPORT

/* Includes */
#include <stddef.h>
#include <stdint.h>

/**
 * \brief Current version of the application interface, that is, the API described
 * by this file.
 *
 * This value should be set in the sp_bridge_config struct passed to sp_bridge_create().
 *
 * If an (upgraded) library is no longer compatible with this version the error
 * #SP_BRIDGE_ERROR_BAD_API_VERSION will be returned from sp_bridge_create().
 * Future versions of the library will provide you with some kind of mechanism
 * to request an updated version of the library.
 */
#define BRIDGE_API_VERSION 21

/**
 * \brief Maximum length of the device's unique ID (not counting terminating NULL)
 * \see sp_bridge_config.
 */
#define SP_BRIDGE_MAX_UNIQUE_ID_LENGTH (64)

/**
 * \brief Maximum length of the device display name (not counting terminating NULL)
 * \see sp_bridge_config.
 */
#define SP_BRIDGE_MAX_DISPLAY_NAME_LENGTH (64)

/**
 * \brief Maximum length of the brand name string (not counting terminating NULL)
 * \see sp_bridge_config
 */
#define SP_BRIDGE_MAX_BRAND_NAME_LENGTH (32)

/**
 * \brief Maximum length of the model name string (not counting terminating NULL)
 * \see sp_bridge_config
 */
#define SP_BRIDGE_MAX_MODEL_NAME_LENGTH (30)

/**
 * \brief The default scope to request for ZeroConf access tokens
 * \see sp_bridge_config
 */
#define SP_BRIDGE_SCOPE_DEFAULT "umbrella-tv,streaming,client-authorization-universal"


/**
 * \brief Error codes
 *
 * These are the possible status codes returned by functions in the bridge. They
 * should be used to determine if an action was successful, and if not, why the
 * action failed.
 */
typedef enum sp_bridge_error {
  /**
   * \brief No errors encountered.
   */
  SP_BRIDGE_ERROR_OK                        = 0,

  /**
   * \brief The library version targeted does not match the version passed in
   * sp_bridge_config::api_version.
   */
  SP_BRIDGE_ERROR_BAD_API_VERSION           = 1,

  /**
   * \brief Initialization of library failed - check sp_bridge_config parameters
   *  passed to sp_bridge_create().
   */
  SP_BRIDGE_ERROR_API_INITIALIZATION_FAILED = 2,

  /**
   * \brief Input data was either missing or invalid.
   */
  SP_BRIDGE_ERROR_INVALID_INDATA            = 3,

  /**
   * \brief An invalid argument was specified.
   */
  SP_BRIDGE_ERROR_INVALID_ARGUMENT          = 4,

  /**
   * \brief An operating system error.
   */
  SP_BRIDGE_ERROR_SYSTEM_FAILURE            = 5,

  /**
   * \brief One or more callbacks were not specified.
   */
  SP_BRIDGE_ERROR_MISSING_CALLBACK          = 6,
} sp_bridge_error;

/**
 * \brief Audio format
 */
typedef struct sp_bridge_audioformat {
  /** \brief Number of channels (1 = mono, 2 = stereo) */
  int channels;

  /** \brief Sample rate in Hz (such as 22050, 44100 or 48000) */
  int sample_rate;
} sp_bridge_audioformat;

/**
 * \brief Playback events.
 */
typedef enum sp_bridge_playback_event {
  /**
   * \brief Start delivery samples from your internal buffer to the audio-out.
   */
  SP_BRIDGE_PLAYBACK_EVENT_PLAY = 0,

  /**
   * \brief Stop delivery samples from your internal buffer to the audio-out.
   */
  SP_BRIDGE_PLAYBACK_EVENT_PAUSE = 1,

  /**
   * \brief Flush your internal buffer.
   */
  SP_BRIDGE_PLAYBACK_EVENT_FLUSH = 2,

  /**
   * \brief User want to change the volume level.
   */
  SP_BRIDGE_PLAYBACK_EVENT_VOLUME = 3,

  /**
   * \brief A seek operation started.
   * Some implementation might need this information.
   */
  SP_BRIDGE_PLAYBACK_EVENT_SEEK = 4,
} sp_bridge_playback_event;

/**
 * \brief Volume event parameters.
 *
 * see \a on_playback_event
 *
 */
struct pb_volume_param {

  /*
   * \brief Volume level in the range [0..65535]
   */
  uint16_t volume_level;

  /*
   * \brief Indicates whether volume change was originated remotely [0..1]
   */
  uint8_t remote;
};

/**
 * \brief Seek event parameters.
 *
 * see \a on_playback_event
 *
 */
struct pb_seek_param {

  /*
   * \brief Position in ms the user seeked to.
   */
  uint32_t position_ms;
};

/*
 * \brief common data structure for on_playback events.
 *
 * see \a on_playback_event
 */
typedef struct {
  union {
    /*
     * valid only for SP_BRIDGE_PLAYBACK_EVENT_VOLUME event
     */
    struct pb_volume_param volume;

    /*
     * valid only for SP_BRIDGE_PLAYBACK_EVENT_SEEK event
     */
    struct pb_seek_param seek;
  } u;
} sp_bridge_pb_param;

/**
 * Bridge callbacks
 *
 * Registered when you create the bridge.
 */
typedef struct sp_bridge_callbacks {

  /**
   * \brief A bridge event occurred.
   *
   * \param[in]  evt JSON structure describing the event. The user has to copy the
   *             data in evt if it wishes to retain it.
   * \param[in]  ns name of the namspace that generated the event or NULL if evt must
   *             be passed to the upper layer.
   * \param[in]  userdata Data supplied by the user in the sp_bridge_config structure
   *
   */
  void (*on_event)(const char *evt, const char *ns, void *userdata);

  /**
   * \brief An action from the audio driver is required.
   *
   * \param[in]  evt one of the playback event specified in \a sp_bridge_playback_event.
   * \param[in]  userdata Data supplied by the user in the \a sp_bridge_config structure
   *
   */
  void (*on_playback_event)(sp_bridge_playback_event evt, sp_bridge_pb_param *param, void *userdata);

  /**
   * \brief Callback for sending audio data to the application
   *
   * \param[in] samples Pointer to 16-bit PCM data. The buffer contains
   *              \a sample_count samples, whereby each sample contains the data
   *              for a single audio channel.
   * \param[in] sample_count Number of samples in the \a samples buffer. This is
   *              always a multiple of \a sample_format->channels.
   * \param[in] sample_format Information about the format of the audio data.
   *              See the note below.
   * \param[out] samples_buffered The number of samples that the application
   *              has received but that have not been played yet. See the note below.
   * \param[in] userdata Data supplied by the user in the sp_bridge_config structure
   * \return The number of samples that the application accepted.
   *
   * \note \a sample_format can change at any time as playback goes from one track
   * to the next. The application should check the format during every invocation
   * of the callback and reinitialize the audio pipeline if necessary.
   * \par
   *
   * \note It is important to return an accurate value in \a samples_buffered.
   * The library uses this value to calculate an accurate playback position within
   * the track. For example, if the library has delivered 1.5 seconds of audio data,
   * but the application is buffering half a second of audio data, the actual
   * playback position is 1 second.
   *  - By default, when \a samples_buffered is 0, the library will calculate the
   *    playback position and the notifications that are sent when playback reaches
   *    the end of a track based on the amount of audio data that was delivered in
   *    music_delivery().
   *  - However, audio data is delivered faster than the playback happens (1.5 times
   *    playback speed). This means, as long as the application accepts samples in
   *    music_delivery(), these samples will be calculated as "consumed" and
   *    the playback position will move ahead. Eventually, the last sample for a track
   *    will be delivered in music_delivery().
   *  - To adjust playback position and notifications for the amount of data that the
   *    application has buffered but that have not been played yet, the application
   *    should set \a samples_buffered to the amount of samples in all buffers. This
   *    will make sure that the internal position is adjusted, the notifications
   *    will be sent at the correct time to the upper layers.
   *  - If the application has buffered data, music_delivery() may be invoked with
   *    \a samples_count = 0 after all audio data has been delivered by the library.
   *    In this case, the application should make sure to update \a samples_buffered
   *    (since the application's buffers are draining) and to return 0 from the
   *    callback.
   * \par
   *
   * \note Most tracks are stereo, 44100 Hz, so it is a good idea to initialize
   * the audio pipeline to this format when the application starts.
   * \par
   *
   * \note The application should not block or call other API functions in the
   * callback.
   */
  size_t (*music_delivery)(const int16_t *samples, size_t sample_count,
      const struct sp_bridge_audioformat *sample_format,
      uint32_t *samples_buffered, void *userdata);

  /**
   * \brief Callback for sending compressed audio data (OGG Vorbis) to the application
   *
   * This callback is invoked as soon as compressed ogg bits are available.
   *
   * Ogg headers are transmitted once per track, and are always at offset 0.
   * The headers must be parsed for the track format.  In addition to the three
   * mandatory Ogg headers, there is an optional fourth Spotify-specific Ogg
   * header that is sent as the first header of a track.  This header should
   * simply be skipped.
   *
   * The return value is a boolean response to whether the data was accepted.  It is
   * not supported to accept only part of the buffer.  If there is not room to store
   * or process the entire buffer, all data should be rejected.  The data will be
   * redelivered later.
   *
   * \param[in] data Block of compressed Ogg Vorbis data.
   * \param[in] size Size of the data buffer.
   * \param[in] offset Offset of the data buffer within the track (in bytes).
   If offset is 0, it is the beginning of an Ogg track and contains
   Ogg headers.  These must be parsed for the track format.
   * \param[in] userdata Data supplied by the user in the sp_bridge_config structure
   * \return 0 if data was rejected, non-zero if all data was accepted.
   */
  uint8_t (*compress_delivery)(const void *data, uint32_t size,
      uint32_t offset, void *context);

  /**
   * \brief Callback for reporting the current playback position to the Spotify library
   *
   * This callback is invoked periodically by the Spotify library to request the
   * current playback position, in milliseconds.  It is up to the integrator to
   * track how much of the audio has played and report that here as accurately
   * as possible.
   *
   * The reported time should represent the amount of audio that has actually
   * played, not the amount that is buffered.
   *
   * \param[out] position_ms Playback position, in milliseconds, of current track
   * \param[in] context Context pointer that was passed when registering the callback
   */
  void (*stream_position)(uint32_t *position_ms, void *context);

  /**
   * \brief Callback for platform specific tasks upon the Spotify thread start.
   *
   * The callback will be called immediately after the Spotify thread will be
   * started and as all the other callbacks it will be called from the thread.
   *
   * \param[in] userdata Data supplied by the user in the sp_bridge_config structure
   */
  void (*on_thread_started)(void *userdata);

  /**
   * \brief Callback for platform logging.
   *
   * This callback will be invoked for every log message originated from eSDK or from the TvBridge.
   * Used only for debugging purposes. If no callback is speificed and logging is enabled 
   * it will go to stdout. Max message length is 2048 bytes including null termination
   *
   * \param[in] message Debug message
   * \param[in] userdata Data supplied by the user in the sp_bridge_config structure
   */
  void (*log_print)(const char *message, void *userdata);
} sp_bridge_callbacks;


/**
 * \brief Configuration
 * \see sp_bridge_create
 */
typedef struct sp_bridge_config {
  /**
   * \brief The version of the API contained in this header file. Must be
   *        set to BRIDGE_API_VERSION.
   */
  int api_version;

  /**
   * \brief The Spotify Application Key for the application/device.
   *
   * See the Spotify Developer web site for information about Application Keys.
   */
  const uint8_t *application_key;

  /**
   * \brief Size of the buffer pointed to by \a app_key
   */
  size_t application_key_size;

  /**
   * \brief Size of the \a memory_block buffer in bytes
   */
  uint32_t memory_block_size;

  /**
   * \brief A NULL-terminated character string that uniquely identifies the
   *        device (such as a MAC address)
   *
   * The string will be truncated to SP_MAX_UNIQUE_ID_LENGTH characters,
   * not counting the terminating NULL.
   *
   * The library may use this to distinguish this device from other Spotify
   * Connect-enabled devices that the users has. On any given device, the
   * ID should not change between calls to sp_bridge_create().
   *
   * \warning If the unique ID collides with other devices that the user has,
   * the device might not be usable with Spotify Connect. Therefore, it is
   * important to minimize the chance of such collisions while still making
   * sure that the unique ID does not change between sessions. (A MAC address
   * or the device's serial number should work well. The device's model name
   * or its IP address will not work.)
   *
   * sp_bridge_create() returns SP_BRIDGE_ERROR_API_INITIALIZATION_FAILED if
   * this is not specified.
   */
  const char *unique_id;

  /**
   * \brief A UTF-8-encoded display name for the application/device
   *
   * When using Spotify Connect, this is the name that the Spotify app
   * will use in the UI to refer to this instance of the application/this device.
   *
   * The string will be truncated to SP_MAX_DISPLAY_NAME_LENGTH bytes
   * (not UTF-8-encoded characters), not counting the terminating NULL.
   *
   * The display name can be changed later with SpSetDisplayName().
   *
   * sp_bridge_create() returns SP_BRIDGE_ERROR_API_INITIALIZATION_FAILED if
   * this is not specified.
   */
  const char *display_name;

  /**
   * \brief A NULL-terminated string containing the brand name of the hardware
   *        device (for hardware integrations)
   *
   * This should be an ASCII string containing only letters, digits,
   * "_" (underscore), "-" (hyphen), and "." (period).
   *
   * sp_bridge_create() returns SP_BRIDGE_ERROR_API_INITIALIZATION_FAILED if
   * this is not specified.
   */
  const char *brand_name;

  /**
   * \brief A NULL-terminated string containing the model name of the hardware
   *        device (for hardware integrations)
   *
   * This should be an ASCII string containing only letters, digits,
   * "_" (underscore), "-" (hyphen), and "." (period).
   *
   * sp_bridge_create() returns SP_BRIDGE_ERROR_API_INITIALIZATION_FAILED if
   * this is not specified.
   */
  const char *model_name;

  /**
   * \brief The device type that best describes this product
   *
   * This device type will be reported to client applications and might
   * result in a suitable icon being shown, etc.
   */
  int device_type;

  /**
   * \brief Callbacks that will be invoked when events in the library occur.
   */
  const sp_bridge_callbacks *callbacks;

  /**
   * \brief Context pointer that will passed as input to any
   * callback.
   */
  void *userdata;

  /**
   * \brief volume level in the range [0..65535]
   */
  uint16_t volume;

  /**
   * \brief reserved for future use.
   * must be 0.
   */
  int reserved;

  /**
   * \brief The number of volume steps the device can support.
   *   - 0 will set an internal default value.
   *   - The max value that is possible to set is 65535.
   *   - -1 means the volume cannot be controlled remotely through
   *     the Spotify Connect feature.
   */
  int volume_steps;

  /**
   * \brief A NULL-terminated string containing the client id of the application
   *
   * The Client ID identifies the application using Spotify, Register your
   * application <a href="https://developer.spotify.com/my-applications/#!/">here</a>.
   *
   * This can be an ASCII string containing only hexadecimal characters, or NULL
   *
   * sp_bridge_create() returns SP_BRIDGE_ERROR_INVALID_ARGUMENT if this is invalid or longer than
   * 32 characters.
   */
  const char *client_id;

  /**
   * \brief A NULL-terminated string containing the OAuth scope requested when
   *        authenticating with the Spotify backend.
   *
   * This can be a comma-separated string of Spotify scopes, or NULL (which
   * would mean the default SP_BRIDGE_SCOPE_DEFAULT)
   *
   */
  const char *scope;

  /**
   * \brief dynamcially disable zeroconf discovery (-1 = disable)
   */
  int zeroconf_enabled;
} sp_bridge_config;

/**
 * \brief Initialize the bridge.
 *
 * \param[in]  config    The configuration to use for bridge
 * \param[in]  appconfig The configuration passed down by the upper layer.
 *                       The upper layer can override one or more fields of
 *                       the sp_bridge_config structure.
 *
 * \return               An SP_BRIDGE_ERROR code
 *
 */
SP_LIBEXPORT(sp_bridge_error) sp_bridge_create(sp_bridge_config *config, const char *appconfig);

/**
 * \brief Execute a command.
 *
 * \param[in]   input     A JSON structure describe the command that should be executed.
 * \param[out]  output    The output of the command. The allocated buffer is valid until the next
 *                        call to the bridge. The bridge is responsible for freeing the memory.
 *
 * \return                An SP_BRIDGE_ERROR code
 *
 */
SP_LIBEXPORT(sp_bridge_error) sp_bridge_execute(const char *input, const char **output);

/**
 * \brief Release the bridge.
 *
 * \return                An SP_BRIDGE_ERROR code
 */
SP_LIBEXPORT(sp_bridge_error) sp_bridge_release(void);

/**
 * \brief Update the Spotify Connect volume level.
 *
 * \param[in]   volume    The current volume level in the range [0..65535]
 *
 * \return                An SP_BRIDGE_ERROR code
 */
SP_LIBEXPORT(sp_bridge_error) sp_bridge_apply_volume(uint16_t volume);


#ifdef __cplusplus
}
#endif

#endif

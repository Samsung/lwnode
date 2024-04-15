/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TizenInputDeviceKeyMap__
#define __TizenInputDeviceKeyMap__

namespace DeviceAPI {

// tizen, kernal, webapi name, webapi code
#define FOR_EACH_TIZEN_KEY_MAP_TV(F)                                           \
  F("0", 11, "0", 48)                                                          \
  F("1", 2, "1", 49)                                                           \
  F("2", 3, "2", 50)                                                           \
  F("3", 4, "3", 51)                                                           \
  F("4", 5, "4", 52)                                                           \
  F("5", 6, "5", 53)                                                           \
  F("6", 7, "6", 54)                                                           \
  F("7", 8, "7", 55)                                                           \
  F("8", 9, "8", 56)                                                           \
  F("9", 10, "9", 57)                                                          \
  F("Minus", 22, "minus", 189)                                                 \
  F("XF86AudioRaiseVolume", 68, "VolumeUp", 447)                               \
  F("XF86AudioLowerVolume", 67, "VolumeDown", 448)                             \
  F("XF86AudioMute", 66, "VolumeMute", 449)                                    \
  F("XF86RaiseChannel", 88, "ChannelUp", 427)                                  \
  F("XF86LowerChannel", 87, "ChannelDown", 428)                                \
  F("XF86Red", 59, "ColorF0Red", 403)                                          \
  F("XF86Green", 60, "ColorF1Green", 404)                                      \
  F("XF86Yellow", 61, "ColorF2Yellow", 405)                                    \
  F("XF86Blue", 62, "ColorF3Blue", 406)                                        \
  F("XF86SysMenu", 125, "Menu", 10133)                                         \
  F("XF86SimpleMenu", 127, "Tools", 10135)                                     \
  F("XF86Info", 188, "Info", 457)                                              \
  F("XF86Exit", 174, "Exit", 10182)                                            \
  F("XF86Search", 217, "Search", 10225)                                        \
  F("XF86ChannelGuide", 130, "Guide", 458)                                     \
  F("XF86AudioRewind", 168, "MediaRewind", 412)                                \
  F("XF86AudioPause", 201, "MediaPause", 19)                                   \
  F("XF86AudioNext", 208, "MediaFastForward", 417)                             \
  F("XF86AudioRecord", 167, "MediaRecord", 416)                                \
  F("XF86AudioPlay", 200, "MediaPlay", 415)                                    \
  F("XF86AudioStop", 166, "MediaStop", 413)                                    \
  F("XF86PlayBack", 244, "MediaPlayPause", 10252)                              \
  F("XF86PreviousChapter", 224, "MediaTrackPrevious", 10232)                   \
  F("XF86NextChapter", 225, "MediaTrackNext", 10233)                           \
  F("XF86Display", 64, "Source", 10072)                                        \
  F("XF86PictureSize", 132, "PictureSize", 10140)                              \
  F("XF86PreviousChannel", 182, "PreviousChannel", 10190)                      \
  F("XF86ChannelList", 65, "ChannelList", 10073)                               \
  F("XF86EManual", 138, "E-Manual", 10146)                                     \
  F("XF86MTS", 187, "MTS", 10195)                                              \
  F("XF863D", 191, "3D", 10199)                                                \
  F("XF86SoccerMode", 220, "Soccer", 10228)                                    \
  F("XF86Caption", 213, "Caption", 10221)                                      \
  F("XF86TTXMIX", 192, "Teletext", 10200)                                      \
  F("XF86ExtraApp", 245, "Extra", 10253)

}  // namespace DeviceAPI

#endif

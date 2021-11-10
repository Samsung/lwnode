/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

// @note 'binding' is process module binding
function createProcessEntry(binding) {
  function _internalLog(...args) {
    if (binding.print) {
      binding.print.apply(null, args);
    }
  }

  return {
    _print: (...args) => {
      _internalLog.apply(null, arguments);
    },
    _ptr: (...args) => {
      if (isDebugBuild && binding.print) {
        binding.print.ptr.apply(null, args);
      }
    },
    _stack: (...args) => {
      if (isDebugBuild && binding.print) {
        binding.print.stack.apply(null, args);
      }
    },
    PssUsage: (...args) => {
      if (binding.PssUsage) {
        return binding.PssUsage.apply(null, args);
      }
    },
    PssSwapUsage: (...args) => {
      if (binding.PssSwapUsage) {
        return binding.PssSwapUsage.apply(null, args);
      }
    },
    RssUsage: (...args) => {
      if (binding.PssUsage) {
        return binding.RssUsage.apply(null, args);
      }
    },
    MemSnapshot: (...args) => {
      if (binding.MemSnapshot) {
        return binding.MemSnapshot.apply(null, args);
      }
    },
    isReloadScriptEnabled: () => {
      return !!binding.CreateReloadableSourceFromFile;
    },
    CreateReloadableSourceFromFile: (...args) => {
      if (binding.CreateReloadableSourceFromFile) {
        return binding.CreateReloadableSourceFromFile.apply(null, args);
      }
    },
  };
}

module.exports = {
  createProcessEntry,
};

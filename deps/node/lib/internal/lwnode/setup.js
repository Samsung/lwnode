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

const { MemWatcher, MemDiff } = require("internal/lwnode/memory");

const useEscargot = process.config.variables.javascript_engine == "escargot";
const isDebugBuild =
  process.config.target_defaults.default_configuration == "Debug";

// @note using `process.features` is considerable
const disabledFeatures = new Set(["WASI"]);

// @note 'binding' is process module binding
function wrapLWNodeMethods(binding) {
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
    MemWatcher,
    MemDiff,
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
    checkIfHandledAsOneByteString: (...args) => {
      if (binding.checkIfHandledAsOneByteString) {
        return binding.checkIfHandledAsOneByteString.apply(null, args);
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
    isEnabledFeature: (name) => {
      if (useEscargot == false) {
        return true;
      }

      let enabled = !disabledFeatures.has(name);
      _internalLog(`feature '${name}': ${enabled}`);
      return enabled;
    },
  };
}

module.exports = {
  wrapLWNodeMethods,
};

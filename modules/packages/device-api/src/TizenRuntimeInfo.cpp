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

#include "TizenRuntimeInfo.h"
#include <pkgmgr-info.h>
#include <tpk_manifest_handlers/privileges_handler.h>
#include <tpk_manifest_handlers/tpk_config_parser.h>
#include <memory>
#include <sstream>
#include "TizenDeviceAPIBase.h"
#include "lwnode/lwnode.h"

namespace DeviceAPI {

TizenRuntimeInfo* TizenRuntimeInfo::getInstance() {
  static TizenRuntimeInfo s_instance;
  return &s_instance;
}

TizenRuntimeInfo::TizenRuntimeInfo() {
  initializeDefaultVariable();
}

void TizenRuntimeInfo::initializeDefaultVariable() {
  std::string appid;
  if (!LWNode::SystemInfo::getInstance()->get("appid", appid)) {
    DEVICEAPI_LOG_ERROR("cannot get appid");
    return;
  } else {
    appid_ = appid;
    DEVICEAPI_LOG_INFO("app id: %s", appid.c_str());
  }

  pkgmgrinfo_appinfo_h handle;
  if (pkgmgrinfo_appinfo_get_appinfo(appid.c_str(), &handle) != PMINFO_R_OK) {
    DEVICEAPI_LOG_ERROR("pkgmgrinfo_appinfo_get_appinfo failed");
    return;
  }

  char* pkgid = nullptr;
  if (pkgmgrinfo_appinfo_get_pkgid(handle, &pkgid) != PMINFO_R_OK) {
    DEVICEAPI_LOG_ERROR("pkgmgrinfo_appinfo_get_pkgid failed");
  } else {
    pkgid_ = pkgid;
    DEVICEAPI_LOG_INFO("package id: %s", pkgid);
  }

  char* rootpath = nullptr;
  if (pkgmgrinfo_appinfo_get_root_path(handle, &rootpath) != PMINFO_R_OK) {
    DEVICEAPI_LOG_ERROR("pkgmgrinfo_appinfo_get_root_path failed");
  } else {
    rootpath_ = rootpath;
    DEVICEAPI_LOG_INFO("rootpath: %s", rootpath);
  }

  pkgmgrinfo_appinfo_destroy_appinfo(handle);
}

const std::string TizenRuntimeInfo::privileges() {
  if (!privileges_.empty()) {
    return privileges_;
  }

  tpk::parse::TPKConfigParser parser;
  boost::filesystem::path manifestPath(rootpath_ + "/tizen-manifest.xml");
  parser.ParseManifest(manifestPath);
  if (!parser.ParseManifest(manifestPath)) {
    DEVICEAPI_LOG_ERROR("cannot read manifest");
    return "";
  }

  auto manifestData = parser.GetManifestData(tpk::parse::PrivilegesInfo::key());
  auto privileges =
      reinterpret_cast<const tpk::parse::PrivilegesInfo*>(manifestData.get())
          ->GetPrivileges();

  std::stringstream stream;
  for (auto& privilege : privileges) {
    stream << privilege.first << ",";
  }

  privileges_ = stream.str();
  DEVICEAPI_LOG_INFO("privileges: %s", privileges_.c_str());

  return privileges_;
}

const std::string TizenRuntimeInfo::getRuntimeVariable(const std::string& key) {
  if (key == "application_id") {
    return appid_;
  } else if (key == "package_id") {
    return pkgid_;
  } else if (key == "app_root") {
    return rootpath_;
  } else if (key == "privileges") {
    return privileges();
  }
  return "";
}

}  // namespace DeviceAPI

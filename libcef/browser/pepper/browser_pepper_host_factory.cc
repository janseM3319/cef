// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/pepper/browser_pepper_host_factory.h"

#include "libcef/browser/pepper/pepper_flash_browser_host.h"

#include "build/build_config.h"
#include "chrome/browser/renderer_host/pepper/pepper_flash_clipboard_message_filter.h"
#include "chrome/browser/renderer_host/pepper/pepper_flash_drm_host.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/message_filter_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

using ppapi::host::MessageFilterHost;
using ppapi::host::ResourceHost;
using ppapi::host::ResourceMessageFilter;

CefBrowserPepperHostFactory::CefBrowserPepperHostFactory(
    content::BrowserPpapiHost* host)
    : host_(host) {}

CefBrowserPepperHostFactory::~CefBrowserPepperHostFactory() {}

scoped_ptr<ResourceHost> CefBrowserPepperHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK(host == host_->GetPpapiHost());

  // Make sure the plugin is giving us a valid instance for this resource.
  if (!host_->IsValidInstance(instance))
    return scoped_ptr<ResourceHost>();

  // Flash interfaces.
  if (host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_FLASH)) {
    switch (message.type()) {
      case PpapiHostMsg_Flash_Create::ID:
        return scoped_ptr<ResourceHost>(
            new PepperFlashBrowserHost(host_, instance, resource));
      case PpapiHostMsg_FlashClipboard_Create::ID: {
        scoped_refptr<ResourceMessageFilter> clipboard_filter(
            new chrome::PepperFlashClipboardMessageFilter);
        return scoped_ptr<ResourceHost>(new MessageFilterHost(
            host_->GetPpapiHost(), instance, resource, clipboard_filter));
      }
      case PpapiHostMsg_FlashDRM_Create::ID:
        return scoped_ptr<ResourceHost>(
            new chrome::PepperFlashDRMHost(host_, instance, resource));
    }
  }

  NOTREACHED() << "Unhandled message type: " << message.type();
  return scoped_ptr<ResourceHost>();
}

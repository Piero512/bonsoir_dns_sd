#include "include/bonsoir_dns_sd/bonsoir_dns_sd_plugin.h"
#include "plugin.hpp"
 // namespace

void BonsoirDnsSdPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  BonsoirDnsSdPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

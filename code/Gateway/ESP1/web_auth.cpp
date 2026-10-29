#include "web_auth.h"
#include "config.h"
#include <Preferences.h>

bool loadOrCreateWebUiCredentials(WebUiCredentials& credentials) {
  static_assert(sizeof(WEBUI_USERNAME) <= sizeof(credentials.username), "Web UI username is too long");
  static_assert(sizeof(WEBUI_PASSWORD) <= sizeof(credentials.password), "Web UI password is too long");
  constexpr uint32_t magic = 0x43424131; // CBA1
  Preferences prefs;
  if (!prefs.begin("capibridge", false)) return false;
  bool ok = false;
  if (prefs.isKey("webAuth")) {
    ok = prefs.getBytesLength("webAuth") == sizeof(credentials) &&
         prefs.getBytes("webAuth", &credentials, sizeof(credentials)) == sizeof(credentials) &&
         credentials.magic == magic &&
         memchr(credentials.username, '\0', sizeof(credentials.username)) &&
         memchr(credentials.password, '\0', sizeof(credentials.password));
  } else {
    memset(&credentials, 0, sizeof(credentials));
    credentials.magic = magic;
    strlcpy(credentials.username, WEBUI_USERNAME, sizeof(credentials.username));
    strlcpy(credentials.password, WEBUI_PASSWORD, sizeof(credentials.password));
    ok = prefs.putBytes("webAuth", &credentials, sizeof(credentials)) == sizeof(credentials);
  }
  prefs.end();
  return ok;
}

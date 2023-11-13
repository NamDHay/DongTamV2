#ifndef DNS_PING_H
#define DNS_PING_H

#include "esp_err.h"

#define DNS_USER_HOST "app.iotvision.vn"
#define DNS_GOOGLE "google.com"
#define DNS_ESPRESSIF "www.espressif.com"

esp_err_t DNS_PingToHost(char *host);
void DNS_StartToPing();
char* DNS_GetCurrentHost();
void DNS_StopPing();
bool DNS_IsPinging();
#endif
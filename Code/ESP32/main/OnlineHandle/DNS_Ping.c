#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "OnlineStatusEvent.h"
#include "DNS_Ping.h"
esp_ping_handle_t ping;

#define TIME_TO_PING 3
char *currentHost = NULL;
bool IsPinging = 0;
bool DNS_IsPinging(){
    return IsPinging;
}

static void TestOnPingSuccess(esp_ping_handle_t hdl, void *args)
{
    IsPinging = 1;
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    printf("Successfully get %lu bytes from %s icmp_seq=%d ttl=%d time=%ld ms\n",recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
    if(seqno == TIME_TO_PING){
        OnlEvt_SetBit(ONL_EVT_PING_SUCCESS);
        OnlEvt_ClearBit(ONL_EVT_PING_TIMEOUT);
        IsPinging = 0;
    }
}

static void TestOnPingTimeout(esp_ping_handle_t hdl, void *args)
{
    IsPinging = 1;
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    printf("Ping timeout from %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);
    if(seqno == TIME_TO_PING){
        OnlEvt_SetBit(ONL_EVT_PING_TIMEOUT);
        OnlEvt_ClearBit(ONL_EVT_PING_SUCCESS);
        DNS_StopPing(ping);
        IsPinging = 0;
    }
}

static void TestOnPingEnd(esp_ping_handle_t hdl, void *args)
{
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;

    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    printf("End with %lu packets transmitted, %ld received, time %ldms\n", transmitted, received, total_time_ms);
}

char* DNS_GetCurrentHost()
{ 
    if(currentHost) return currentHost;
    return 0;
}

void DNS_StartToPing()
{
    if(!OnlEvt_CheckBit(ONL_EVT_WIFI_CONNECTED) || !DNS_GetCurrentHost()) return;
    esp_ping_start(ping);
}

void DNS_StopPing()
{
    esp_ping_stop(ping);
}

esp_err_t DNS_PingToHost(char *host)
{
    if(!OnlEvt_CheckBit(ONL_EVT_WIFI_CONNECTED)) return ESP_ERR_INVALID_ARG;
    esp_err_t err = ESP_OK;
    printf("Ping to %s ....",host);
    ip_addr_t targetAddress;
    memset(&targetAddress,0,sizeof(targetAddress));
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint,0,sizeof(hint));
    // Lấy thông tin server 
    err = getaddrinfo(host, NULL, &hint, &res);
    //Trong trường hợp không có kết nối Internet thì không thể lấy được thông tin server
    if(err != ESP_OK) return err;
    /* 
    Sau khi gọi hàm getaddrinfo từ host thì thông tin được lưu vào trong res thuộc kiểu addrinfo
    Trong addrinfo có một member struct sockaddr với tên con trỏ tên là ai_addr(address info_ address)
    ép kiểu con trỏ ai_addr đó thành kiểu sockaddr_in* (nghĩa là ép kiểu từ sockaddr thành sockaddr_in) 
    và trỏ tới member in_addr có tên sin_addr trong sockaddr_in và gán vào addr4
    */
    struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
    // ESP_LOGI("DNS_Check","ai_addr:%p,sa_data:%s,addr4:%lu",&res->ai_addr,res->ai_addr->sa_data,addr4.s_addr);
    inet_addr_to_ip4addr(ip_2_ip4(&targetAddress),&addr4);
    freeaddrinfo(res);
    

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = targetAddress;          // target IP address
    ping_config.count = TIME_TO_PING; // or use esp_ping_stop

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.on_ping_success = TestOnPingSuccess;
    cbs.on_ping_timeout = TestOnPingTimeout;
    cbs.on_ping_end = TestOnPingEnd;

    
    esp_ping_new_session(&ping_config, &cbs, &ping);
    esp_ping_start(ping);
    return err;
}
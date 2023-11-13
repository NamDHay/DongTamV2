#ifndef _HTTP_POSTGET_H
#define _HTTP_POSTGET_H
#include "POSTGET.h"

/*
{
    'IMEI': "30C6F741735C", 
    'Power':1, 
    'FAN':1, 
    'ODCMode':1, 
    'ValveError':0, 
    'VanKich':0, 
    'DeltaPH':1000, 
    'DeltaP':100.1,
    'DeltaPL':250,
    'LED10Bar':1,
    'RTC': "17/8/2023 11:57:00"
}
*/


#define URL_POST_IOTVISION_DONGTAM "http://app.iotvision.vn/api/DongTam_DuLieu"
HTTP_CODE_e onl_HTTP_SendToServer(int a);

#endif
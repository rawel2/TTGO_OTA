/*
 * mk_fota_https.h
 *
 *  Created on: 30 kwi 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef MAIN_MK_FOTA_HTTPS_H_
#define MAIN_MK_FOTA_HTTPS_H_

/* BIN firmware - filename to download */
#define FIRMWARE_FILENAME	"TTGO_ota"


#define FOTA_WEB_URL	"https://test.romaniuk.cloud/" FIRMWARE_FILENAME ".bin"

//#define FOTA_WEB_URL	"https://onet.pl/" FIRMWARE_FILENAME ".bin"

//#define FOTA_WEB_URL	"https://nyc.com/" FIRMWARE_FILENAME ".bin"

#include "st7789.h"

extern const char *TAG;

extern TFT_t *dev1;

extern esp_err_t start_fota_https( void );



#endif /* MAIN_MK_FOTA_HTTPS_H_ */

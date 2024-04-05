/*
 * mk_fota_https.c
 *
 *  Created on: 30 kwi 2022
 *      Author: Miros³aw Kardaœ
 */
#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"




//-----------------------------------------

#include "mk_wifi.h"
#include "mk_tools.h"

#include "mk_fota_https.h"
#include "st7789.h"

extern const char server_ca_cert_cer_start[]  asm("_binary_ca_cert_cer_start");

extern FontxFile fx24G[2];

esp_err_t client_event_handler(esp_http_client_event_t *evt) {

	static int len,len1;
	char buf[20];

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
        	ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
        	ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
        	ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
//        	ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        	len += evt->data_len;

        	printf( LLN "Downloaded bytes: %d\n", len );
        	if (len1 < len /10000){
        		buf[0] = 0;
        		len1 = len /10000;
        		sprintf( buf,"OTA:%03d0KB", len1);
        		lcdDrawString(dev1, fx24G, 1,200 , (uint8_t*)buf, TFT_WHITE);

        	}

            break;
        case HTTP_EVENT_ON_FINISH:
        	ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
        	ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;

    }
    return ESP_OK;
}




esp_err_t start_fota_https( void ) {

	/*********************************************************************************
	 * 			F O T A   ----->    H T T P S										 *
	 *********************************************************************************/


			esp_http_client_config_t clientConfig = {
					.url = FOTA_WEB_URL,
					.cert_pem = server_ca_cert_cer_start,
					.event_handler = client_event_handler,
			};

			esp_https_ota_config_t otaConfig = {
					.http_config = &clientConfig,

			};

			if( esp_https_ota( &otaConfig ) == ESP_OK ) {

				return ESP_OK;

			}

			ESP_LOGE(TAG, "Failed to update firmware");
			return ESP_FAIL;
	/********************************************************** FOTA HTTPS end ********/

}



















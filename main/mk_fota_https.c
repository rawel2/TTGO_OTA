/*
 * mk_fota_https.c
 *
 *  Created on: 30 kwi 2022
 *      Author: Miros�aw Karda�
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

//-----BEGIN CERTIFICATE-----
//MIIHcDCCBVigAwIBAgIQEnN3l6MViG3+t2x+aloDVDANBgkqhkiG9w0BAQsFADBr
//MQswCQYDVQQGEwJJVDEOMAwGA1UEBwwFTWlsYW4xIzAhBgNVBAoMGkFjdGFsaXMg
//Uy5wLkEuLzAzMzU4NTIwOTY3MScwJQYDVQQDDB5BY3RhbGlzIEF1dGhlbnRpY2F0
//aW9uIFJvb3QgQ0EwHhcNMjAwNzA2MDYzNjUwWhcNMzAwOTIyMTEyMjAyWjCBhDEL
//MAkGA1UEBhMCSVQxEDAOBgNVBAgMB0JlcmdhbW8xGTAXBgNVBAcMEFBvbnRlIFNh
//biBQaWV0cm8xFzAVBgNVBAoMDkFjdGFsaXMgUy5wLkEuMS8wLQYDVQQDDCZBY3Rh
//bGlzIERvbWFpbiBWYWxpZGF0aW9uIFNlcnZlciBDQSBHMzCCAiIwDQYJKoZIhvcN
//AQEBBQADggIPADCCAgoCggIBALVGM4gsi80jpqFUmuK3Z0QKmhrCyEytbycoy44/
//0IN9lOxS8RHPHgqOOCwrK+WaFlELgfpv2Yx9HI4bMvZUeKKSBNgOTpxceBzUIIv7
//T9eoeN9lrzSlbcdLq0TZFl2iNuBFKzMVhu3W+bb+UKR3JvrxFmjfHv9KuUJK6LgI
//xlfQaLCpXf25FMk1lSCqmgSVmRc59rIa2PNwHTnRcGqOHCX7kPQJ88JHjNGLHbVj
//8QuOCLdZB7rADR2jsJ1p7rZAMksbNJ9gWdlmA3H0s9Z/4SJIU7R4RIcOpVzNqHo8
//Jv4OhDo4ZoBysdaxDN7sRvTJos/2QAVB7/aPFanIQlaZpFBnZIzufAQ58FkbcBJY
//ZQf9lysjemiHJbqYrokqseaQ0NXOh36D2cEk6CIFK4j6D388K1gi1scikGz3GTei
//QXAxCaHwuco8e7cjzHRV5C+a/DBhEcErKNp0i1w20gYOdKpQTiu8IETYu8epjmIH
//2BSJKWDwlDIDpiJAvtNmrKj2xYL+1F8nOJyEcx/M4mMrK5wdN2oWWSIiUwA0zUhg
//0K21w2is0J2QuCBnDE+fL9FTud5V7Uq9w4ZaKgsQtzAjq9+vhIzfEbAZZnnr1odI
//VTSCcPVSd4mm6RDBWpbKIYCPpctov0kIYyowjUzZ4H26AsjVuOHk1Y0KszIQIvWz
//gDIpAgMBAAGjggH0MIIB8DAPBgNVHRMBAf8EBTADAQH/MB8GA1UdIwQYMBaAFFLY
//iDrIn3hm7YnzezhwlMkCAjbQMEEGCCsGAQUFBwEBBDUwMzAxBggrBgEFBQcwAYYl
//aHR0cDovL29jc3AwNS5hY3RhbGlzLml0L1ZBL0FVVEgtUk9PVDBFBgNVHSAEPjA8
//MDoGBFUdIAAwMjAwBggrBgEFBQcCARYkaHR0cHM6Ly93d3cuYWN0YWxpcy5pdC9h
//cmVhLWRvd25sb2FkMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATCB4wYD
//VR0fBIHbMIHYMIGWoIGToIGQhoGNbGRhcDovL2xkYXAwNS5hY3RhbGlzLml0L2Nu
//JTNkQWN0YWxpcyUyMEF1dGhlbnRpY2F0aW9uJTIwUm9vdCUyMENBLG8lM2RBY3Rh
//bGlzJTIwUy5wLkEuJTJmMDMzNTg1MjA5NjcsYyUzZElUP2NlcnRpZmljYXRlUmV2
//b2NhdGlvbkxpc3Q7YmluYXJ5MD2gO6A5hjdodHRwOi8vY3JsMDUuYWN0YWxpcy5p
//dC9SZXBvc2l0b3J5L0FVVEgtUk9PVC9nZXRMYXN0Q1JMMB0GA1UdDgQWBBRCg22A
//fAmEZ/2AV6vxJvV3yCKCcTAOBgNVHQ8BAf8EBAMCAQYwDQYJKoZIhvcNAQELBQAD
//ggIBACeAFKdmcRaIJbdy6mSR+NXzZulMEx3D3gsmOtd8Or0xcK351UH+uKZuAOQJ
//T/9vODD7/kVD46Ln2M9soJigiyxSRc+Pano2cpT/viqyYZEwIC1JKQWy1dsZNI4k
//c3gJMuN+KOu9gSjlXgyn+pTcjiFC7iWbpO4tt3HNtKjRkCBXGtp9mE4pQr7eyn3l
//14Rf+E++3eMVsp39YZ7yUjUJzFiMGoPuwPaEsqnj0FYytJ7vqLzsHXGhvr3C3K8S
//Xzs5yQdgmGUDrkSHI1Bg0mlSQ1I9g4L3NKzZVJFbBW4p5AvbjmWQFMek0MOSVFNA
//0J6NduiOMRCMtdCrEySiK8rai+UwVHXMDJwy016aodBsjmlydSgbSq9CWlKNQaUs
//tB3NHgzbZRexvXMP0BaPB0QyzWLZ2cMQ9DtaTtsvWFQn7J00FYPV9ZG3MMkBId+5
//qk/q8c6UZ3u5kdehslZ31p40ptT1cNK0U/mRnMg2OzAR/l62Uq5v+Z6ZTiD469iR
//2Tb24vQGa7G4NB8nMyC/MxPtmKlwLd1PTIlLpnkqvX266b3HAPGDyJj2B7NzEtIm
//9WSR5HM+J+GFzssAyI/feJxA1cdLiOIyHtdMMRMjlsUxaMWsNzf55bifWl7Zttxm
//M14ovZrkI+kjvfoT83bXDOgsB5E5TZFg6q5azRdkx6AHC8ah
//-----END CERTIFICATE-----

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



















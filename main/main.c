
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"


#include "FreeRTOSConfig.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <errno.h>

#include "lwip/dns.h"

#include "esp_err.h"
#include "esp_log.h"

#include <time.h>
#include "lwip/apps/sntp.h"
#include <sys/time.h>


#include "mk_wifi.h"
#include "mk_tools.h"
#include "mk_i2c.h"

#include "st7789.h"
#include "fontx.h"
#include "bmpfile.h"
#include "decode_jpeg.h"
#include "decode_png.h"
#include "pngle.h"

#include "tools.h"
#include "lcd_tests.h"

#include "mk_fota_https.h"


#define LED1_GPIO 			GPIO_NUM_2
#define FOTA_START_GPIO		GPIO_NUM_35


#define	INTERVAL		400
#define WAIT	vTaskDelay(INTERVAL)

const char *TAG = "FOTA";

// You have to set these CONFIG value using menuconfig.
#if 0
#define CONFIG_WIDTH  135
#define CONFIG_HEIGHT 240
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_CS_GPIO -1
#define CONFIG_DC_GPIO 19
#define CONFIG_RESET_GPIO 15
#define CONFIG_BL_GPIO -1
#endif



TFT_t *dev1;

FontxFile fx16G[2];
FontxFile fx24G[2];
FontxFile fx32G[2];
FontxFile fx32L[2];
FontxFile fx16M[2];
FontxFile fx24M[2];
FontxFile fx32M[2];

SemaphoreHandle_t ota_semaphore;

void mk_got_ip_cb( char * ip ) {

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(mk_netif_sta, &ip_info);
	printf( "[STA] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[STA] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[STA] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );

	esp_netif_get_ip_info(mk_netif_ap, &ip_info);
	printf( "[AP] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[AP] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[AP] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );


}


void mk_sta_disconnected_cb( void ) {

	gpio_set_level(GPIO_NUM_33, 1);

	printf("\n************ STA Disconnected - callback \n");

}


void run_ota(void *params) {

	static uint8_t block;
	TFT_t *dev;
	uint8_t ascii[20];

	dev = (TFT_t *)params;

	while (1) {

		xSemaphoreTake(ota_semaphore, portMAX_DELAY);
		ESP_LOGI(TAG, "Invoking OTA");
		lcdDrawRect(dev, 0, 132, CONFIG_WIDTH-1, 162, TFT_YELLOW);
			ascii[0]=0;
			strcpy((char *)ascii, "Start OTA");
			lcdDrawString(dev, fx24G, 5, 160, ascii, TFT_WHITE);

		if (block)
			continue;	// blokada drga� styk�w i przed ponownym wci�ni�ciem
		block = 1;


		if (start_fota_https() == ESP_OK ){
			printf("restarting in 5 seconds\n");
			ascii[0]=0;
			strcpy((char *)ascii, " Restart ");
			lcdDrawString(dev, fx24G, 5, 160, ascii, TFT_WHITE);
			vTaskDelay(pdMS_TO_TICKS(5000));
			lcdFillScreen(dev, TFT_BLACK);
			vTaskDelay(pdMS_TO_TICKS(1000));
			esp_restart();
		}else
		{
			printf("OTA Error\n");
			ascii[0]=0;
			strcpy((char *)ascii, "OTA ERROR ");
			lcdDrawString(dev, fx24G, 5, 160, ascii, TFT_WHITE);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}


	}
}

void on_button_pushed(void *params) {
	if(get_sta_ip_state()){
		xSemaphoreGiveFromISR(ota_semaphore, pdFALSE);
	}
}

void ST7789(void *pvParameters)
{
	TFT_t dev;

	dev = *((TFT_t *)pvParameters);

	
	while(1) {

		FillTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
		WAIT;

//		ColorBarTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		ArrowTest(&dev, fx16G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		LineTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		CircleTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;

//		RoundRectTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		RectAngleTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		TriangleTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		if (CONFIG_WIDTH >= 240) {
//			DirectionTest(&dev, fx24G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		} else {
//			DirectionTest(&dev, fx16G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		}
//		WAIT;
//
//		if (CONFIG_WIDTH >= 240) {
//			HorizontalTest(&dev, fx24G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		} else {
//			HorizontalTest(&dev, fx16G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		}
//		WAIT;
//
//		if (CONFIG_WIDTH >= 240) {
//			VerticalTest(&dev, fx24G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		} else {
//			VerticalTest(&dev, fx16G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		}
//		WAIT;
//
//		FillRectTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		ColorTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		CodeTest(&dev, fx32G, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		CodeTest(&dev, fx32L, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//		char file[32];
//		strcpy(file, "/spiffs/image.bmp");
//		BMPTest(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//
//#ifndef CONFIG_IDF_TARGET_ESP32S2
//		strcpy(file, "/spiffs/esp32.jpeg");
//		JPEGTest(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;
//#endif
//
//		strcpy(file, "/spiffs/esp_logo.png");
//		PNGTest(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
//		WAIT;

		// Multi Font Test
		uint16_t color;
		uint8_t ascii[40];
		uint16_t margin = 0;
		lcdFillScreen(&dev, TFT_BLACK);
		color = TFT_WHITE;
		lcdSetFontDirection(&dev, 0);
		uint16_t xpos = 0;
		uint16_t ypos = 15;
		int xd = 0;
		int yd = 1;
		if(CONFIG_WIDTH < CONFIG_HEIGHT) {
					lcdSetFontDirection(&dev, 1);
					xpos = (CONFIG_WIDTH-1)-16;
					ypos = 0;
					xd = 1;
					yd = 0;
				}
				strcpy((char *)ascii, "16Dot Gothic Font");
				lcdDrawString(&dev, fx16G, xpos, ypos, ascii, color);

				xpos = xpos - (24 * xd) - (margin * xd);
				ypos = ypos + (16 * yd) + (margin * yd);
				strcpy((char *)ascii, "24Dot Gothic Font");
				lcdDrawString(&dev, fx24G, xpos, ypos, ascii, color);

				if (CONFIG_WIDTH >= 135) {
					xpos = xpos - (32 * xd) - (margin * xd);
					ypos = ypos + (24 * yd) + (margin * yd);
					strcpy((char *)ascii, "32Dot Gothic Font");
					lcdDrawString(&dev, fx32G, xpos, ypos, ascii, color);
				}

				xpos = xpos - (10 * xd) - (margin * xd);
				ypos = ypos + (10 * yd) + (margin * yd);
				strcpy((char *)ascii, "16Dot Mincyo Font");
				lcdDrawString(&dev, fx16M, xpos, ypos, ascii, color);

				xpos = xpos - (24 * xd) - (margin * xd);;
				ypos = ypos + (16 * yd) + (margin * yd);
				strcpy((char *)ascii, "24Dot Mincyo Font");
				lcdDrawString(&dev, fx24M, xpos, ypos, ascii, color);

				if (CONFIG_WIDTH >= 135) {
					xpos = xpos - (32 * xd) - (margin * xd);;
					ypos = ypos + (24 * yd) + (margin * yd);
					strcpy((char *)ascii, "32Dot Mincyo Font");
					lcdDrawString(&dev, fx32M, xpos, ypos, ascii, color);
				}

		lcdSetFontDirection(&dev, 0);
		WAIT;

	} // end while

	// never reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}


void display_date_time( TFT_t *dev ) {


	char buf[20];
	buf[0] = 0;
	uint32_t mem=0;

	if(sta_ip_state_changed(  )) {
		lcdDrawFillRect(dev, 0, 0, CONFIG_WIDTH-1, 28, TFT_BLACK);
		if(get_sta_ip_state()){
			lcdDrawRect(dev, 0, 0, CONFIG_WIDTH-1, 28, TFT_GREEN);
			strcpy(buf, "Connected");
			lcdDrawString(dev, fx24G, 10, 26, (uint8_t*)buf, TFT_GREEN);
			buf[0] = 0;
			strcat( buf, "%F %H:%M" );
			mk_get_formatted_system_datetime( buf );
			if( !buf[0] ) sprintf( buf, "WAITING FOR SNTP" );
			lcdDrawFillRect(dev, 0, 222, 134, 239, TFT_BLACK);
			lcdDrawString(dev, fx16M, 2, 239, (uint8_t*)buf, TFT_YELLOW);
		} else {
			lcdDrawRect(dev, 0, 0, CONFIG_WIDTH-1, 28, TFT_RED);
			strcpy(buf, "Disconect");
			lcdDrawString(dev, fx24G, 10, 26, (uint8_t*)buf, TFT_RED);
		}

	}

	mem = esp_get_free_heap_size();
	//ESP_LOGW(TAG, "Free memory heap: %d", mem);
	buf[0] = 0;
	sprintf( buf,"MEM: %06d", (int)mem );

	// lcdDrawFillRect(dev, 50, 29, CONFIG_WIDTH-1, 54, TFT_BLACK);
	// lcdUnsetFontFill(dev);
	lcdDrawString(dev, fx24G, 1,54 , (uint8_t*)buf, TFT_WHITE);
	// lcdSetFontFill(dev, TFT_BLACK);

	if( !is_time_changed(60) ) return;	// if time not changed - return without display

	buf[0] = 0;
	strcat( buf, "%F %H:%M\n" );
	mk_get_formatted_system_datetime( buf );

	if( !buf[0] ) sprintf( buf, "WAITING FOR SNTP\n" );

	printf(buf);

	//lcdDrawFillRect(dev, 0, 222, 134, 239, BLACK);

	lcdDrawString(dev, fx16M, 2, 239, (uint8_t*)buf, TFT_YELLOW);

}


void app_main(void)
{
	uint8_t ascii[40];

	vTaskDelay( 100 );	// start
	printf("\nREADY\n");

	/*........ konfiguracja pin�w dla diod LED ..................................*/
	gpio_set_direction( LED1_GPIO, GPIO_MODE_OUTPUT );
	gpio_set_level( LED1_GPIO, 0 );

	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 10,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total,&used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	SPIFFS_Directory("/spiffs/");

	// set font file

	InitFontx(fx16G,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
	InitFontx(fx24G,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
	InitFontx(fx32G,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic
	InitFontx(fx32L,"/spiffs/LATIN32B.FNT",""); // 16x32Dot Latin

	InitFontx(fx16M,"/spiffs/ILMH16XB.FNT",""); // 8x16Dot Mincyo
	InitFontx(fx24M,"/spiffs/ILMH24XB.FNT",""); // 12x24Dot Mincyo
	InitFontx(fx32M,"/spiffs/ILMH32XB.FNT",""); // 16x32Dot Mincyo

	TFT_t dev;

	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
	lcdSetFontFill(&dev, TFT_BLACK);

	dev1 = &dev;

	// Initialize NVS
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	lcdFillScreen(&dev, TFT_BLACK);

	lcdDrawRect(&dev, 0, 0, CONFIG_WIDTH-1, 28, TFT_YELLOW);
	strcpy((char *)ascii, "WIFI scan");
	lcdDrawString(&dev, fx24G, 10, 26, ascii, TFT_WHITE);


    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	if( mk_wifi_scan( (uint8_t*)"szajs22" ) ) {
		set_my_sta_ssid( "szajs22" , "bdghjkptw" );
	}
	else if( mk_wifi_scan( (uint8_t*)"Aptek" ) ) {
		set_my_sta_ssid( "Aptek" , "bdghjkptw" );
	}


	/* ```````` Inicjalizacja WiFi ````````````````````````````````````````````` */
//	mk_wifi_init( WIFI_MODE_APSTA, mk_got_ip_cb, mk_sta_disconnected_cb, mk_ap_join_cb, mk_ap_leave_cb  );
//	mk_wifi_init( WIFI_MODE_AP, NULL, NULL, mk_ap_join_cb, mk_ap_leave_cb );
	mk_wifi_init( WIFI_MODE_STA, mk_got_ip_cb, mk_sta_disconnected_cb, NULL, NULL );


	/*...... konfiguracja przycisku do aktywacji FOTA ...............*/
	gpio_config_t gpioConfig = {
		  .pin_bit_mask = 1ULL << FOTA_START_GPIO,
		  .mode = GPIO_MODE_DEF_INPUT,
		  .pull_up_en = GPIO_PULLUP_ENABLE,
		  .pull_down_en = GPIO_PULLDOWN_DISABLE,
		  .intr_type = GPIO_INTR_NEGEDGE};
	gpio_config(&gpioConfig);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(FOTA_START_GPIO, on_button_pushed, NULL);


	ota_semaphore = xSemaphoreCreateBinary();
	xTaskCreate(run_ota, "run_ota", 8192, (void *) &dev, 5, NULL);

	ESP_LOGW(TAG, "I'm OLD HTTPS esp32 Firmware :(");
//	ESP_LOGW(TAG, "It's NEW HTTPS ESP32 Firmware version !!!");

	lcdDrawRect(&dev, 0, 100, CONFIG_WIDTH-1, 130, TFT_YELLOW);
	ascii[0]=0;
	strcpy((char *)ascii, "OLD Version");
//	strcpy((char *)ascii, "NEW Version");
	lcdDrawString(&dev, fx24G, 2, 126, ascii, TFT_WHITE);

	 // xTaskCreate(ST7789, "ST7789", 1024*6, (void *) &dev, 2, NULL);

	 while (1) {

	    	display_date_time(&dev);

	        vTaskDelay( 200 );
	    }
}

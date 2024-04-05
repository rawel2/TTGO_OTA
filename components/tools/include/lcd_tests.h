/*
 * lcd_tests.h
 *
 *  Created on: 6 cze 2022
 *      Author: pawrom
 */

#ifndef COMPONENTS_TOOLS_INCLUDE_LCD_TESTS_H_
#define COMPONENTS_TOOLS_INCLUDE_LCD_TESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "st7789.h"
#include "fontx.h"
#include "bmpfile.h"
#include "decode_jpeg.h"
#include "decode_png.h"
#include "pngle.h"

TickType_t FillTest(TFT_t * dev, int width, int height);

TickType_t ColorBarTest(TFT_t * dev, int width, int height);

TickType_t ArrowTest(TFT_t * dev, FontxFile *fx, int width, int height);

TickType_t DirectionTest(TFT_t * dev, FontxFile *fx, int width, int height);

TickType_t HorizontalTest(TFT_t * dev, FontxFile *fx, int width, int height);

TickType_t VerticalTest(TFT_t * dev, FontxFile *fx, int width, int height);

TickType_t LineTest(TFT_t * dev, int width, int height);

TickType_t CircleTest(TFT_t * dev, int width, int height);

TickType_t RectAngleTest(TFT_t * dev, int width, int height);

TickType_t TriangleTest(TFT_t * dev, int width, int height);

TickType_t RoundRectTest(TFT_t * dev, int width, int height);

TickType_t FillRectTest(TFT_t * dev, int width, int height);

TickType_t ColorTest(TFT_t * dev, int width, int height);

TickType_t BMPTest(TFT_t * dev, char * file, int width, int height);

TickType_t JPEGTest(TFT_t * dev, char * file, int width, int height);

TickType_t PNGTest(TFT_t * dev, char * file, int width, int height);

TickType_t CodeTest(TFT_t * dev, FontxFile *fx, int width, int height);



#endif /* COMPONENTS_TOOLS_INCLUDE_LCD_TESTS_H_ */

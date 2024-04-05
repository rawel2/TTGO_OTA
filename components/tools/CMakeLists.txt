idf_component_register(SRCS "tools.c" "st7789.c" "fontx.c" "decode_png.c" "pngle.c" "decode_jpeg.c" "lcd_tests.c"
                    	INCLUDE_DIRS "." "include" 
                    	REQUIRES "spiffs" "driver" )

# tjpgd library does not exist in ESP32-S2 ROM.
if(IDF_TARGET STREQUAL "esp32")
    list(APPEND srcs "decode_jpeg.c")
endif()

if(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND srcs "decode_jpeg.c")
endif()

if(IDF_TARGET STREQUAL "esp32c3")
    list(APPEND srcs "decode_jpeg.c")
endif()
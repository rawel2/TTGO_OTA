idf_component_register(SRCS "tools.c" "st7789.c" "fontx.c" "lcd_tests.c"
                    	INCLUDE_DIRS "." "include" 
                    	REQUIRES "spiffs" "driver" )


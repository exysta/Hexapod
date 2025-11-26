idf_component_register(
    SRCS "main.c" 
    PRIV_REQUIRES spi_flash driver pca9685 web-server
    INCLUDE_DIRS ""
)

# Include SPIFFS CMake functions
include($ENV{IDF_PATH}/components/spiffs/project_include.cmake)

# Create the SPIFFS partition image
spiffs_create_partition_image(storage ../webData FLASH_IN_PROJECT)

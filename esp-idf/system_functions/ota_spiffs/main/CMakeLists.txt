idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../OTA FLASH_IN_PROJECT)
idf_component_register(SRCS "camera.c"
                    INCLUDE_DIRS "."
                    REQUIRES log lcd wifi espressif__esp32-camera)

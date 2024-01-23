set(component_srcs "mq9.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES driver esp_adc
                       INCLUDE_DIRS ".")
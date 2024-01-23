set(component_srcs "sen0159.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES driver esp_adc
                       INCLUDE_DIRS ".")
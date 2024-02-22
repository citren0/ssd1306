set(srcs "ssd1306.c")

idf_component_register(
    SRCS "${srcs}"
    PRIV_REQUIRES driver
    INCLUDE_DIRS "."
)
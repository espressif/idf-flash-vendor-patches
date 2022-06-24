idf_component_register(SRCS "xmc_lock_sr.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES bootloader_support spi_flash
                       LDFRAGMENTS linker.lf)

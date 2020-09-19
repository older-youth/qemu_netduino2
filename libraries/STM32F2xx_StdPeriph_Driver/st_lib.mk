# C_SOURCES += stm32f0xx_adc.c
# C_SOURCES += stm32f0xx_can.c
# C_SOURCES += stm32f0xx_cec.c
# C_SOURCES += stm32f0xx_comp.c
# C_SOURCES += stm32f0xx_crc.c
# C_SOURCES += stm32f0xx_crs.c
# C_SOURCES += stm32f0xx_dac.c
# C_SOURCES += stm32f0xx_dma.c
# C_SOURCES += stm32f0xx_exti.c
# C_SOURCES += stm32f0xx_flash.c
# C_SOURCES += stm32f0xx_gpio.c
# C_SOURCES += stm32f0xx_i2c.c
# C_SOURCES += stm32f0xx_iwdg.c
# C_SOURCES += stm32f0xx_misc.c
# C_SOURCES += stm32f0xx_pwr.c
# C_SOURCES += stm32f0xx_rcc.c
# C_SOURCES += stm32f0xx_rtc.c
# C_SOURCES += stm32f0xx_spi.c
# C_SOURCES += stm32f0xx_syscfg.c
# C_SOURCES += stm32f0xx_tim.c
# C_SOURCES += stm32f0xx_usart.c
# C_SOURCES += stm32f0xx_wwdg.c

DEPPATH += --dep-path $(ST_LIB_DIR)/src
# Makefile文件中的特殊变量,作用在在依赖文件上,如果当前目录找不到就到VPATH下去找 
VPATH += :$(ST_LIB_DIR)/src

C_INCLUDES += -I$(ST_LIB_DIR)/inc
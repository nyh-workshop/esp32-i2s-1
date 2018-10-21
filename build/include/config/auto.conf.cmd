deps_config := \
	/f/esp32-idf-compiler/esp-idf/components/app_trace/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/aws_iot/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/bt/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/driver/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/esp32/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/esp_adc_cal/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/esp_http_client/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/ethernet/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/fatfs/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/freertos/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/heap/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/http_server/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/libsodium/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/log/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/lwip/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/mbedtls/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/mdns/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/openssl/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/pthread/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/spi_flash/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/spiffs/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/tcpip_adapter/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/vfs/Kconfig \
	/f/esp32-idf-compiler/esp-idf/components/wear_levelling/Kconfig \
	/f/esp32-idf-compiler/esp-idf/Kconfig.compiler \
	/f/esp32-idf-compiler/esp-idf/components/bootloader/Kconfig.projbuild \
	/f/esp32-idf-compiler/esp-idf/components/esptool_py/Kconfig.projbuild \
	/f/esp32-idf-compiler/esp-idf/components/partition_table/Kconfig.projbuild \
	/f/esp32-idf-compiler/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;

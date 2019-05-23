deps_config := \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/log/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/mqtt/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/e/git-repositories/ESP8266/esp-aliyun-v2.3.0/mqtt_example/main/Kconfig.projbuild \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/e/git-repositories/ESP8266/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;

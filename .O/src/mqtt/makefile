MODULE_NAME := src/mqtt
ALL_SUB_DIRS := sample src/cmp src/coap src/cota src/dm src/fota src/http src/log src/mqtt src/ota src/platform src/sdk-tests src/shadow src/subdev src/system src/tfs src/tls src/utils
AR := ar
CC := gcc
CFLAGS := --coverage -DCMP_ENABLED -DCMP_VIA_MQTT_DIRECT -DCONFIG_GUIDER_AUTH_TIMEOUT=500 -DCONFIG_HTTP_AUTH_TIMEOUT=500 -DCONFIG_MID_HTTP_TIMEOUT=500 -DDM_ENABLED -DGATEWAY_SUPPORT -DHTTP_COMM_ENABLED -DIOTX_NET_INIT_WITH_PK_EXT -DIOTX_WITHOUT_ITLS -DMQTT_COMM_ENABLED -DMQTT_DIRECT -DMQTT_DIRECT_NOITLS -DMQTT_SHADOW -DOTA_ENABLED -DOTA_SIGNAL_CHANNEL=1 -DSERVICE_OTA_ENABLED -DSUBDEVICE_ENABLED -DSUBDEVICE_SUPPORT -DSUBDEV_VIA_MQTT -DUSING_SHA1_IN_HMAC -DUSING_UTILS_JSON -D_PLATFORM_IS_LINUX_ -D__UBUNTU_SDK_DEMO__ -Os -Wall -Werror -g3 -rdynamic
COMP_LIB := libiot_sdk.a
CONFIG_VENDOR := ubuntu
CROSS_CANDIDATES := AR CC CXX LD OBJCOPY STRIP
CROSS_PREFIX := 
CXX := g++
IMPORT_DIR := /home/wzh/c_sdk_tsl/src/import
INSTALL_DIR := /home/wzh/c_sdk_tsl/.O/usr
INSTALL_LIB_DIR := /home/wzh/c_sdk_tsl/.O/usr/lib
KERNEL_DIR := 
LD := ld
LDFLAGS := -lpthread
MAKE_ENV_VARS := 
OBJCOPY := objcopy
PACKAGE_DIR := /home/wzh/c_sdk_tsl/src/packages
RULE_DIR := /home/wzh/c_sdk_tsl/build-rules
STRIP := strip
SYSROOT_INC := /home/wzh/c_sdk_tsl/.O/usr/include
TOP_DIR := /home/wzh/c_sdk_tsl

include $(RULE_DIR)/settings.mk
include $(CONFIG_TPL)

all:

LIBA_TARGET     := libiot_mqtt.a
HDR_REFS        := src

PKG_SOURCE      := Link-MQTT.git
PKG_UPSTREAM    := git@gitlab.alibaba-inc.com:iot-middleware/Link-MQTT.git

env:
	@echo ""
	@printf -- "-----------------------------------------------------------------\n"
	@$(foreach var,$(SHOW_ENV_VARS),$(call Dump_Var,$(var)))
	@printf -- "-----------------------------------------------------------------\n"
	@echo ""

include $(RULE_DIR)/rules.mk

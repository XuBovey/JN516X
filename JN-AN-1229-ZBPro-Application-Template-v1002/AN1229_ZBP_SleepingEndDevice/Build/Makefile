##############################################################################
#
# MODULE:   Makefile
#
# DESCRIPTION: Makefie for the ZBPro EndDevice
# 
##############################################################################
# 
# This software is owned by NXP B.V. and/or its supplier and is protected
# under applicable copyright laws. All rights are reserved. We grant You,
# and any third parties, a license to use this software solely and
# exclusively on NXP products [NXP Microcontrollers such as JN5169, JN5168,
# JN5164, JN5161].
# You, and any third parties must reproduce the copyright and warranty notice
# and any other legend of ownership on each copy or partial copy of the
# software.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# 
# Copyright NXP B.V. 2015. All rights reserved
#
##############################################################################
# Application target name

TARGET = AN1229_ZBP_SleepingEndDevice

##############################################################################
# User definable make parameters that may be overwritten from the command line
# Default target device is the JN5168

JENNIC_CHIP ?= JN5168
JENNIC_CHIP_FAMILY ?=JN516x

#DEBUG_MODE ?=SWD_TRACE

##############################################################################
# Default DK4 development kit target hardware
JENNIC_PCB ?= DEVKIT4 

##############################################################################
# Select the network stack (e.g. MAC, ZBPro)

JENNIC_STACK ?= ZBPro

##############################################################################
# Default SDK is the IEEE802.15.4 SDK

JENNIC_SDK ?= JN-SW-4170

##############################################################################
# Default MAC is the IEEE802.15.4 Mini MAC

JENNIC_MAC ?= MiniMacShim

##############################################################################
# ZBPro Stack specific options
ZBPRO_DEVICE_TYPE = ZED
PDM_BUILD_TYPE ?= _EEPROM
STACK_SIZE = 5000
MINIMUM_HEAP_SIZE = 2000


ifeq ($(JENNIC_CHIP_FAMILY), JN517x)
JENNIC_SDK              = JN-SW-4270
JENNIC_PCB              = DEVKIT5
ZNCLKCMD                = AppBuildZBPro.ld
ENDIAN                  = LITTLE_ENDIAN_PROCESSOR
DISABLE_LTO = 1
else 
ifeq ($(JENNIC_CHIP_FAMILY), JN516x)
JENNIC_SDK              = JN-SW-4170
JENNIC_PCB              = DEVKIT4
ZNCLKCMD                = AppBuildZBPro.ld
ENDIAN 	                = BIG_ENDIAN
endif
endif

##############################################################################
# Debug options - define DEBUG to enable
# DEBUG ?=HW
#
# Define which UART to use for SW/HW debug
# DEBUG_PORT ?= UART1

##############################################################################
# By default TRACE and OVERLAYS have been turned off
# Set them to 1 to enable TRACE and overlays.
TRACE ?=1

ifeq ($(TRACE), 1)
CFLAGS  += -DDBG_ENABLE
$(info Building trace version ...)
# Enable any debug output here:
CFLAGS  += -DDEBUG_APP=1
CFLAGS  += -DDEBUG_BLINK_LED=1
CFLAGS  += -DDEBUG_PDM=1
CFLAGS  += -DDEBUG_EXCEPTION=1
endif

ifeq ($(JENNIC_CHIP_FAMILY),JN517x)
INTERRUPT_HANDLER_BBC = zps_isrMAC
LDFLAGS += -Wl,-u$(INTERRUPT_HANDLER_BBC) -Wl,-defsym,vAHI_IntHandlerBbc_select=$(INTERRUPT_HANDLER_BBC)
endif
##############################################################################
# For 6x use Number based PDM id's for newer families use 16 bit id numbers

$(info Building numbered PDM IDs version ...)
CFLAGS   += -DPDM_USER_SUPPLIED_ID

##############################################################################
# Path definitions

# Use if application directory contains multiple targets
SDK_BASE_DIR        =  ../../../../sdk/$(JENNIC_SDK)
APP_BASE            =  ../..
APP_BLD_DIR			= $(APP_BASE)/$(TARGET)/Build
APP_SRC_DIR 	   	= $(APP_BASE)/$(TARGET)/Source
APP_COMMON_SRC_DIR  = $(APP_BASE)/Common/Source
UTIL_SRC_DIR        = $(COMPONENTS_BASE_DIR)/ZigbeeCommon/Source

##############################################################################
# Application Source files

# Note: Path to source file is found using vpath below, so only .c filename is required
ifeq ($(JENNIC_CHIP_FAMILY), JN517x)
APPSRC = port_JN517x.c
endif
ifeq ($(JENNIC_CHIP_FAMILY), JN516x)
APPSRC += irq_JN516x.S
APPSRC += portasm_JN516x.S
APPSRC += port_JN516x.c
endif

APPSRC += pdum_gen.c
APPSRC += pdum_apdu.S
APPSRC += zps_gen.c
APPSRC += app_blink_led.c
APPSRC += app_start_SED.c
APPSRC += app_sleeping_enddevice.c
APPSRC += app_syscon.c
APPSRC += app_endpoint.c
APPSRC += appZdpExtraction.c
APPSRC += appZpsBeaconHandler.c
APPSRC += app_pdm.c

APP_ZPSCFG = ZBProAppTemplate.zpscfg
##############################################################################
# Standard Application header search paths

INCFLAGS += -I$(APP_SRC_DIR)
INCFLAGS += -I$(APP_SRC_DIR)/..
INCFLAGS += -I$(APP_COMMON_SRC_DIR)

# Application specific include files
INCFLAGS += -I$(COMPONENTS_BASE_DIR)/ZigbeeCommon/Include

##############################################################################
# Application libraries
# Specify additional Component libraries

#APPLIBS += 

##############################################################################

# You should not need to edit below this line

##############################################################################
##############################################################################
# Configure for the selected chip or chip family

include $(SDK_BASE_DIR)/Chip/Common/Build/config.mk
include $(SDK_BASE_DIR)/Platform/Common/Build/Config.mk
include $(SDK_BASE_DIR)/Stack/Common/Build/config.mk

##############################################################################

TEMP = $(APPSRC:.c=.o)
APPOBJS = $(TEMP:.S=.o)

##############################################################################
# Application dynamic dependencies

APPDEPS = $(APPOBJS:.o=.d)

##############################################################################
# Linker

# Add application libraries before chip specific libraries to linker so
# symbols are resolved correctly (i.e. ordering is significant for GCC)

APPLDLIBS := $(foreach lib,$(APPLIBS),$(if $(wildcard $(addprefix $(COMPONENTS_BASE_DIR)/Library/lib,$(addsuffix _$(JENNIC_CHIP).a,$(lib)))),$(addsuffix _$(JENNIC_CHIP),$(lib)),$(addsuffix _$(JENNIC_CHIP_FAMILY),$(lib))))
LDLIBS := $(APPLDLIBS) $(LDLIBS)


##############################################################################
# Dependency rules

.PHONY: all clean
# Path to directories containing application source 
vpath % $(APP_SRC_DIR):$(APP_COMMON_SRC_DIR):$(UTIL_SRC_DIR)


all: $(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).bin

-include $(APPDEPS)
#%.d:
#	rm -f $*.o

ifeq ($(JENNIC_CHIP_FAMILY), JN517x)
$(APP_SRC_DIR)/pdum_gen.c $(APP_SRC_DIR)/pdum_gen.h: $(APP_COMMON_SRC_DIR)/$(APP_ZPSCFG) $(PDUMCONFIG)
	$(info Configuring the PDUM ...)
	$(PDUMCONFIG) -z $(TARGET)  -e $(ENDIAN) -f $< -o $(APP_SRC_DIR)

$(APP_SRC_DIR)/zps_gen.c $(APP_SRC_DIR)/zps_gen.h: $(APP_COMMON_SRC_DIR)/$(APP_ZPSCFG) $(ZPSCONFIG)
	$(info Configuring the Zigbee Protocol Stack ...)
	$(ZPSCONFIG) -n $(TARGET) -t $(JENNIC_CHIP_FAMILY) -l $(ZPS_NWK_LIB) -a $(ZPS_APL_LIB) -c $(TOOL_COMMON_BASE_DIR)/$(TOOLCHAIN_PATH) -e $(ENDIAN) -f $< -o $(APP_SRC_DIR)

endif

ifeq ($(JENNIC_CHIP_FAMILY), JN516x)
$(APP_SRC_DIR)/pdum_gen.c $(APP_SRC_DIR)/pdum_gen.h: $(APP_COMMON_SRC_DIR)/$(APP_ZPSCFG) $(PDUMCONFIG)
	$(info Configuring the PDUM ...)
	$(PDUMCONFIG) -z $(TARGET)  -f $< -o $(APP_SRC_DIR)

$(APP_SRC_DIR)/zps_gen.c $(APP_SRC_DIR)/zps_gen.h: $(APP_COMMON_SRC_DIR)/$(APP_ZPSCFG) $(ZPSCONFIG)
	$(info Configuring the Zigbee Protocol Stack ...)
	$(ZPSCONFIG) -n $(TARGET) -t $(JENNIC_CHIP_FAMILY) -l $(ZPS_NWK_LIB) -a $(ZPS_APL_LIB) -c $(TOOL_COMMON_BASE_DIR)/$(TOOLCHAIN_PATH) -f $< -o $(APP_SRC_DIR)
endif


%.o: %.S
	$(info Assembling $< ...)
	$(CC) -c -o $(subst Source,Build,$@) $(CFLAGS) $(INCFLAGS) $< -MMD -MF $*.d -MP
	@echo

%.o: %.c 
	$(info Compiling $< ...)
	$(CC) -c -o $(subst Source,Build,$@) $(CFLAGS) $(INCFLAGS) $< -MMD -MF $*.d -MP
	@echo

$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf: $(APPOBJS) $(addsuffix.a,$(addprefix $(COMPONENTS_BASE_DIR)/Library/lib,$(APPLDLIBS))) 
	$(info Linking $@ ...)
	$(CC) $(LDFLAGS) -T$(ZNCLKCMD) -o $@ -Wl,--start-group $(APPOBJS) $(addprefix -l,$(LDLIBS)) -Wl,--end-group -Wl,-Map,$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).map 	 	 
	$(SIZE) $@ 


$(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).bin: $(TARGET)_$(JENNIC_CHIP)$(BIN_SUFFIX).elf 
	$(info Generating binary ...)
	$(OBJCOPY) -S -O binary $< $@
	
##############################################################################

clean:
	rm -f $(APPOBJS) $(APPDEPS) $(TARGET)_$(JENNIC_CHIP)*.bin $(TARGET)_$(JENNIC_CHIP)*.elf $(TARGET)_$(JENNIC_CHIP)*.map
	rm -f $(APP_SRC_DIR)/pdum_gen.* $(APP_SRC_DIR)/zps_gen*.* $(APP_SRC_DIR)/pdum_apdu.S

##############################################################################

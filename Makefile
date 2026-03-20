SDK  := ../tc_ble_single_sdk/tc_ble_single_sdk
PROJ := .
OUT  := out

CC      := tc32-elf-gcc
OBJCOPY := tc32-elf-objcopy
SIZE    := tc32-elf-size

CHIP   := -DCHIP_TYPE=CHIP_TYPE_827x -DMCU_STARTUP_8278
LS     := $(SDK)/project/tlsr_tc32/B87/boot.link
LIBS   := -llt_827x
LIBGCC := $(shell $(CC) -print-libgcc-file-name)

ELF        := $(OUT)/8bitdo_kbd.elf
DEBUG_ELF  := $(OUT)/8bitdo_kbd_debug.elf
BIN        := $(OUT)/8bitdo_kbd.bin

BASE_CFLAGS := \
    -ffunction-sections \
    -fdata-sections \
    -Wall \
    -fpack-struct \
    -fshort-enums \
    -finline-small-functions \
    -std=gnu99 \
    -fshort-wchar \
    -fms-extensions \
    -include $(PROJ)/src/app_config.h \
    $(CHIP)

INCLUDES := \
    -I$(SDK) \
    -I$(SDK)/common \
    -I$(SDK)/drivers/B87 \
    -I$(SDK)/drivers/B87/lib/include \
    -I$(PROJ)/src

# Release flags

RELEASE_CFLAGS := \
    -O2

RELEASE_LDFLAGS := \
    -Wl,--gc-sections \
    -Wl,-Map=$(OUT)/8bitdo_kbd.map

# Debug flags meant for Ghidra

DEBUG_CFLAGS := \
    -O0 \
    -g3 \
    -ggdb \
    -fno-omit-frame-pointer \
    -fno-inline \
    -fno-inline-functions

DEBUG_LDFLAGS := \
    -Wl,--gc-sections \
    -Wl,-Map=$(OUT)/8bitdo_kbd_debug.map

C_SRCS := \
    $(wildcard $(PROJ)/src/*.c) \
    $(wildcard $(SDK)/common/*.c) \
    $(wildcard $(SDK)/drivers/B87/flash/*.c) \
    $(SDK)/drivers/B87/driver_ext/ext_misc.c \
    $(SDK)/drivers/B87/gpio.c \
    $(SDK)/drivers/B87/clock.c \
    $(SDK)/drivers/B87/usbhw.c \
    $(SDK)/drivers/B87/timer.c \
    $(SDK)/drivers/B87/analog.c \
    $(SDK)/drivers/B87/flash.c \
    $(SDK)/drivers/B87/adc.c \
    $(SDK)/vendor/common/ble_flash.c

S_SRCS := $(SDK)/boot/B87/cstartup_827x.S

C_OBJS := $(patsubst %.c,$(OUT)/%.o,$(notdir $(C_SRCS)))
S_OBJS := $(patsubst %.S,$(OUT)/%.o,$(notdir $(S_SRCS)))
OBJS   := $(C_OBJS) $(S_OBJS)

VPATH := $(sort $(dir $(C_SRCS) $(S_SRCS)))

all: $(BIN)
debug: $(DEBUG_ELF)

$(OUT)/%.o: %.c
	@mkdir -p $(OUT)
	$(CC) $(BASE_CFLAGS) $(CFLAGS_EXTRA) $(INCLUDES) -c $< -o $@

$(OUT)/%.o: %.S
	@mkdir -p $(OUT)
	$(CC) $(BASE_CFLAGS) $(CFLAGS_EXTRA) $(INCLUDES) -c $< -o $@

# Release link

$(ELF): CFLAGS_EXTRA := $(RELEASE_CFLAGS)
$(ELF): LDFLAGS_EXTRA := $(RELEASE_LDFLAGS)
$(ELF): $(OBJS)
	$(CC) $(BASE_CFLAGS) $(CFLAGS_EXTRA) \
		-T $(LS) -L$(SDK)/proj_lib \
		-nostdlib \
		$(LDFLAGS_EXTRA) \
		-Wl,--defsym,__PM_DEEPSLEEP_RETENTION_ENABLE=0 \
		-Wl,--defsym,__SRAM_SIZE=0x850000 \
		-o $@ $(OBJS) $(LIBS) $(LIBGCC)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(SIZE) $(ELF)

# Debug link

$(DEBUG_ELF): CFLAGS_EXTRA := $(DEBUG_CFLAGS)
$(DEBUG_ELF): LDFLAGS_EXTRA := $(DEBUG_LDFLAGS)
$(DEBUG_ELF): $(OBJS)
	$(CC) $(BASE_CFLAGS) $(CFLAGS_EXTRA) \
		-T $(LS) -L$(SDK)/proj_lib \
		-nostdlib \
		$(LDFLAGS_EXTRA) \
		-Wl,--defsym,__PM_DEEPSLEEP_RETENTION_ENABLE=0 \
		-Wl,--defsym,__SRAM_SIZE=0x850000 \
		-Wl,--start-group $(OBJS) $(LIBS) $(LIBGCC) -Wl,--end-group \
		-o $@
	$(SIZE) $(DEBUG_ELF)

clean:
	rm -rf $(OUT)
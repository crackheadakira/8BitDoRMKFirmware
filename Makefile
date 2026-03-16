SDK  := ../tc_ble_single_sdk/tc_ble_single_sdk
PROJ := .
OUT  := out

CC      := tc32-elf-gcc
LD      := tc32-elf-ld
OBJCOPY := tc32-elf-objcopy
SIZE    := tc32-elf-size

CHIP   := -DCHIP_TYPE=CHIP_TYPE_827x -DMCU_STARTUP_8278
LIBGCC := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

CFLAGS := \
    -ffunction-sections \
    -fdata-sections \
    -Wall \
    -O2 \
    -fpack-struct \
    -fshort-enums \
    -finline-small-functions \
    -std=gnu99 \
    -fshort-wchar \
    -fms-extensions \
    $(CHIP)

INCLUDES := \
    -I$(SDK) \
    -I$(SDK)/common \
    -I$(SDK)/drivers/B87 \
    -I$(SDK)/drivers/B87/lib/include \
    -I$(PROJ)/src

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

LS       := $(SDK)/project/tlsr_tc32/B87/boot.link
LIBS     := -llt_827x
ELF      := $(OUT)/8bitdo_kbd.elf
BIN      := $(OUT)/8bitdo_kbd.bin

# --- TARGETS ---
all: $(BIN)

$(OUT)/%.o: %.c
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT)/%.o: %.S
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) -T $(LS) -L$(SDK)/proj_lib \
		-nostdlib \
		-Wl,--gc-sections \
		-Wl,--defsym,__PM_DEEPSLEEP_RETENTION_ENABLE=0 \
		-Wl,--defsym,__SRAM_SIZE=0x850000 \
		-Wl,-Map=$(OUT)/8bitdo_kbd.map \
		-o $@ $(OBJS) $(LIBS) $(LIBGCC)

$(BIN): $(ELF)
	$(OBJCOPY) -v -O binary $(ELF) $(BIN)
	$(SIZE) $(ELF)

clean:
	rm -rf $(OUT)

.PHONY: all clean
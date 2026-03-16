SDK := ../tc_ble_single_sdk/tc_ble_single_sdk
PROJ := .
OUT := out

CC      := tc32-elf-gcc
LD      := tc32-elf-ld
OBJCOPY := tc32-elf-objcopy
SIZE    := tc32-elf-size

CHIP := -DCHIP_TYPE=CHIP_TYPE_827x

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
    $(SDK)/drivers/B87/gpio.c \
    $(SDK)/drivers/B87/clock.c \
    $(SDK)/drivers/B87/usbhw.c \
    $(SDK)/drivers/B87/timer.c \
    $(SDK)/drivers/B87/analog.c \
    $(SDK)/drivers/B87/flash.c \
    $(SDK)/drivers/B87/adc.c

S_SRCS := \
    $(SDK)/boot/B87/cstartup_827x.S

C_OBJS := $(patsubst %.c,$(OUT)/%.o,$(notdir $(C_SRCS)))
S_OBJS := $(patsubst %.S,$(OUT)/%.o,$(notdir $(S_SRCS)))
OBJS   := $(C_OBJS) $(S_OBJS)

LS       := $(SDK)/project/tlsr_tc32/B87/boot.link
LIBS     := -llt_827x
LIB_PATH := -L$(SDK)/proj_lib
ELF      := $(OUT)/8bitdo_kbd.elf
BIN      := $(OUT)/8bitdo_kbd.bin

all: $(BIN)

$(OUT)/%.o: $(PROJ)/src/%.c
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT)/%.o: $(SDK)/drivers/B87/%.c
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT)/%.o: $(SDK)/boot/B87/%.S
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(ELF): $(OBJS)
	$(LD) --gc-sections $(LIB_PATH) -T $(LS) --defsym __PM_DEEPSLEEP_RETENTION_ENABLE=0 --defsym __SRAM_SIZE=0x850000 -Map=$(OUT)/8bitdo_kbd.map -o $@ $(OBJS) $(LIBS) $(LIBGCC)

$(BIN): $(ELF)
	$(OBJCOPY) -v -O binary $(ELF) $(BIN)
	$(SIZE) $(ELF)

clean:
	rm -rf $(OUT)

.PHONY: all clean
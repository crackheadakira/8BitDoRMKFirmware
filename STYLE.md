# Style

This document is meant to aid in writing documentation for this software.

## Table of Contents

- [Comments](#comments)
    - [Functions](#functions)
    - [Inline](#inline)
    - [File Headers](#file-headers)
    - [Structs](#structs)
    - [Constants](#constants)
- [Naming Conventions](#naming-conventions)
    - [Files](#files)
    - [Functions](#functions-1)
    - [Variables](#variables)
    - [Types](#types)
    - [Constants](#constants-1)
- [File Structure](#file-structure)
    - [C File Structure](#c-file-structure)
    - [Header File Structure](#header-file-structure)
- [Reverse Engineering Conventions](#reverse-engineering-conventions)
    - [Address References](#address-references)
    - [Ghidra Names](#ghidra-names)
    - [Unknown Behavior](#unknown-behavior)
    - [SDK Boundaries](#sdk-boundaries)
- [Global State](#global-state)
- [Function Complexity](#function-complexity)

## Comments

### Functions

```c
/**
 * @brief      One line summary of what the function does.
 *
 * @details    More detailed explanation if needed. Describe the observed
 *             behavior, any non-obvious logic, and why certain decisions
 *             were made in the reimplementation.
 *
 * @note       Any important caveats, side effects, or assumptions.
 *             For example: "Modifies global state X" or "Must be called
 *             after Y is initialized".
 *
 * @param      param_name   Description of parameter.
 * @param      param_name   Description of parameter.
 *
 * @return     Description of return value. If void, omit this tag.
 *
 * @fw_addr    0x0000XXXX   Address in original firmware binary.
 * @fw_ver     1.3.6r       Firmware version this was reversed from.
 * @source     re           How we know what this does:
 *                            re  = reverse engineered from Ghidra decompilation
 *                            sdk = wrapper around known SDK function
 *                            inf = inferred from context / call graph
 */
```

### Inline

```c
/* --- Section header for grouping related code --- */

/* Single line explanation of non-obvious logic */

/*
 * Multi-line explanation for complex blocks.
 * Explain WHY not just WHAT — the what is visible
 * from the code itself.
 */

/* RE: explains something specific to the reverse engineering */
/* e.g. RE: Ghidra decompiled this as PTR_DAT_0000bb00 + magic */

/* SDK: notes where SDK behavior is being relied upon */
/* e.g. SDK: ota_program_offset set by cpu_wakeup_init internally */

/* TODO: something that needs verification or implementation */
/* FIXME: known issue that needs fixing */
/* HACK: workaround for something not fully understood */
```

### File Headers

```c
/**
 * @file    filename.c
 *
 * @brief   One line description of this file's purpose.
 *
 * @details Longer description if needed. For reimplemented files,
 *          describe what original firmware functionality this covers
 *          and what was left out.
 *
 * @author  crackheadakira
 * @date    March 20, 2026
 *
 * @fw_ver  1.3.6r
 */
```

### Structs

```c
/**
 * @brief   Description of what this struct represents.
 *
 * @fw_addr 0x0000XXXX   Base address of this structure in RAM.
 * @fw_ver  1.3.6r
 * @note    Field names are interpretations based on observed usage.
 *          Original firmware accesses these as raw offsets e.g. *(iVar + 0x08).
 */
```

### Constants

```c
/* -------------------------------------------------------
 * Section name — group related constants together
 * ------------------------------------------------------- */

/** Brief description of what this constant represents. */
#define MAGIC_HI     0xAA    /** High byte of HID packet magic number. */
```

## Naming Conventions

### Files

Files map directly to prefixes used in their functions:

- `usb.c` → `usb_*`
- `dfu.c` → `dfu_*`
- `flash.c` → `flash_*`
- `hw.c` → `hw_*`

### Functions

- Public functions: `module_verb_noun()` e.g. `dfu_handle_packet()`
- Static/private functions: `verb_noun()` e.g. `validate_packet()`
- Init functions: `module_init()` e.g. `usb_hid_init()`
- Poll/tick functions: `module_poll()` e.g. `usb_poll()`

### Variables

- Globals: `g_` prefix e.g. `g_dfu_state`
- Static file-scope: `s_` prefix e.g. `s_ep_toggle`
- Booleans: `is_` or `has_` prefix e.g. `is_dfu_initialized`
- Pointers to hardware registers: match SDK names exactly

### Types

- Structs always end in `_t` e.g. `dfu_state_t`
- Enums always end in `_e` e.g. `flash_mid_e`
- No typedef for enums used as flags

### Constants

- `#define` constants: `UPPER_SNAKE_CASE`
- Enum values: `UPPER_SNAKE_CASE`
- No magic numbers in code — all constants must be named

## File Structure

### C File Structure

1. File header comment
2. Includes (system headers, then SDK headers, then local headers)
3. Private constants and macros
4. Private type definitions
5. Private global variables (`static`)
6. Public global variables (with `extern` in matching `.h`)
7. Private function prototypes (`static`)
8. Public function implementations
9. Private function implementations

### Header File Structure

1. File header comment
2. `#pragma once`
3. Includes required by this header only
4. Public constants and macros
5. Public type definitions
6. `extern` declarations for public globals
7. Public function prototypes

## Reverse Engineering Conventions

### Address References

When referencing original firmware addresses, always use the format:
`@ 0x0000XXXX` in comments. This allows grepping for all RE references.

### Ghidra Names

When a Ghidra-generated name is referenced, prefix with `DAT_` or `FUN_`
to distinguish from reimplemented names:

```c
/* RE: Originally DAT_0000bb1c in Ghidra — response buffer pointer */
uint8_t g_hid_response_buf[64];
```

### Unknown Behavior

When behavior is unclear, document both what the code does AND
what it might mean:

```c
/* RE: Original sets this to 0x36. Meaning unclear — possibly
 * USB full-speed mode select. Confirmed functional via testing. */
reg_usb_ctrl = 0x36;
```

### SDK Boundaries

Clearly mark where SDK behavior is relied upon vs reimplemented:

```c
/* SDK: cpu_wakeup_init() sets ota_program_offset internally
 * based on flash capacity. We rely on this side effect.
 * See: tc_ble_single_sdk/drivers/B87/... */
cpu_wakeup_init(DCDC_MODE, INTERNAL_CAP_XTAL24M);
```

## Global State

All mutable state for a subsystem must live in a single struct:

```c
typedef struct {
    uint8_t  initialized;
    uint32_t flash_offset;
} dfu_state_t;

extern dfu_state_t g_dfu;
```

Never use scattered individual globals for related state.
Reset functions must use `memset` on the struct then re-initialize
non-zero defaults:

```c
void dfu_state_reset(void) {
    memset(&g_dfu, 0, sizeof(dfu_state_t));
    g_dfu.crc_bank0 = CRC_INIT;  /* non-zero default */
}
```

## Function Complexity

- Switch statements with complex cases should delegate to helpers:

```c
/* Preferred */
void hid_flash_handler(hid_rx_packet_t *pkt) {
    switch (pkt->command) {
        case CMD_DATA: handle_data(pkt); break;
    }
}

static void handle_data(hid_rx_packet_t *pkt) {
    /* implementation here */
}

/* Avoid */
void hid_flash_handler(hid_rx_packet_t *pkt) {
    switch (pkt->command) {
        case CMD_DATA:
            /* 80 lines of implementation here */
            break;
    }
}
```

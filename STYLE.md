# Style

This is a documentation meant to aid in writing documentation for this software.

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

## Inline

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

## File Headers

```c
/**
 * @file       filename.c
 * @brief      One line description of this file's purpose.
 *
 * @details    Longer description if needed. For reimplemented files,
 *             describe what original firmware functionality this covers
 *             and what was left out.
 *
 * @fw_ver     1.3.6r
 * @fw_addr    0x0000XXXX - 0x0000YYYY  (address range in original firmware)
 *
 * @note       This file was developed through reverse engineering of the
 *             8BitDo Retro Keyboard firmware for interoperability purposes.
 *             Function names and behavior are based on Ghidra decompilation
 *             and USB packet captures. Some details may be incorrect.
 */
```

## Structs

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

## Constants

```c
/* -------------------------------------------------------
 * Section name — group related constants together
 * ------------------------------------------------------- */

/** Brief description of what this constant represents. */
#define MAGIC_HI     0xAA    /** High byte of HID packet magic number. */
```

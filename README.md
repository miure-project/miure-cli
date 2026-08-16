<!-- 
SPDX-FileCopyrightText: 2026 miure-project
SPDX-License-Identifier: GPL-3.0-or-later
-->

# miure-cli

A small C library providing reusable building blocks for command-line interfaces.

`miure-cli` provides two core APIs:

* **Terminal output** — buffered output, styling, and structured CLI presentation.
* **Argument iteration** — lightweight parsing and traversal of positional arguments, flags, and key-value pairs.

The library is designed to keep CLI implementations small while providing a consistent interface across the Miure ecosystem.

## API

The public API is provided through a single header:

```c
#include <miure/cli.h>
```

### Terminal Output

`mterm_buf` provides buffered terminal output with optional TTY-aware styling.

```c
struct mterm_buf buf;

mterm_buf_init(&buf, stderr);

mterm_header(&buf, "miurecfg", "Miure configuration tool");
mterm_section(&buf, "Configuration");

mterm_entry(&buf, "kernel", "Configure the Miure kernel", 1);
mterm_entry(&buf, "output", "Select the output format", 1);

mterm_buf_flush(&buf, stderr);
mterm_buf_free(&buf);
```

Basic output operations are available through:

```c
mterm_buf_puts(&buf, "Hello\n");
mterm_buf_putc(&buf, '\n');
mterm_buf_printf(&buf, "value: %d\n", value);
```

### Terminal Styles

Terminal styles can be configured with `mterm_style`:

```c
struct mterm_style style = {
    .bold = 1,
    .dim = 0,
    .underline = 0,
    .fg = MTERM_COLOR_GREEN,
};

mterm_buf_style(&buf, style);
mterm_buf_puts(&buf, "success");
mterm_buf_reset(&buf);
```

Available foreground colors:

```text
MTERM_COLOR_DEFAULT
MTERM_COLOR_RED
MTERM_COLOR_GREEN
MTERM_COLOR_YELLOW
MTERM_COLOR_BLUE
MTERM_COLOR_MAGENTA
MTERM_COLOR_CYAN
MTERM_COLOR_WHITE
MTERM_COLOR_GRAY
```

### Structured Output

The terminal API also provides helpers for common CLI layouts:

```c
mterm_header()
mterm_section()
mterm_entry()
mterm_example()
mterm_note()
```

These are intended for consistent human-readable command-line output.

## Argument Iterator

`marg_iter` provides a lightweight iterator over `argc`/`argv`.

```c
struct marg_iter it = marg_iter_new(argc, argv);
struct marg arg;

while (marg_iter_next(&it, &arg)) {
    switch (arg.type) {
    case MARG_TYPE_POSITIONAL:
        /* arg.value */
        break;

    case MARG_TYPE_FLAG:
        /* arg.key */
        break;

    case MARG_TYPE_PAIR:
        /* arg.key + arg.value */
        break;

    default:
        break;
    }
}
```

The iterator supports three primary argument types:

| Type                   | Example         | Key       | Value  |
| ---------------------- | --------------- | --------- | ------ |
| `MARG_TYPE_POSITIONAL` | `foo`           | `NULL`    | `foo`  |
| `MARG_TYPE_FLAG`       | `--verbose`     | `verbose` | `NULL` |
| `MARG_TYPE_PAIR`       | `--output file` | `output`  | `file` |

`MARG_TYPE_END` represents the end of the argument stream.

### Manual Argument Access

For cases where automatic iteration is insufficient, the iterator exposes raw argument access:

```c
const char *next = marg_iter_peek(&it);
const char *arg  = marg_iter_take(&it);
```

`marg_iter_peek()` inspects the next argument without consuming it.

`marg_iter_take()` consumes and returns the next raw argument.

### Named Output Arguments

`marg_named_output()` extracts a name from arguments following the `--NAME-o` convention.

```c
char name[64];

if (marg_named_output("kernel-o", name, sizeof(name))) {
    /* name contains "kernel" */
}
```

### Program Name

`marg_prog_name()` provides a portable basename for `argv[0]`:

```c
const char *name = marg_prog_name(argv[0]);
```

## Design

`miure-cli` intentionally keeps the API small.

The library does not attempt to provide a complete command framework, subcommand router, configuration system, or argument schema language. Instead, it provides low-level primitives that CLI applications can compose according to their own needs.

This makes the library suitable for both small standalone utilities and larger tools within the Miure ecosystem.

## Building

The library can be built as a shared library:

```bash
make
```

The resulting library can then be linked by applications using the public `cli.h` header.

For example:

```bash
cc main.c -lmiure-cli -o my-app
```

## License

See the repository license for licensing information.

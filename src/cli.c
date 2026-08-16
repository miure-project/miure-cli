// SPDX-FileCopyrightText: 2026 miure-project
// SPDX-License-Identifier: GPL-3.0-or-later

// cli.c

#include <miure/cli.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

// ============================================================
// Term — internal
// ============================================================

static int mterm_color_enabled(struct mterm_buf *b) {
  if (!b->is_tty)
    return 0;
  if (getenv("NO_COLOR"))
    return 0;
  return 1;
}

static void mterm_buf_push(struct mterm_buf *b, const char *s, size_t n) {
  if (b->len + n + 1 > b->cap) {
    b->cap = (b->cap + n + 1) * 2;
    b->buf = realloc(b->buf, b->cap);
  }
  memcpy(b->buf + b->len, s, n);
  b->len += n;
  b->buf[b->len] = '\0';
}

// ============================================================
// Term — public
// ============================================================

void mterm_buf_init(struct mterm_buf *b, struct mterm_theme *t, FILE *f) {
  b->buf = NULL;
  b->len = 0;
  b->cap = 0;
  b->is_tty = isatty(fileno(f));
  b->theme =
      t ? *t
        : (struct mterm_theme){
              .header_title =
                  (struct mterm_style){.bold = 1, .fg = MTERM_COLOR_WHITE},
              .header_tagline = (struct mterm_style){.dim = 1},
              .section_title =
                  (struct mterm_style){.bold = 1, .fg = MTERM_COLOR_YELLOW},
              .entry_title = (struct mterm_style){.fg = MTERM_COLOR_CYAN},
              .entry_desc = (struct mterm_style){.dim = 1},
              .example_message = (struct mterm_style){.fg = MTERM_COLOR_GRAY},
              .note_message = (struct mterm_style){.dim = 1},
              .error_title =
                  (struct mterm_style){.bold = 1, .fg = MTERM_COLOR_RED},
              .error_message = (struct mterm_style){.dim = 1},
              .indent = 2,
              .entry_column_width = 30
          };

#ifdef _WIN32
  if (b->is_tty) {
    HANDLE h = GetStdHandle(f == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
#endif
}

void mterm_buf_flush(struct mterm_buf *b, FILE *f) {
  if (b->buf && b->len > 0)
    fwrite(b->buf, 1, b->len, f);
  b->len = 0;
}

void mterm_buf_free(struct mterm_buf *b) {
  free(b->buf);
  b->buf = NULL;
  b->len = 0;
  b->cap = 0;
}

void mterm_indent(struct mterm_buf *b, const unsigned int indent,
                      const char c) {
  char s[indent];
  __builtin_memset(s, c, indent);
  mterm_buf_push(b, s, indent);
}

void mterm_buf_puts(struct mterm_buf *b, const char *s) {
  mterm_buf_push(b, s, strlen(s));
}

void mterm_buf_putc(struct mterm_buf *b, char c) { mterm_buf_push(b, &c, 1); }

void mterm_buf_printf(struct mterm_buf *b, const char *fmt, ...) {
  char tmp[1024];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  if (n > 0)
    mterm_buf_push(b, tmp, (size_t)n);
}

void mterm_buf_style(struct mterm_buf *b, struct mterm_style s) {
  if (!mterm_color_enabled(b))
    return;

  static const int fg_codes[] = {39, 31, 32, 33, 34, 35, 36, 37, 90};

  mterm_buf_puts(b, "\033[");
  int first = 1;

#define SEP() do { if (!first) mterm_buf_putc(b, ';'); first = 0; } while(0)
    if (s.bold)      { SEP(); mterm_buf_puts(b, "1"); }
    if (s.dim)       { SEP(); mterm_buf_puts(b, "2"); }
    if (s.underline) { SEP(); mterm_buf_puts(b, "4"); }
    if (s.fg)        { SEP(); mterm_buf_printf(b, "%d", fg_codes[s.fg]); }
#undef SEP

  if (first)
    mterm_buf_puts(b, "0");
  mterm_buf_putc(b, 'm');
}

void mterm_buf_reset(struct mterm_buf *b) {
  if (!mterm_color_enabled(b))
    return;
  mterm_buf_puts(b, "\033[0m");
}

void mterm_header(struct mterm_buf *b, struct mterm_style *s_title,
                  struct mterm_style *s_tagline, const char *name,
                  const char *tagline) {
  mterm_buf_style(b, s_title ? *s_title : b->theme.header_title);
  mterm_buf_puts(b, name);
  mterm_buf_reset(b);
  if (tagline) {
    mterm_buf_puts(b, " — ");
    mterm_buf_style(b,
                    s_tagline ? *s_tagline : b->theme.header_tagline);
    mterm_buf_puts(b, tagline);
    mterm_buf_reset(b);
  }
  mterm_buf_putc(b, '\n');
}

void mterm_section(struct mterm_buf *b, struct mterm_style *s,
                   const char *title) {
  mterm_buf_putc(b, '\n');
  mterm_buf_style(b, s ? *s : b->theme.section_title);
  mterm_buf_puts(b, title);
  mterm_buf_reset(b);
  mterm_buf_putc(b, '\n');
}

void mterm_entry(struct mterm_buf *b, struct mterm_style *s_title,
                 struct mterm_style *s_desc, const char *name, const char *desc,
                 unsigned short indent) {
  mterm_indent(b, indent == (unsigned short)-1 ? b->theme.indent : indent, ' ');
  mterm_buf_style(b, s_title ? *s_title : b->theme.entry_title);
  // mterm_buf_printf(b, "%*s", -(indent == (sizeof(unsigned short) - 1) ? b->theme.entry_column_width : indent), name);
  mterm_buf_printf(b, "%*s", -((int)b->theme.entry_column_width), name);
  mterm_buf_reset(b);
  if (desc) {
    mterm_buf_style(b, s_desc ? *s_desc : b->theme.entry_desc);
    mterm_buf_puts(b, desc);
    mterm_buf_reset(b);
  }
  mterm_buf_putc(b, '\n');
}

void mterm_example(struct mterm_buf *b, struct mterm_style *s,
                   const char *text) {
  mterm_indent(b, b->theme.indent, ' ');
  mterm_buf_style(b, s ? *s : b->theme.example_message);
  mterm_buf_puts(b, text);
  mterm_buf_reset(b);
  mterm_buf_putc(b, '\n');
}

void mterm_note(struct mterm_buf *b, struct mterm_style *s, const char *text) {
  mterm_indent(b, b->theme.indent, ' ');
  mterm_buf_style(b, s ? *s : b->theme.note_message);
  mterm_buf_puts(b, text);
  mterm_buf_reset(b);
  mterm_buf_putc(b, '\n');
}

void mterm_error(struct mterm_buf *b, struct mterm_style *s_title,
                 struct mterm_style *s_message, const char *text) {
  mterm_buf_style(b, s_title ? *s_title : b->theme.error_title);
  mterm_buf_puts(b, "error: ");
  mterm_buf_reset(b);
  mterm_buf_style(b, s_message ? *s_message : b->theme.error_message);
  mterm_buf_puts(b, text);
  mterm_buf_reset(b);
  mterm_buf_putc(b, '\n');
}

// ============================================================
// Arg iterator
// ============================================================

struct marg_iter marg_iter_new(int argc, char **argv) {
  return (struct marg_iter){.argc = argc, .argv = argv, .pos = 1};
}

int marg_iter_next(struct marg_iter *it, struct marg *out) {
  if (it->pos >= it->argc) {
    out->type = MARG_TYPE_END;
    out->key = NULL;
    out->value = NULL;
    return 0;
  }

  const char *cur = it->argv[it->pos++];

  if (strncmp(cur, "--", 2) == 0) {
    out->key = cur + 2;

    // peek: if next exists and is not a flag, treat as pair
    if (it->pos < it->argc && it->argv[it->pos][0] != '-') {
      out->type = MARG_TYPE_PAIR;
      out->value = it->argv[it->pos++];
    } else {
      out->type = MARG_TYPE_FLAG;
      out->value = NULL;
    }

  } else if (cur[0] == '-' && cur[1] != '\0') {
    out->key = cur + 1;

    if (it->pos < it->argc && it->argv[it->pos][0] != '-') {
      out->type = MARG_TYPE_PAIR;
      out->value = it->argv[it->pos++];
    } else {
      out->type = MARG_TYPE_FLAG;
      out->value = NULL;
    }

  } else {
    out->type = MARG_TYPE_POSITIONAL;
    out->key = NULL;
    out->value = cur;
  }

  return 1;
}

const char *marg_iter_peek(struct marg_iter *it) {
  if (it->pos >= it->argc)
    return NULL;
  return it->argv[it->pos];
}

const char *marg_iter_take(struct marg_iter *it) {
  if (it->pos >= it->argc)
    return NULL;
  return it->argv[it->pos++];
}

int marg_extract_inner(const char *key, const char *prefix, const char *suffix,
                       char *name_out, size_t name_cap) {
  if (!key || !name_out || name_cap == 0)
    return 0;

  size_t key_len = __builtin_strlen(key);
  size_t pref_len = prefix ? __builtin_strlen(prefix) : 0;
  size_t suff_len = suffix ? __builtin_strlen(suffix) : 0;

  if (key_len <= (pref_len + suff_len))
    return 0;

  if (pref_len > 0) {
    if (__builtin_strncmp(key, prefix, pref_len) != 0)
      return 0;
  }

  if (suff_len > 0) {
    const char *key_suff_start = key + (key_len - suff_len);
    if (__builtin_strcmp(key_suff_start, suffix) != 0)
      return 0;
  }

  size_t name_len = key_len - pref_len - suff_len;

  if (name_len >= name_cap)
    return 0;

  __builtin_memcpy(name_out, key + pref_len, name_len);
  name_out[name_len] = '\0';

  return 1;
}

const char *marg_prog_name(const char *argv0) {
  const char *p = strrchr(argv0, '/');
#ifdef _WIN32
  const char *q = strrchr(argv0, '\\');
  if (q > p)
    p = q;
#endif
  return p ? p + 1 : argv0;
}

void mterm_backspace(struct mterm_buf *b, unsigned short len) {
    if (len == 0) return;

    mterm_buf_printf(b, "\033[%dD", len);
}

void mterm_backspace_line(struct mterm_buf *b, unsigned short line_count) {
    if (!b) return;

    mterm_buf_printf(b, "\033[K");

    for (unsigned short i = 0; i < line_count; i++) {
        mterm_buf_printf(b, "\033[A\033[999C\033[K");
    }
}
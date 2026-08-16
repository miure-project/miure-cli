// SPDX-FileCopyrightText: 2026 miure-project
// SPDX-License-Identifier: GPL-3.0-or-later

// cli.h

#pragma once
#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdio.h>

// ============================================================
// Term
// ============================================================

enum mterm_color {
  MTERM_COLOR_DEFAULT = 0,
  MTERM_COLOR_RED,
  MTERM_COLOR_GREEN,
  MTERM_COLOR_YELLOW,
  MTERM_COLOR_BLUE,
  MTERM_COLOR_MAGENTA,
  MTERM_COLOR_CYAN,
  MTERM_COLOR_WHITE,
  MTERM_COLOR_GRAY,
};

struct mterm_style {
  int bold;
  int dim;
  int underline;
  enum mterm_color fg;
};

struct mterm_theme {
  struct mterm_style header_title;
  struct mterm_style header_tagline;
  struct mterm_style section_title;
  struct mterm_style entry_title;
  struct mterm_style entry_desc;
  struct mterm_style example_message;
  struct mterm_style note_message;
  struct mterm_style error_title;
  struct mterm_style error_message;
  unsigned short indent;
  unsigned short entry_column_width;
};

struct mterm_buf {
  struct mterm_theme theme;
  char *buf;
  size_t len;
  size_t cap;
  int is_tty;
};

void mterm_buf_init(struct mterm_buf *b, struct mterm_theme *t, FILE *f);
void mterm_buf_flush(struct mterm_buf *b, FILE *f);
void mterm_buf_free(struct mterm_buf *b);

void mterm_indent(struct mterm_buf *b, const unsigned int indent,
                      const char c);
void mterm_buf_puts(struct mterm_buf *b, const char *s);
void mterm_buf_putc(struct mterm_buf *b, const char c);
void mterm_buf_printf(struct mterm_buf *b, const char *fmt, ...);

void mterm_buf_style(struct mterm_buf *b, struct mterm_style s);
void mterm_buf_reset(struct mterm_buf *b);

void mterm_header(struct mterm_buf *b, struct mterm_style *s_title,
                  struct mterm_style *s_tagline, const char *name,
                  const char *tagline);
void mterm_section(struct mterm_buf *b, struct mterm_style *s,
                   const char *title);
void mterm_entry(struct mterm_buf *b, struct mterm_style *s_title,
                 struct mterm_style *s_desc, const char *name, const char *desc,
                 unsigned short indent);
void mterm_example(struct mterm_buf *b, struct mterm_style *s,
                   const char *text);
void mterm_note(struct mterm_buf *b, struct mterm_style *s, const char *text);
void mterm_error(struct mterm_buf *b, struct mterm_style *s_title,
                 struct mterm_style *s_message, const char *text);
void mterm_backspace(struct mterm_buf *b, unsigned short len);
void mterm_backspace_line(struct mterm_buf *b, unsigned short line_count);

// ============================================================
// Arg iterator
// ============================================================

enum marg_type {
  MARG_TYPE_POSITIONAL, // foo
  MARG_TYPE_FLAG,       // --foo, -f      (no value)
  MARG_TYPE_PAIR,       // --foo bar, -o bar, --foo-o bar
  MARG_TYPE_END,
};

struct marg {
  enum marg_type type;
  const char *key;   // flag name without dashes, or NULL for positional
  const char *value; // positional value, or paired value
};

struct marg_iter {
  int argc;
  char **argv;
  int pos;
};

struct marg_iter marg_iter_new(int argc, char **argv);
// returns 0 when done
int marg_iter_next(struct marg_iter *it, struct marg *out);
// peek at next raw argv without consuming
const char *marg_iter_peek(struct marg_iter *it);
// consume and return next raw argv (for manual pair handling)
const char *marg_iter_take(struct marg_iter *it);

// extract "NAME" from "--NAME-o", returns 0 if not that pattern
int marg_extract_inner(const char *key, const char *prefix, const char *suffix,
                       char *name_out, size_t name_cap);

// portable argv[0] basename
const char *marg_prog_name(const char *argv0);

#endif
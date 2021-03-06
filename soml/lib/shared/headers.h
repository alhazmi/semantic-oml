/*
 * Copyright 2010-2014 National ICT Australia (NICTA), Australia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef HEADERS_H__
#define HEADERS_H__

#include <stdlib.h>

enum HeaderTag {
  H_NONE,
  H_PROTOCOL,
  H_DOMAIN,
  H_CONTENT,
  H_APP_NAME,
  H_SENDER_ID,
  H_SCHEMA,
  H_START_TIME,
  H_max /* For calculating the max value for use in tables */
};

struct header {
  enum HeaderTag tag;
  char *value;
  struct header *next; /* For linked lists */
};

enum HeaderTag tag_from_string (const char *str, size_t n);
const char * tag_to_string (enum HeaderTag tag);
struct header *header_from_string (const char *str, size_t n);
void header_free (struct header *header);

#endif /* HEADERS_H__ */

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/

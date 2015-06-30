/*
 * Copyright 2010-2014 National ICT Australia (NICTA)
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
/** \file cbuf.c
 * \brief A FIFO implementation used by the proxy that uses a circular list of pages to manage the memory.
 * 
 * It always writes in the tail buffer and always reads from the read buffer;
 * tail->next points back to the start of the buffer:
 * 
 * \verbatim
 *  +------------------------------------------+
 *  V                                          |
 * [X] -> [X] -> ... [X] -> ... -> [X] -> [X] -+
 *                    ^                    ^
 *                    |_ read              |_ tail
 * \endverbatim
 *
 * When the tail buffer fills up, add a new one after tail, link the
 * new tail to the head, and keep writing.
 *
 * To remove a buffer from the head, link the tail to the next one.
 *
 * Could also implement this as a circular chain where beyond a certain
 * size the chain does not expand, by having an empty flag on the nodes
 * and allowing the tail to process, rather than inserting new nodes at
 * the tail.
 */
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "cbuf.h"

#define CBUFFER_DEFAULT_SIZE 1024

CBuffer*
cbuf_create(int default_size)
{
  CBuffer *cbuf = NULL;
  if (default_size <= 0)
    default_size = CBUFFER_DEFAULT_SIZE;

  cbuf = oml_malloc (sizeof (CBuffer));

  if (cbuf == NULL)
    return NULL;

  cbuf->page_size = default_size;

  if (cbuf_add_page (cbuf, -1) == -1) {
    oml_free (cbuf);
    return NULL;
  }

  return cbuf;
}

void
cbuf_destroy (CBuffer *cbuf)
{
  if (cbuf == NULL)
    return;

  if (cbuf->tail != NULL) {
    struct cbuffer_page *head, *current, *next;
    current = cbuf->tail;
    head = cbuf->tail->next;

    do {
      if (current->buf)
        oml_free (current->buf);
      next = current->next;
      oml_free (current);
      current = next;
    } while (current != NULL && current != head);
  }
  oml_free (cbuf);
}

int
cbuf_add_page (CBuffer *cbuf, int size)
{
  struct cbuffer_page *page;

  if (cbuf == NULL)
    return -1;

  if (size <= 0)
    size = cbuf->page_size;

  page = oml_malloc (sizeof (struct cbuffer_page));

  if (page == NULL)
    return -1;

  page->buf = oml_malloc (size);

  if (page->buf == NULL) {
    oml_free (page);
    return -1;
  }

  page->empty = 1;
  page->fill = 0;
  page->size = size;
  page->read = 0;
  page->next = NULL;

  if (cbuf->tail == NULL) {
    cbuf->tail = page;
    cbuf->read = page;
    cbuf->tail->next = cbuf->tail;
  } else {
    struct cbuffer_page *head = cbuf->tail->next;
    cbuf->tail->next = page;
    cbuf->tail = page;
    cbuf->tail->next = head;
  }
  return 0;
}

int
cbuf_write (CBuffer *cbuf, char *buf, size_t size)
{
  int count = 0;

  if (cbuf == NULL || buf == NULL)
    return -1;

  do {
    struct cbuffer_page *page = cbuf->tail;
    size_t remain = (page->size - page->fill);
    size_t to_write = size < remain ? size : remain;

    memcpy (&page->buf[page->fill], buf + count, to_write);
    page->empty = 0;
    page->fill += to_write;
    count += to_write;

    if (size > to_write) {
      if (page->next->empty) {
        cbuf->tail = page->next;
      } else {
        if (cbuf_add_page (cbuf, -1) == -1) {
          break;
        }
      }
    }
    size -= to_write;
  } while (size > 0);

  return count;
}

/**
 *  Get a cursor pointing to the current write position in the buffer
 *  chain.
 */

void
cbuf_write_cursor (CBuffer *cbuf, struct cbuffer_cursor *cursor)
{
  if (cbuf == NULL || cursor == NULL)
    return;

  cursor->page = cbuf->tail;
  cursor->index = cursor->page->fill;
}

/**
 *  Get a cursor pointing to the current read position in the buffer
 *  chain.  Returns the number of bytes between the cursor position
 *  and the end of the page the cursor points to.
 *
 */
int
cbuf_read_cursor (CBuffer *cbuf, struct cbuffer_cursor *cursor, size_t n)
{
  if (cbuf == NULL || cursor == NULL)
    return -1;

  cursor->page = cbuf->read;
  cursor->index = cursor->page->read;

  size_t remainder = cursor->page->fill - cursor->index;

  if (remainder < n)
    return remainder;
  else
    return n;
}

char*
cbuf_cursor_pointer (struct cbuffer_cursor *cursor)
{
  return cursor->page->buf + cursor->index;
}

/**
 *  Get the number of bytes remaining on the page following the
 *  cursor.
 */
size_t
cbuf_cursor_page_remaining (struct cbuffer_cursor *cursor)
{
  return cursor->page->fill - cursor->index;
}


/*
 * Advance the cursor, but don't mark the page as empty if we go past the end.
 */
int
cbuf_advance_cursor (struct cbuffer_cursor *cursor, size_t n)
{
  int count = 0;
  do {
    size_t page_remaining = cbuf_cursor_page_remaining (cursor);
    size_t to_consume = n < page_remaining ? n : page_remaining;
    count += to_consume;
    cursor->index += to_consume;
    n -= to_consume;

    /* If we've read to the end of this page, skip to the next page */
    if (cursor->index == cursor->page->fill) {
      /* Advance to the next page */
      cursor->page = cursor->page->next;
      cursor->index = 0;
    }
  } while (n > 0);

  return count;
}

/**
 * a
 */
int
cbuf_consume_cursor (struct cbuffer_cursor *cursor, size_t n)
{
  int count = 0;
  do {
    size_t page_remaining = cbuf_cursor_page_remaining (cursor);
    size_t to_consume = n < page_remaining ? n : page_remaining;
    count += to_consume;
    cursor->index += to_consume;
    n -= to_consume;

    /* If we've read to the end of this page, skip to the next page */
    if (cursor->index == cursor->page->fill) {
      /* Reset the current page to empty */
      cursor->page->empty = 1;
      cursor->page->read = 0;
      cursor->page->fill = 0;

      /* Advance to the next page */
      cursor->page = cursor->page->next;
      cursor->index = 0;
    }
  } while (n > 0);

  cursor->page->read = cursor->index;
  //  cbuf->read = cursor->page; /* Not sure about this */
  return count;
}

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/

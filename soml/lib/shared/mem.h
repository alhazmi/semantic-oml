#ifndef MEM_H__
#define MEM_H__

#include <stdint.h>
#include <stdlib.h>

#include "ocomm/o_log.h"

void *oml_malloc (size_t size);
void *oml_calloc (size_t count, size_t size);
void *oml_realloc (void *ptr, size_t size);
size_t oml_malloc_usable_size(void *ptr);
char *oml_stralloc (size_t len);
void *oml_memdupz (const void *data, size_t len);
char *oml_strndup (const char *str, size_t len);
void oml_free (void *ptr);

size_t xmembytes();
size_t xmemnew();
size_t xmemfreed();
size_t xmaxbytes();
char *oml_memsummary ();
char *oml_memsummary_r (char *s, size_t s_sz);
void oml_memreport (int loglevel);

/* Duplicate nil-terminated string
 *
 * \param str string to copy in the new xchunk
 * \return the newly allocated xchunk, or NULL
 * \see oml_strndup
 * \see strndup(3), strlen(3)
 */
#define xstrdup(str)  oml_strndup((str), strlen((str)))

#endif /* MEM_H__ */

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/

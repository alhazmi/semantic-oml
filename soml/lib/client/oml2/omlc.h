/*
 * Copyright 2007-2014 National ICT Australia (NICTA)
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
/** \file omlc.h
 * \brief Public API of the OML client library.
 */

#ifndef OML_OMLC_H_
#define OML_OMLC_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <ocomm/o_log.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OmlValueT {
  /* Meta value types */
  OML_DB_PRIMARY_KEY = -3,
  OML_INPUT_VALUE = -2,
  OML_UNKNOWN_VALUE = -1,

  /* Concrete value types */
  OML_DOUBLE_VALUE = 0,
  OML_LONG_VALUE,
  OML_PADDING1_VALUE,
  OML_STRING_VALUE,
  OML_INT32_VALUE,
  OML_UINT32_VALUE,
  OML_INT64_VALUE,
  OML_UINT64_VALUE,
  OML_BLOB_VALUE,
  OML_GUID_VALUE,
  OML_BOOL_VALUE,

  OML_DATETIME_VALUE,

  /* Vector types */
  OML_VECTOR_DOUBLE_VALUE,
  OML_VECTOR_INT32_VALUE,
  OML_VECTOR_UINT32_VALUE,
  OML_VECTOR_INT64_VALUE,
  OML_VECTOR_UINT64_VALUE,
  OML_VECTOR_BOOL_VALUE,


  OML_LAST_VALUE /* For easy range checks */
} OmlValueT;

#define omlc_is_integer_type(t)                             \
  (((t) == OML_LONG_VALUE) ||                               \
   ((t) == OML_INT32_VALUE) ||                              \
   ((t) == OML_UINT32_VALUE) ||                             \
   ((t) == OML_INT64_VALUE) ||                              \
   ((t) == OML_UINT64_VALUE))

#define omlc_is_numeric_type(t)                             \
  ((omlc_is_integer_type(t)) ||                             \
   ((t) == OML_DOUBLE_VALUE))

#define omlc_is_string_type(t) \
  ((t) == OML_STRING_VALUE)
#define omlc_is_blob_type(t) \
  ((t) == OML_BLOB_VALUE)
#define omlc_is_guid_type(t) \
  ((t) == OML_GUID_VALUE)
#define omlc_is_bool_type(t) \
  ((t) == OML_BOOL_VALUE)
#define omlc_is_vector_type(t) \
  (((t) == OML_VECTOR_DOUBLE_VALUE) ||                       \
   ((t) == OML_VECTOR_INT32_VALUE)  ||                       \
   ((t) == OML_VECTOR_UINT32_VALUE) ||                       \
   ((t) == OML_VECTOR_INT64_VALUE)  ||                       \
   ((t) == OML_VECTOR_UINT64_VALUE) ||                       \
   ((t) == OML_VECTOR_BOOL_VALUE))
#define omlc_is_scalar_type(t) \
  (!omlc_isvector_type(t))

#define omlc_is_datetime_type(t) \
  ((t) == OML_DATETIME_VALUE)


#define omlc_is_integer(v) \
  omlc_is_integer_type((v).type)
#define omlc_is_numeric(v) \
  omlc_is_numeric_type((v).type)
#define omlc_is_string(v) \
  omlc_is_string_type((v).type)
#define omlc_is_blob(v) \
  omlc_is_blob_type((v).type)
#define omlc_is_guid(v) \
  omlc_is_guid_type((v).type)
#define omlc_is_bool(v) \
  omlc_is_bool_type((v).type)
#define omlc_is_vector(v) \
  omlc_is_vector_type((v).type)

#define omlc_is_datetime(v) \
  omlc_is_datetime_type((v).type)

/**  Representation of a vector value. */
typedef struct OmlVector {

  /** Pointer to the base of the vector */
  void *ptr;

  /** The amount of storage (in bytes) used by this vector */
  size_t length;

  /** The size of underlying allocated storage (in bytes) in ptr */
  size_t size;

  /** The number of elements of this vector */
  uint16_t nof_elts;

  /** The size (in octets) of an individual element */
  uint16_t elt_sz;

} OmlVector;

/**  Representation of a string measurement value.  */
typedef struct OmlString {
  /** Pointer to a nil-terminated C string */
  char *ptr;

  /** Length of string pointed to by ptr, not including nil-terminator */
  size_t length;

  /** Size of the allocated underlying storage in ptr (>= length + 1) */
  size_t size;

  /** True if ptr references const storage */
  int is_const;

} OmlString;

/**  Representation of a blob measurement value.  */
typedef struct OmlBlob {
  /** Pointer to blob data storage */
  void *ptr;

  /** Length of the blob pointed to by ptr */
  size_t length;

  /** Size of the allocated underlying storage in ptr (>= length) */
  size_t size;

} OmlBlob;

/** An opaque type to represent globally unique IDs.  */
typedef uint64_t oml_guid_t;

#define OMLC_GUID_NULL ((oml_guid_t) 0)

#define OMLC_BOOL_FALSE 0
#define OMLC_BOOL_TRUE (!0)

/** Multi-typed variable container without type information.
 *
 * WARNING: OmlValueU MUST be omlc_zero()d out before use. Additionally, if the
 * last type of data they contained was an OML_STRING_VALUE, OML_BLOB_VALUE or
 * OML_VECTOR_*_VALUE, they should be omlc_reset_(string|blob|vector)(). Not doing
 * so might result in memory problems (double free or memory leak).
 *
 * When wrapped in OmlValue, the right thing is done, by the initialisation/reset functions.
 *
 * \see omlc_zero, omlc_zero_array, omlc_reset_string, omlc_reset_blob
 * \see oml_value_init, oml_value_array_init, oml_value_reset, oml_value_array_reset
 */
typedef union OmlValueU {
  long      longValue;
  double    doubleValue;
  OmlString stringValue;
  int32_t   int32Value;
  uint32_t  uint32Value;
  int64_t   int64Value;
  uint64_t  uint64Value;
  OmlBlob   blobValue;
  oml_guid_t    guidValue;
  uint8_t   boolValue;
  OmlVector vectorValue;
  OmlString datetimeValue;
} OmlValueU;

/* Declarations from internal "mem.h", to be used in the macros below
 * 
 * DO NOT USE DIRECTLY IN CLIENT APPLICATIONS!
 */
void *oml_malloc (size_t size);
void oml_free (void *ptr);
size_t oml_malloc_usable_size(void *ptr);

/** Zero out a freshly declared OmlValueU.
 *
 * \param var OmlValueU to manipulate
 */
#define omlc_zero(var) \
  memset(&(var), 0, sizeof(OmlValueU))

/** Zero out a freshly declared array of OmlValueU.
 *
 * \param var OmlValueU to manipulate
 * \param n number of OmlValueU in the array
 */
#define omlc_zero_array(var, n) \
  memset((var), 0, n*sizeof(OmlValueU))

/** Get an intrinsic C value from an OmlValueU.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to manipulate
 * \param type type of data contained in the OmlValueU
 * \return the value
 * \see omlc_get_int32, omlc_get_uint32, omlc_get_int64, omlc_get_uint64, omlc_get_double, omlc_get_guid, omlc_get_bool
 */
#define _omlc_get_intrinsic_value(var, type) \
  ((var).type ## Value)

/** Set an intrinsic C value in an OmlValueU.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to manipulate
 * \param type type of data contained in the OmlValueU
 * \param val value to store in the OmlValueU
 * \return the new value (val)
 * \see omlc_set_int32, omlc_set_uint32, omlc_set_int64, omlc_set_uint64, omlc_set_double, omlc_set_guid, omlc_get_bool
 */
#define _omlc_set_intrinsic_value(var, type, val) \
  ((var).type ## Value = (val))

/** \see _omlc_get_intrinsic_value */
#define omlc_get_int32(var) \
  ((int32_t)(_omlc_get_intrinsic_value(var, int32)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_uint32(var) \
  ((uint32_t)(_omlc_get_intrinsic_value(var, uint32)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_int64(var) \
  ((int64_t)(_omlc_get_intrinsic_value(var, int64)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_uint64(var) \
  ((uint64_t)(_omlc_get_intrinsic_value(var, uint64)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_double(var) \
  ((double)(_omlc_get_intrinsic_value(var, double)))
/** DEPRECATED \see omlc_get_uint32, omlc_get_uint64 */
#define omlc_get_long(var) \
  ((long)(_omlc_get_intrinsic_value(var, long)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_guid(var) \
  ((oml_guid_t)(_omlc_get_intrinsic_value(var, guid)))
/** \see _omlc_get_intrinsic_value */
#define omlc_get_bool(var) \
  ((_omlc_get_intrinsic_value(var, bool)) != OMLC_BOOL_FALSE)

#define omlc_get_datetime(var) \
  ((_omlc_get_intrinsic_value(var, datetime)))


/** \see _omlc_set_intrinsic_value */
#define omlc_set_int32(var, val) \
  _omlc_set_intrinsic_value(var, int32, (int32_t)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_uint32(var, val) \
  _omlc_set_intrinsic_value(var, uint32, (uint32_t)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_int64(var, val) \
  _omlc_set_intrinsic_value(var, int64, (int64_t)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_uint64(var, val) \
  _omlc_set_intrinsic_value(var, int64, (uint64_t)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_double(var, val) \
  _omlc_set_intrinsic_value(var, double, (double)(val))
/** DEPRECATED \see omlc_set_uint32, omlc_set_uint64 */
#define omlc_set_long(var, val) \
  _omlc_set_intrinsic_value(var, long, (long)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_guid(var, val) \
  _omlc_set_intrinsic_value(var, guid, (oml_guid_t)(val))
/** \see _omlc_set_intrinsic_value */
#define omlc_set_bool(var, val) \
  _omlc_set_intrinsic_value(var, bool, (val != OMLC_BOOL_FALSE))

/** Get fields of an OmlValueU containing pointer to possibly dynamically allocated storage.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour, but have less parameters.
 *
 * \param var OmlValueU to manipulate
 * \param type type of data contained in the OmlValueU
 * \param field field to access
 * \return the value of the field
 * \see OmlString, omlc_get_string_ptr, omlc_get_string_length, omlc_get_string_size, omlc_get_string_is_const
 * \see OmlBlob, omlc_get_blob_ptr, omlc_get_blob_length, omlc_get_blob_size
 */
#define _oml_get_storage_field(var, type, field) \
  ((var).type ## Value.field)

/** Set fields of an OmlValueU containing pointer to possibly dynamically allocated storage.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to manipulate
 * \param type type of data contained in the OmlValueU
 * \param field field to access
 * \param val value to set the field to
 * \return the newly-set value of the field (val)
 * \see OmlString, omlc_set_string_ptr, omlc_set_string_length, omlc_set_string_size, omlc_set_string_is_const
 * \see OmlBlob, omlc_set_blob_ptr, omlc_set_blob_length, omlc_set_blob_size
 */
#define _oml_set_storage_field(var, type, field, val) \
  ((var).type ## Value.field = (val))

/** \see _oml_get_storage_field */
#define omlc_get_string_ptr(var) \
  (_oml_get_storage_field(var, string, ptr))
/** Get string length (not including nil-terminator)
 * \see _oml_get_storage_field, strlen(3) */
#define omlc_get_string_length(var) \
  (_oml_get_storage_field(var, string, length))
/** \see _oml_get_storage_field */
#define omlc_get_string_size(var) \
  (_oml_get_storage_field(var, string, size))
/** \see _oml_get_storage_field */
#define omlc_get_string_is_const(var) \
  (_oml_get_storage_field(var, string, is_const))

/** \see _oml_set_storage_field */
#define omlc_set_string_ptr(var, val) \
  _oml_set_storage_field(var, string, ptr, (char*)(val))
/** Set string length (not including nil-terminator)
 * \see _oml_set_storage_field, strlen(3) */
#define omlc_set_string_length(var, val) \
  _oml_set_storage_field(var, string, length, (size_t)(val))
/** \see _oml_set_storage_field */
#define omlc_set_string_size(var, val) \
  _oml_set_storage_field(var, string, size, (size_t)(val))
/** \see _oml_set_storage_field */
#define omlc_set_string_is_const(var, val) \
  _oml_set_storage_field(var, string, is_const, (int)(val))

/** \see _oml_get_storage_field */
#define omlc_get_blob_ptr(var) \
  ((void *)_oml_get_storage_field(var, blob, ptr))
/** \see _oml_get_storage_field */
#define omlc_get_blob_length(var) \
  (_oml_get_storage_field(var, blob, length))
/** \see _oml_get_storage_field */
#define omlc_get_blob_size(var) \
  (_oml_get_storage_field(var, blob, size))

/** \see _oml_set_storage_field */
#define omlc_set_blob_ptr(var, val) \
  _oml_set_storage_field(var, blob, ptr, (char*)(val))
/** \see _oml_set_storage_field */
#define omlc_set_blob_length(var, val) \
  _oml_set_storage_field(var, blob, length, (size_t)(val))
/** \see _oml_set_storage_field */
#define omlc_set_blob_size(var, val) \
  _oml_set_storage_field(var, blob, size, (size_t)(val))

/** Free the storage contained in an OmlValueU if needed.
 *
 * Other metadata such as length is not changed, unlike _omlc_reset_storage.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to operate on
 * \param type type of data contained in the OmlValueU
 * \see omlc_free_string, omlc_free_blob, oml_free, _omlc_reset_storage
 */
#define _omlc_free_storage(var, type)                     \
  do {                                                    \
    if (_oml_get_storage_field((var), type, size) > 0) {  \
      oml_free(_oml_get_storage_field((var), type, ptr));    \
      _oml_set_storage_field((var), type, size, 0);       \
    }                                                     \
  } while(0)

/** Reset the contents of an OmlValueU, freeing allocated memory if needed.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to operate on
 * \param type type of data contained in the OmlValueU
 * \see omlc_reset_string, omlc_reset_blob, omlc_reset_vector, _omlc_free_storage
 */
#define _omlc_reset_storage(var, type)                    \
  do {                                                    \
    _omlc_free_storage((var), type);                      \
    _oml_set_storage_field((var), type, ptr, NULL);       \
    _oml_set_storage_field((var), type, length, 0);       \
  } while(0)

/** Copy data into the dedicated storage of an OmlValueU, allocating memory if needed.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have less parameters.
 *
 * \param var OmlValueU to manipulate
 * \param type type of data contained in the OmlValueU
 * \param str data to copy
 * \param len length of the data
 * \see omlc_set_string_copy, omlc_set_blob, oml_malloc
 * \see oml_malloc, memcpy(3)
 */
/* XXX: Does not check result of oml_malloc */
#define _omlc_set_storage_copy(var, type, data, len)                          \
  do {                                                                        \
    if (len >= _oml_get_storage_field((var), type, size)) {                   \
      _omlc_reset_storage((var), type);                                       \
      _oml_set_storage_field((var), type, ptr, oml_malloc(len));                 \
      _oml_set_storage_field((var), type, size,                               \
          oml_malloc_usable_size(_oml_get_storage_field((var), type, ptr)));     \
    }                                                                         \
    memcpy(_oml_get_storage_field((var), type, ptr), (void*)(data), (len));   \
    _oml_set_storage_field((var), type, length, (len));                       \
  } while(0)

/** \see _omlc_free_storage */
#define omlc_free_string(var) \
  _omlc_free_storage((var), string)
/** \see _omlc_reset_storage */
#define omlc_reset_string(var)          \
  do {                                  \
    _omlc_reset_storage((var), string); \
    omlc_set_string_is_const(var, 0);   \
  } while(0)

/** Copy a string into the dedicated storage of an OmlValueU.
 *
 * Allocate (or reuse) a buffer of size len+1, copy the string, and
 * nul-terminate it.
 *
 * The length attribute is the length of the string; not how much of the
 * storage is used (len + 1), as is the case for generic storage (blobs).
 *
 * \param var OmlValueU to manipulate
 * \param str terminated string to copy
 * \param len length of the string (not including nul terminator, i.e., output of strlen(3))
 * \see _omlc_set_storage_copy, strlen(3)
 */
#define omlc_set_string_copy(var, str, len)                    \
  do {                                                         \
    _omlc_set_storage_copy((var), string, (str), (len) + 1);   \
    ((char*)(var).stringValue.ptr)[len] = '\0';                \
    omlc_set_string_length(var, len);                          \
    omlc_set_string_is_const(var, 0);                          \
  } while(0)

/** Duplicate an OmlValueU containing a string, allocating storage for an actual copy of the C string.
 *
 * Allocate (or reuse) a buffer of size len+1, copy the string, and
 * nul-terminate it. As the string is actually copied. the destination string
 * is never const, regardless of the source.
 *
 * \param dst destination OmlValueU
 * \param src source OmlValueU
 * \see omlc_set_string_copy
 */
#define omlc_copy_string(dst, src) \
  omlc_set_string_copy((dst), omlc_get_string_ptr(src), omlc_get_string_length(src))

/** Store a pointer to a C string in an OmlValueU's string storage.
 * The string is marked as statically allocated, so subsequent calls so
 * omlc_reset_string will *NOT* free the memory is needed.

 * \param var OmlValueU to operate on
 * \param str C-string pointer to use
 * \see omlc_set_string_size,omlc_reset_string
 */
#define omlc_set_string(var, str)                               \
  do {                                                          \
    omlc_reset_string(var);                                     \
    omlc_set_string_ptr((var), (str));                          \
    omlc_set_string_length((var), ((str)==NULL)?0:strlen(str)); \
  } while (0)

#define omlc_set_datetime(var, str)                               \
do {                                                          \
  omlc_reset_string(var);                                     \
  omlc_set_string_ptr((var), (str));                          \
  omlc_set_string_length((var), ((str)==NULL)?0:strlen(str)); \
} while (0)

/** Store a pointer to a constant C string in an OmlValueU's string storage.
 * \param var OmlValueU to operate on
 * \param str constant C-string pointer to use
 */
#define omlc_set_const_string(var, str)                         \
  do {                                                          \
    omlc_free_string(var);                                      \
    omlc_set_string_ptr((var), (char *)(str));                  \
    omlc_set_string_is_const((var), 1);                         \
    omlc_set_string_length((var), ((str)==NULL)?0:strlen(str)); \
  } while (0)

/** \see _omlc_free_storage */
#define omlc_free_blob(var) \
  _omlc_free_storage((var), blob)
/** \see _omlc_reset_storage */
#define omlc_reset_blob(var) \
  _omlc_reset_storage((var), blob)
/** Convenience alias to omlc_set_blob_copy */
#define omlc_set_blob(var, val, len) \
  omlc_set_blob_copy(var, val, len)
/** \see _omlc_set_storage */
#define omlc_set_blob_copy(var, val, len) \
  _omlc_set_storage_copy((var), blob, (val), (len))

/** Duplicate an OmlValueU containing a blob, allocating storage for an actual copy of the data.
 *
 * Allocate (or reuse) a buffer of size len and copy the data
 *
 * \param dst destination OmlValueU
 * \param src source OmlValueU
 * \see _omlc_set_storage_copy
 */
#define omlc_copy_blob(dst, src) \
  _omlc_set_storage_copy((dst), blob, omlc_get_blob_ptr(src), omlc_get_blob_length(src))

/** \see _oml_get_storage_field */
#define omlc_get_vector_ptr(var) \
  (_oml_get_storage_field((var), vector, ptr))
/** \see _oml_get_storage_field */
#define omlc_get_vector_length(var) \
  (_oml_get_storage_field((var), vector, length))
/** \see _oml_get_storage_field */
#define omlc_get_vector_size(var) \
  (_oml_get_storage_field((var), vector, size))
/** \see _oml_get_storage_field */
#define omlc_get_vector_elt_size(var) \
  (_oml_get_storage_field((var), vector, elt_sz))
/** \see _oml_get_storage_field */
#define omlc_get_vector_nof_elts(var) \
  (_oml_get_storage_field((var), vector, nof_elts))

/** \see _oml_set_storage_field */
#define omlc_set_vector_ptr(var, val)                   \
  _oml_set_storage_field((var), vector, ptr, (void*)(val))
/** \see _oml_set_storage_field */
#define omlc_set_vector_length(var, val) \
  _oml_set_storage_field((var), vector, length, (size_t)(val))
/** \see _oml_set_storage_field */
#define omlc_set_vector_size(var, val) \
  _oml_set_storage_field((var), vector, size, (size_t)(val))
/** \see _oml_set_storage_field */
#define omlc_set_vector_elt_size(var, val) \
  _oml_set_storage_field((var), vector, elt_sz, (size_t)(val))
/** \see _oml_get_storage_field */
#define omlc_set_vector_nof_elts(var, val) \
  (_oml_set_storage_field((var), vector, nof_elts, (uint16_t)(val)))

/** \see _omlc_free_storage */
#define omlc_free_vector(var) \
    _omlc_free_storage((var), vector)                       
/** \see _omlc_reset_storage */
#define omlc_reset_vector(var)                                   \
  do {                                                           \
    _omlc_reset_storage((var), vector);                          \
    omlc_set_vector_elt_size((var), 0);                          \
    omlc_set_vector_nof_elts((var), 0);                          \
  } while(0);

/** Set an vector into the dedicated storage area of an OmlValueU.
 *
 * DO NOT USE THIS MACRO DIRECTLY!
 *
 * It is a helper for specific manipulation macros, which share its behaviour,
 * but have fewer parameters.
 *
 * \param var The OMLValueU to set.
 * \param data Non-null pointer to the base of the vector.
 * \param nof_elts The number of elements in the vector.
 * \param size The size (in bytes) of each vector element.
 */
#define _omlc_set_vector_copy(var, data, nof_elts, size)         \
  do {                                                           \
    size_t bytes = (nof_elts) * (size);                          \
    _omlc_set_storage_copy((var), vector, data, bytes);          \
    omlc_set_vector_nof_elts((var), (nof_elts));                 \
    omlc_set_vector_elt_size((var), (size));                     \
  } while(0)
  
#define omlc_set_vector_double(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(double))
#define omlc_set_vector_long(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(long))
#define omlc_set_vector_int32(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(int32_t))
#define omlc_set_vector_uint32(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(uint32_t))
#define omlc_set_vector_int64(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(int64_t))
#define omlc_set_vector_uint64(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(uint64_t))
#define omlc_set_vector_bool(var, val, nof_elts) \
  _omlc_set_vector_copy((var), (val), (nof_elts), sizeof(bool))

#define omlc_copy_vector(dst, src) \
  _omlc_set_vector_copy((dst), omlc_get_vector_ptr(src), omlc_get_vector_nof_elts(src), omlc_get_vector_elt_size(src))

/** Typed container for an OmlValueU
 *
 * WARNING: OmlValue MUST be oml_value_init()ialised before use and oml_value_reset() after.
 * Not doing so might result in memory problems (double free or memory leak).
 *
 * This takes care of manipulating the contained OmlValueU properly.
 *
 * \see oml_value_init, oml_value_array_init, oml_value_reset,oml_value_array_reset
 */
typedef struct OmlValue {
  /** Type of value */
  OmlValueT type;

  /** Value */
  OmlValueU value;

} OmlValue;

typedef struct OMLSemDef {
    char* subject;
    char* predicate;
    char* object;
    struct OMLSemDef* next;
} OMLSemDef;

/**
 * Definition of one field of an MP.
 *
 * An array of these create a full measurement point.
 * \see omlc_add_mp, OmlMP */
typedef struct OmlMPDef {
  /** Name of the field */
  const char* name;

  /** Type of the field */
  OmlValueT  param_types;

  /** List of semantic concepts */
  OMLSemDef* relations;

} OmlMPDef;

/* Forward declaration, see below */
struct OmlMStream;

/** Definition of a Measurement Point.
 *
 * This structure contains an array of OmlMPDef defining the fields of the MP,
 * and an array of OmlMStream defining which streams need to receive output
 * from this MP.
 *
 * \see omlc_inject, OmlMStream, omlc_add_mp, OmlMP
 */
typedef struct OmlMP {
  /** Name of this MP */
  const char* name;

  /** Array of the fields of this MP */
  OmlMPDef*   param_defs;
  /** Length of the param_defs (i.e., number of fields) */
  int         param_count;

  /** Number of MSs associated to this MP */
  int         table_count;

  /** Linked list of MSs */
  struct OmlMStream* streams;
#define firstStream streams

  /** Set to 1 if this MP is active (i.e., there is at least one MS) */
  int         active;

  /** Mutex for the streams list */
  pthread_mutex_t* mutexP;
  /** Mutex for this storage */
  pthread_mutex_t  mutex;

  /** Next MP in the instance's linked list */
  struct OmlMP*   next;

  /** A pointer to the default MS for that MP,  */
  /* FIXME: to be taken up aftern streams on the next library major bump */
  struct OmlMStream* default_ms;

} OmlMP;

/* Forward declaration from oml_filter.h */
struct OmlFilter;   // can't include oml_filter.h yet
struct OmlWriter;   // forward declaration

/** Definition of a Measurement Stream.
 *
 * A measurement stream links an MP to an output, defined by a writing function
 * (OmlWriter), passing some or all of the fields into a filter (OmlFilter).
 *
 * All the samples injected into an MP are received, but through filtering and
 * aggregation, the output rate of the MS might be different (e.g., 1/n
 * samples, or with a time-based periodicity).
 *
 * \see OmlMP, OmlWriter, OmlFilter, omlc_inject
 */
typedef struct OmlMStream {
  /** Name of this stream (and, usually, the database table it get stored in) */
  char table_name[64];

  /** MP associated to this stream */
  OmlMP* mp;

  /** Current output values */
  OmlValue** values;


  /** Linked list of the filters associated to this MS */
  struct OmlFilter* filters;
#define firstFilter filters

  /** Index of this stream */
  int index;

  /** Number of samples received in the last window */
  int sample_size;
  /** Number of samples to receive before producing an output (if > 1)*/
  int sample_thres;

  /** Interval between periodic reporting [s] */
  double sample_interval;

  /** Output's sequence number (i.e., number of samples produced so far */
  long seq_no;

  /** Condition variable for sample-mode filter (XXX: Never used) */
  pthread_cond_t  condVar;
  /** Filtering thread */
  pthread_t  filter_thread;

  /** Outputting function
   * XXX: This field is deprecated, and replaced with the writers (plural) array below (#1292)*/
  struct OmlWriter* writer __attribute__ ((deprecated));

  /** Next MS in this MP's linked list */
  struct OmlMStream* next;

  /** Output's sequence number for the metadata associated to this stream */
  long meta_seq_no;

  /** Array of OmlWriters through which this MS's tuples should be sent */
  struct OmlWriter** writers;
  /** Length of writers */
  int nwriters;

  /** Number of tuples successfully injected */
  uint32_t written;

  /** Number of tuples dropped */
  uint32_t dropped;

} OmlMStream;

/* Initialise the measurement library. */
int omlc_init(const char *appName, int *argcPtr, const char **argv, o_log_fn oml_log);

/*  Register a measurement point. */
OmlMP *omlc_add_mp(const char *mp_name, OmlMPDef *mp_def);

/* Get ready to start the measurement collection. */
int omlc_start(void);

OMLSemDef *oml_sem_register_concepts(char***concepts, int n);

/*  Inject a measurement sample into a Measurement Point.  */
int omlc_inject(OmlMP *mp, OmlValueU *values);

/** Inject metadata (key/value) for a specific MP.  */
int omlc_inject_metadata(OmlMP *mp, const char *key, const OmlValueU *value, OmlValueT type, const char *fname);

/** Generate a new GUID */
oml_guid_t omlc_guid_generate();

/* DEPRECATED \ee omlc_inject */
void omlc_process(OmlMP* mp, OmlValueU* values);

/**  Terminate all open connections. */
int omlc_close(void);

#ifdef __cplusplus
}
#endif


#endif /*OML_OMLC_H_*/

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/


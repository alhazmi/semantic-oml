/*
 * Copyright 2007-2014 National ICT Australia (NICTA)
 *
 * This software may be used and distributed solely under the terms of
 * the MIT license (License).  You should find a copy of the License in
 * COPYING or at http://opensource.org/licenses/MIT. By downloading or
 * using this software you accept the terms and the liability disclaimer
 * in the License.
 */
/** \file init.c
 * \brief Implement the user-visible initialisation routines of the user-visible OML API.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "oml2/omlc.h"
#include "oml2/oml_filter.h"
#include "oml2/oml_writer.h"
#include "ocomm/o_log.h"
#include "mem.h"
#include "mstring.h"
#include "oml_value.h"
#include "validate.h"
#include "filter/factory.h"
#include "oml_util.h"
#include "client.h"

#define OMLC_COPYRIGHT "Copyright 2007-2014, NICTA"

OmlClient* omlc_instance = NULL;

#define DEF_LOG_LEVEL = O_LOG_INFO;

static OmlMPDef _experiment_metadata[] = {
  {"subject", OML_STRING_VALUE, NULL},
  {"key", OML_STRING_VALUE, NULL},
  {"value", OML_STRING_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};
OmlMP *schema0;

static OmlMPDef _client_instrumentation[] = {
  {"measurements_injected", OML_UINT32_VALUE, NULL},
  {"measurements_dropped", OML_UINT32_VALUE, NULL},
  {"bytes_allocated", OML_UINT64_VALUE, NULL},
  {"bytes_freed", OML_UINT64_VALUE, NULL},
  {"bytes_in_use", OML_UINT64_VALUE, NULL},
  {"bytes_max", OML_UINT64_VALUE, NULL},
  {NULL, (OmlValueT)0, NULL}
};

/** A function pointer suitable for sigaction(3) */
typedef void(*sighandler) (int);

static void usage(void);
static void print_filters(void);
static int  default_configuration(void);
static char *schemastr_from_mpdef(OmlMPDef *mpdef);
static int  write_meta(void);
static int  write_schema(OmlMStream* ms, int index);
static void termination_handler(int signum);
static void install_close_handler(sighandler sig_hdl);
static void setup_features(const char * const features);

static char *default_uri(const char *app_name, const char *name, const char *domain);

extern int parse_config(char* config_file);

/** Initialise the measurement library.
 *
 * This functions parses the command line for --oml-* options and acts
 * accordingly when they are found. A side effect of this function is that
 * these options and their arguments are removed from argv, so the instrumented
 * application doesn't see spurious OML option it can't make sense of.
 *
 * \param name name of the application
 * \param pargc argc of the command line of the application
 * \param argv argv of the command line of the application
 * \param custom_oml_log custom format-based logging function
 * \return 0 on success, -1 on failure, 1 when OML is disabled.
 *
 * \see omlc_add_mp, omlc_start, omlc_close
 * \see o_log_fn, o_set_log
 */
int
omlc_init(const char* application, int* pargc, const char** argv, o_log_fn custom_oml_log)
{
  const char *app_name = validate_app_name (application);
  const char* name = NULL;
  const char* domain = NULL;
  const char* config_file = NULL;
  const char* local_data_file = NULL;
  const char* collection_uri = NULL;
  char *start = NULL, *end = NULL; /* For strtoX(3) */
  enum StreamEncoding default_encoding = SE_None;
  int sample_count = 0;
  double sample_interval = 0.0;
  int max_queue = 0;
  uint32_t instr_interval = 1;
  const char** arg = argv;

  if (!app_name) {
    logerror("Illegal application name '%s': missing or containing whitespaces or dashes\n", application);
    return -1;
  }

  omlc_instance = NULL;

  o_set_log_level(O_LOG_INFO);
  if (custom_oml_log)
    o_set_log (custom_oml_log);

  if (pargc && arg) {
    int i;
    for (i = *pargc; i > 0; i--, arg++) {
      if (strcmp(*arg, "--oml-id") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-id'\n");
          return -1;
        }
        name = *++arg;
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-domain") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-domain'\n");
          return -1;
        }
        domain = *++arg;
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-exp-id") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-exp-id'\n");
          return -1;
        }
        domain = *++arg;
        logwarn("Option --oml-exp-id is getting deprecated; please use '--oml-domain %s' instead\n", domain);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-file") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-file'\n");
          return -1;
        }
        local_data_file = *++arg;
        logwarn("Option --oml-file is getting deprecated; please use '--oml-collect file:%s' instead\n", local_data_file);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-collect") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-collect'\n");
          return -1;
        }
        collection_uri = (char*)*++arg;
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-config") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-config'\n");
          return -1;
        }
        config_file = *++arg;
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-samples") == 0) {
        if (--i <= 0) {
          logerror("Missing argument to '--oml-samples'\n");
          return -1;
        }
        sample_count = atoi(*++arg);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-interval") == 0) {
        if (--i <= 0) {
          logerror("Missing argument to '--oml-interval'\n");
          return -1;
        }
        sample_interval = atof(*++arg);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-log-file") == 0) {
        if (--i <= 0) {
          logerror("Missing argument to '--oml-log-file'\n");
          return -1;
        }
        o_set_log_file((char*)*++arg);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-log-level") == 0) {
        if (--i <= 0) {
          logerror("Missing argument to '--oml-log-level'\n");
          return -1;
        }
        o_set_log_level(atoi(*++arg));
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-server") == 0) {
        if (--i <= 0) {
          logerror("Missing argument for '--oml-server'\n");
          return -1;
        }
        collection_uri = (char*)*++arg;
        logwarn("Option --oml-server is getting deprecated; please use '--oml-collect %s' instead\n", collection_uri);
        *pargc -= 2;
      } else if (strcmp (*arg, "--oml-text") == 0) {
        *pargc -= 1;
        default_encoding = SE_Text;
      } else if (strcmp (*arg, "--oml-binary") == 0) {
        *pargc -= 1;
        default_encoding = SE_Binary;
      } else if (strcmp(*arg, "--oml-bufsize") == 0) {
        if (--i <= 0) {
          logerror("Missing argument to '--oml-bufsize'\n");
          return -1;
        }
        max_queue = atoi(*++arg);
        *pargc -= 2;
      } else if (strcmp(*arg, "--oml-instr-interval") == 0) {
        start = (char *)*++arg; /* XXX: Drop arg's const */
        end = NULL;
        if (--i <= 0) {
          logerror("Missing argument to '--oml-instr-interval'\n");
          return -1;
        }
        instr_interval = strtoul(start, &end, 10);
        if(end == *arg || *end != '\0') {
          logwarn("Invalid argument to '--oml-instr-interval'\n");
          instr_interval = 0;
        }
        *pargc -= 2;
        if(0 == instr_interval) {
          loginfo("Client instrumentation disabled\n");
        }

      } else if (strcmp(*arg, "--oml-noop") == 0) {
        *pargc -= 1;
        omlc_close();
        return 1;
      } else if (strcmp(*arg, "--oml-help") == 0) {
        usage();
        *pargc -= 1;
        exit(0);
      } else if (strcmp(*arg, "--oml-list-filters") == 0) {
        print_filters();
        *pargc -= 1;
        exit(0);
      } else {
        *argv++ = *arg;
      }
    }
  }

  if (name == NULL) {
    name = getenv("OML_NAME");
  }
  if (domain == NULL) {
    if(!(domain = getenv("OML_DOMAIN"))) {
      if ((domain= getenv("OML_EXP_ID"))) {
        logwarn("Enviromnent variable OML_EXP_ID is getting deprecated; please use 'OML_DOMAIN=\"%s\"' instead\n",
            domain);
      }
    }
  }
  if (config_file == NULL) {
    config_file = getenv("OML_CONFIG");
  }
  if (local_data_file == NULL && collection_uri == NULL) {
    if(!(collection_uri = getenv("OML_COLLECT"))) {
      if ((collection_uri = getenv("OML_SERVER"))) {
        logwarn("Enviromnent variable OML_SERVER is getting deprecated; please use 'OML_COLLECT=\"%s\"' instead\n",
            collection_uri);
      }
    }
  }

  if(collection_uri == NULL) {
    collection_uri = default_uri(app_name, name, domain);
  }

  setup_features (getenv ("OML_FEATURES"));

  omlc_instance = oml_malloc(sizeof(OmlClient));
  memset(omlc_instance, 0, sizeof(OmlClient));

  omlc_instance->app_name = app_name;
  omlc_instance->node_name = name;
  omlc_instance->domain = domain;
  omlc_instance->sample_count = sample_count;
  omlc_instance->sample_interval = sample_interval;
  omlc_instance->default_encoding = default_encoding;
  omlc_instance->max_queue = max_queue;
  omlc_instance->instr_time = 0;
  omlc_instance->instr_interval = instr_interval;

  if (local_data_file != NULL) {
    // dump every sample into local_data_file
    if (local_data_file[0] == '-')
      local_data_file = "stdout";
    snprintf(omlc_instance->collection_uri, COLLECTION_URI_MAX_LENGTH, "file:%s", local_data_file);
  } else if (collection_uri != NULL) {
    strncpy(omlc_instance->collection_uri, collection_uri, COLLECTION_URI_MAX_LENGTH);
  }
  omlc_instance->config_file = config_file;

  register_builtin_filters ();

  schema0 = omlc_add_mp("_experiment_metadata", _experiment_metadata);

  omlc_instance->client_instr = omlc_add_mp("_client_instrumentation", _client_instrumentation);

  loginfo ("OML Client %s [OMSPv%d] %s\n",
           VERSION,
           OML_PROTOCOL_VERSION,
           OMLC_COPYRIGHT);

  return 0;
}

OMLSemDef *
destroy_sem_concepts(OMLSemDef *sd)
{
    OMLSemDef *sd_aux = NULL;
    if (sd)
    {
        if (sd->subject) oml_free(sd->subject);
        if (sd->predicate) oml_free(sd->predicate);
        if (sd->object) oml_free(sd->object);
        sd_aux = sd->next;
        oml_free(sd);
    }
    return sd_aux;
}

/** Register a measurement point.
 *
 * This function should be called after omlc_init() and before omlc_start().
 * It can be called multiple times, once for each measurement point that the
 * application needs to define.
 *
 * The returned OmlMP must be the first argument in every omlc_inject() call
 * for this specific measurement point.
 *
 * XXX: The supplied name is not checked for existence.
 *
 * The MP's input structure is defined by the mp_def parameter, it should be
 * initialised as this example shows.
 * \code {.c}
 *   OmlMPDef mp_def[] =
 *   {
 *     { "source", OML_UINT32_VALUE, NULL},
 *     { "destination", OML_UINT32_VALUE, NULL},
 *     { "length", OML_UINT32_VALUE, NULL},
 *     { "weight", OML_DOUBLE_VALUE, NULL},
 *     { "protocol", OML_STRING_VALUE, NULL},
 *     { NULL, (OmlValueT)0, NULL }
 *   };
 * \endcode
 *
 * \param mp_name The name of this MP.  The name must not contain whitespace
 * \param mp_def  Definition of this MP's input tuple, as an array of OmlMPDef objects
 * \return a new measurement point \see omlc_init, omlc_start
 */
OmlMP*
omlc_add_mp (const char* mp_name, OmlMPDef* mp_def)
{
  int pc = 0;
  char *schemastr;
  MString *meta;
  OmlValueU v;

  omlc_zero(v);

  if (omlc_instance == NULL) { return NULL; }
  if (!validate_name (mp_name)) {
    logerror("Illegal MP name '%s': missing or containing whitespaces or dashes; MP will not be created\n", mp_name);
    return NULL;
  }

  logdebug("Adding MP %s\n", mp_name);

  OmlMP* mp = (OmlMP*)oml_malloc(sizeof(OmlMP));
  memset(mp, 0, sizeof(OmlMP));

  mp->name = mp_name;
  mp->param_defs = mp_def;
  OmlMPDef* dp = mp_def;
  while (dp != NULL && dp->name != NULL) {
    if (dp->param_types == OML_LONG_VALUE) {
      logwarn("MP '%s', field '%s': OML_LONG_VALUE is deprecated, please use OML_INT32_VALUE instead; "
          "values outside of [INT_MIN, INT_MAX] will be clamped\n",
          mp->name, dp->name);
    }

    if (!validate_name (dp->name)) {
      logerror("Illegal field name '%s' in MP %s: missing or containing whitespaces or dashes; MP will not be created\n",
          dp->name, mp_name);
      while (destroy_sem_concepts(mp->param_defs->relations));
      oml_free (mp);
      return NULL;
    }

    pc++;
    dp++;
  }
  mp->param_count = pc;
  mp->active = 1;  // True if there is an attached MS.

  if(omlc_instance->start_time > 0) {
    /* omlc_start has already been called, declare MP through schema0 */
    meta = mstring_create();
    schemastr = schemastr_from_mpdef(mp_def);

    /* Unlike how we manage schema0, we do prepend the application name to the
     * MP name, even when declared after the start. This is mostly because that
     * concatenation happens anyway on the client side, when creating the
     * default filter configuration, and we don't want too many discrepancies
     * between the client and servers views at this stage. See #1055. */
    mstring_sprintf(meta, "%d %s_%s%s",
        omlc_instance->next_ms_idx, omlc_instance->app_name, mp_name, schemastr);
    oml_free(schemastr);

    logdebug("omlc_start already called, adding MP through schema 0: %s\n",
        mstring_buf(meta));

    omlc_set_string(v, mstring_buf(meta));
    omlc_inject_metadata(NULL, "schema", &v, OML_STRING_VALUE, NULL);
    omlc_reset_string(v);

    mstring_delete(meta);

    if (!oml_mp_get_default_ms(mp)) {
      logerror("Failed to create default MS for MP %s\n", mp_name);
      oml_free(mp);
      return NULL;
    }

    /* At this stage, we only have one stream set up, and we now its index */
    mp->streams->index = omlc_instance->next_ms_idx++;

  }

  if (NULL == omlc_instance->mpoints) {
    omlc_instance->mpoints = mp;
  } else {
    omlc_instance->last_mpoint->next = mp;
  }
  omlc_instance->last_mpoint = mp;

  return mp;
}

/** Asign all concepts to the measurement Point
 * 
 * \param concepts matrix of concepts
 * \param n number of triples
 * \return OMLSemDef* (can be NULL)
 * \author Mario Poyato Pino <mario.poyato@uam.es>
 */
OMLSemDef *
oml_sem_register_concepts(char***concepts, int n)
{
  int i=0;
  OMLSemDef *sd=NULL,*sd_last,*sd_aux;
  if (n>0) {
    sd = sd_last = sd_aux = (OMLSemDef*)oml_malloc(sizeof(OMLSemDef));
    while (i<n) {
      sd_aux->subject   = xstrdup(concepts[i][0]);
      sd_aux->predicate      = xstrdup(concepts[i][1]);
      sd_aux->object = xstrdup(concepts[i][2]);
      sd_aux->next      = NULL;
      if (++i<n) {
        sd_aux = (OMLSemDef*)oml_malloc(sizeof(OMLSemDef));
        sd_last->next = sd_aux;
        sd_last=sd_aux;
      }
    }
  }
  return sd;
}

/** Destroy MP
 *
 * This function is designed so it can be used in a while loop to clean up the
 * entire linked list:
 *
 *   while( (mp=destroy_mp(mp)) );
 *
 * \param mp pointer to the OmlMP to free
 * \return mp->next (can be NULL)
 */
OmlMP*
destroy_mp(OmlMP *mp)
{
  OmlMP *next;
  OmlMStream *ms;

  if(!mp) {
    return NULL;
  }

  logdebug("Destroying MP %s at %p\n", mp->name, mp);

  next = mp->next;

  if (!mp_lock(mp)) {
    mp->active = 0;
    ms = mp->streams;
    while (destroy_sem_concepts(mp->param_defs->relations));
    while( (ms=destroy_ms(ms)) );
    mp_unlock(mp);
  }

  return next;
}

/** Get ready to start the measurement collection.
 *
 *  This function must be called after omlc_init and after any calls to
 *  omlc_add_mp().  It finalises the initialisation process and initialises
 *  filters on all measurement points, according to the current configuration
 *  (based on either command line options or the XML config file named by the
 *  --oml-config command line option).
 *
 *  It also registers a termination handler.
 *
 *  Once this function has been called, and if it succeeds, the application is
 *  free to start creating measurement samples by calling omlc_inject().
 *
 *  If this function fails, subsequent calls to omlc_inject() will result in
 *  undefined behaviour.
 *
 *  \return 0 on success, -1 on failure
 *
 *  \see omlc_init, omlc_add_mp, omlc_close
 *  \see install_close_handler, termination_handler
 */
int
omlc_start()
{
  if (omlc_instance == NULL) { return -1; }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  omlc_instance->start_time = tv.tv_sec;

  const char* config_file = omlc_instance->config_file;
  if (config_file) {
    if (parse_config((char*)config_file)) {
      logerror("Error while parsing configuration '%s'\n", config_file);
      omlc_close();
      return -1;
    }
  } else {
    if (omlc_instance->collection_uri == NULL) {
      logerror("Missing --oml-collect declaration.\n");
      omlc_close();
      return -2;
    }
    if (default_configuration()) {
      omlc_close();
      return -3;
    }
  }
  install_close_handler(termination_handler);
  if (write_meta() == -1) {
    return -1;
  }
  return 0;
}

/** Terminate data collection on signals
 * \param signum the signal number
 * \see install_close_handler
 */
static void
termination_handler(int signum)
{
  // SIGPIPE is handled by disabling the writer that caused it.
  if (signum != SIGPIPE) {
    logdebug("Closing OML (%d)\n", signum);
    omlc_close();
    exit(-1 * signum);
  }
}

/** Register a signal handler on SIGINT, SIGHUP, SIGTERM, and SIGPIPE
 *
 * \param sig_hdl sighandler to register
 *
 * Use SIG_DFL as sig_hdl to restore the default behaviour.
 *
 * \see omlc_start, termination_handler */
static void
install_close_handler(sighandler sig_hdl)
{
  struct sigaction new_action, old_action;

  /* Set up the structure to specify the new action. */
  new_action.sa_handler = sig_hdl;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN) {
    sigaction (SIGINT, &new_action, NULL);
  }

  sigaction (SIGHUP, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN) {
    sigaction (SIGHUP, &new_action, NULL);
  }

  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN) {
    sigaction (SIGTERM, &new_action, NULL);
  }

  sigaction (SIGPIPE, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN) {
    sigaction (SIGPIPE, &new_action, NULL);
  }
}

/** Terminate all open connections.
 *
 * Once this function has been called, any futher calls to omlc_inject() will
 * be ignored.
 *
 * This call doesn't free all memory used by OML immediately. There may be a
 * few threads which will take some time to finish while the remaining
 * buffered data is sent.
 *
 * \return 0 on sucess, -1 otherwise
 */
int
omlc_close(void)
{

  if (NULL == omlc_instance) {
    return 0; /* Already clear */

  } else {
    OmlWriter* w = omlc_instance->first_writer;
    OmlMP* mp = omlc_instance->mpoints;

    install_close_handler(SIG_DFL);
    while( (mp = destroy_mp(mp)) );
    if (w) {
      while( (w =  w->close(w)) );
    }

    unregister_filters ();
    oml_free(omlc_instance);
  }

  omlc_instance = NULL;

  oml_memreport(O_LOG_DEBUG);
  return 0;
}

/** Print the possible OML command line parameters */
static void
usage(void)

{
  printf("OML Client %s [OMSPv%d]\n",
           VERSION,
           OML_PROTOCOL_VERSION);
  printf("%s\n", OMLC_COPYRIGHT);
  printf("\n");
  printf("OML specific parameters:\n\n");
  printf("  --oml-id id            .. Name to identify this app instance\n");
  printf("  --oml-domain domain    .. Name of experimental domain\n");
  printf("  --oml-collect uri      .. URI of server to send measurements to\n");
  printf("  --oml-config file      .. Reads configuration from 'file'\n");
  printf("  --oml-samples count    .. Default number of samples to collect\n");
  printf("  --oml-interval seconds .. Default interval between measurements\n");
  printf("  --oml-text             .. Use text encoding for all output streams\n");
  printf("  --oml-binary           .. Use binary encoding for all output streams\n");
  printf("  --oml-bufsize size     .. Set size of internal buffers to 'size' bytes\n");
  printf("  --oml-log-file file    .. Writes log messages to 'file'\n");
  printf("  --oml-log-level level  .. Log level used (error: -2 .. info: 0 .. debug4: 4)\n");
  printf("  --oml-noop             .. Do not collect measurements\n");
  printf("  --oml-list-filters     .. List the available types of filters\n");
  printf("  --oml-help             .. Print this message\n");
  printf("\n");
  printf("Valid URI: [tcp:]host[:service], (file|flush):localPath\n");
  printf("\n");
  printf("The following environment variables are recognized:\n");
  printf("  OML_NAME=id            .. Name to identify this app instance (--oml-id)\n");
  printf("  OML_DOMAIN=domain      .. ame of experimental domain (--oml-domain)\n");
  printf("  OML_CONFIG=file        .. Read configuration from 'file' (--oml-config)\n");
  printf("  OML_COLLECT=uri        .. URI of server to send measurements to (--oml-collect)\n");
  printf("\n");
  printf("Obsolescent interfaces:\n\n");
  printf("  --oml-exp-id domain    .. Equivalent to --oml-domain domain\n");
  printf("  --oml-file localPath   .. Equivalent to --oml-collect file:localPath\n");
  printf("  --oml-server uri       .. Equivalent to --oml-collect uri\n");
  printf("  OML_EXP_ID=domain      .. Equivalent to OML_DOMAIN\n");
  printf("  OML_SERVER=uri         .. Equivalent to OML_COLLECT\n");
  printf("\n");
  printf("If the corresponding command line option is present, it overrides\n");
  printf("the environment variable.\n");
  printf("\n");
}

static void
print_filters(void)
{
  register_builtin_filters();

  printf("OML Client V%s\n", VERSION);
  printf("OML Protocol V%d\n", OML_PROTOCOL_VERSION);
  printf("%s\n", OMLC_COPYRIGHT);
  printf("\n");
  printf("OML filters available:\n\n");

  const char* filter = NULL;
  do
    {
      filter = next_filter_name();
      if (filter)
        printf ("\t%s\n", filter);
    }
  while (filter != NULL);
  printf ("\n");
}

/** Create either a file writer or a network writer
 * \param uri collection URI
 * \param encoding StreamEncoding to use for the output, either SE_Text or SE_Binary
 *
 * \return a pointer to the new OmlWriter, or NULL on error
 */
OmlWriter*
create_writer(const char* uri, enum StreamEncoding encoding)
{
  OmlURIType uri_type = oml_uri_type(uri);

  if (omlc_instance == NULL){
    logerror("No omlc_instance:  OML client was not initialized properly.\n");
    return NULL;
  }

  if (uri == NULL || strlen(uri) < 1) {
    logerror ("Missing or invalid collection URI definition (e.g., --oml-collect)\n");
    return NULL;
  }
  if (omlc_instance->node_name == NULL) {
    logerror ("Missing '--oml-id' flag \n");
    return NULL;
  }
  if (omlc_instance->domain == NULL) {
    logerror ("Missing '--oml-domain' flag \n");
    return NULL;
  }

  const char *transport = NULL;
  const char *path = NULL;
  const char *port = NULL;

  if (parse_uri (uri, &transport, &path, &port) == -1) {
    logerror ("Error parsing server destination URI '%s'; failed to create stream for this destination\n",
              uri);
    if (transport) oml_free ((void*)transport);
    if (path) oml_free ((void*)path);
    if (port) oml_free ((void*)port);
    return NULL;
  }

  const char *filepath = NULL;
  const char *hostname = NULL;
  if (oml_uri_is_file(uri_type)) {
    /* 'file://path/to/file' is equivalent to unix path '/path/to/file' */
    if (strncmp (path, "//", 2) == 0)
      filepath = &path[1];
    else
      filepath = path;
  } else if (transport) {
    if (strncmp (path, "//", 2) == 0)
      hostname = &path[2];
    else
      hostname = path;
  } else {
    hostname = path; /* If no transport specified, it must be tcp */
  }

  /* Default transport is tcp if not specified */
  if (!transport) {
    transport = oml_strndup ("tcp", strlen ("tcp"));
  }

  /* If not file transport, use the OML default port if unspecified */
  if (!port && !oml_uri_is_file(uri_type)) {
    port = oml_strndup (DEF_PORT_STRING, strlen (DEF_PORT_STRING));
  }

  OmlOutStream* out_stream;
  if (oml_uri_is_file(uri_type)) {
    out_stream = file_stream_new(filepath);
    if (encoding == SE_None) encoding = SE_Text; /* default encoding */
    if(OML_URI_FILE_FLUSH == uri_type) {
      file_stream_set_buffered(out_stream, 0);
    }
  } else {
    out_stream = net_stream_new(transport, hostname, port);
    if (encoding == SE_None) encoding = SE_Binary; /* default encoding */
  }
  if (out_stream == NULL) {
    logerror ("Failed to create stream for URI %s\n", uri);
    return NULL;
  }

  oml_free ((void*)transport);
  oml_free ((void*)path);
  oml_free ((void*)port);

  // Now create a write on top of the stream
  OmlWriter* writer = NULL;

  switch (encoding) {
  case SE_Text:   writer = text_writer_new (out_stream); break;
  case SE_Binary: writer = bin_writer_new (out_stream); break;
  case SE_None:
    logerror ("No encoding specified (this should never happen -- please report this as an OML bug)\n");
    // should cleanup streams
    return NULL;
  }
  if (writer == NULL) {
    logerror ("Failed to create writer for encoding '%s'.\n", encoding == SE_Binary ? "binary" : "text");
    return NULL;
  }
  writer->next = omlc_instance->first_writer;
  omlc_instance->first_writer = writer;

  return writer;
}

/** Find a named measurement point.
 *
 * \param name The name of the measurement point to search for.
 * \return the measurement point, or NULL if not found.
 */
OmlMP*
find_mp (const char *name)
{
  OmlMP *mp = omlc_instance->mpoints;
  while (mp && strcmp (name, mp->name))
    mp = mp->next;
  return mp;
}

/** \Find a named field of an MP.
 * \param name name of the field to look up
 * \param mp OmlMP to look for the field
 *
 * If either parameter is NULL, act as if the field wasn't found.
 *
 * \return a valid index into the OmlMP's param_defs array, or -1 if not found
 */
int
find_mp_field (const char *name, OmlMP *mp)
{
  size_t i;
  if (NULL == name || NULL == mp) { return -1; }
  size_t len = mp->param_count;
  OmlMPDef *fields = mp->param_defs;
  for (i = 0; i < len; i++) {
    logdebug("Searching MP %s for field '%s', found '%s'\n",
        mp->name, name, fields[i].name);
    if (strcmp (name, fields[i].name) == 0) {
      return i;
    }
  }
  return -1;
}

/** Create an MString containing a comma-separated list of the fields of the MP.
 *
 * \param mp OmlMP to summarize
 * \return the fields summary for mp.  The caller is repsonsible for calling mstring_delete() to deallocate the returned MString.
 */
MString*
mp_fields_summary (OmlMP *mp)
{
  int i;
  OmlMPDef *fields = mp->param_defs;
  MString *s = mstring_create();
  mstring_set (s, "'");

  if (!s) return NULL;

  for (i = 0; i < mp->param_count; i++) {
    mstring_cat (s, fields[i].name);
    if (i < mp->param_count - 1)
      mstring_cat (s, "', '");
  }
  mstring_cat (s, "'");

  return s;
}

/** Find a named MStream among the streams attached to an MP.
 *
 * \param name name of the stream to search for
 * \param mp MP in which to search for the stream
 * \return a pointer to the OmlMStream if found, or NULL if not (or name is NULL)
 */
OmlMStream*
find_mstream_in_mp (const char *name, OmlMP *mp)
{
  OmlMStream *ms = mp->streams;

  if (!name)
    return NULL;

  while (ms) {
    if (ms && ms->table_name && strcmp (name, ms->table_name) == 0)
      break;
    ms = ms->next;
  }

  return ms;
}

/** Find a measurement stream by name.
 * All measurement streams must be named uniquely.
 *
 * \param name the name of the measurement stream to search for.
 * \return a pointer to the measurement stream or NULL if not found.
 */
OmlMStream*
find_mstream (const char *name)
{
  OmlMP *mp = omlc_instance->mpoints;
  OmlMStream *ms = NULL;
  while (mp) {
    ms = find_mstream_in_mp (name, mp);
    if (ms)
      break;
    mp = mp->next;
  }
  return ms;
}

/** Create a new stream of measurement samples from the inputs
 * to a given MP.
 *
 * If a stream by that name already exists, it is used instead.
 *
 * \param sample_interval the sample interval for the filter
 * \param sample_thres the threshold for the filter
 * \param mp the measurement point
 * \param writer the destination for the stream output samples
 * \return the new measurement stream, or NULL if an error occurred.
 */
OmlMStream*
create_mstream (const char *name, OmlMP* mp, OmlWriter* writer, double sample_interval, int sample_thres)
{
  char *stream_name = NULL;
  int new = 1;
  MString *namestr;
  OmlMStream *ms;
  if (!mp) {
    return NULL;
  }

  namestr = mstring_create();
  /* If name is not specified, create one by concatenating the application and
   * MP names.
   *
   * XXX: We probably should not do it for any MP, as an MP should be
   * application-agnostic. This is not the case at the moment, and we don't
   * want to confuse legacy post-processing scripts. See #1055.
   *
   * However, schema 0 is new, so let's do the right thing here.
   *
   */
  namestr = mstring_create();
  if ((mp != schema0) && (mp != omlc_instance->client_instr)) {
    mstring_set (namestr, omlc_instance->app_name);
    mstring_cat (namestr, "_");
  }
  if (name) {
    mstring_cat (namestr, name);
  } else {
    mstring_cat (namestr, mp->name);
  }
  stream_name = mstring_buf (namestr);

  if ((ms=find_mstream (stream_name))) {
    loginfo("MS '%s' already exists, assuming identical parameters for a new destination; use the 'name' attribute of the <stream /> element to change\n", stream_name);
    new = 0;

  } else {
    logdebug("MS '%s' does not exist, creating...\n", stream_name);
    ms = (OmlMStream*)oml_malloc(sizeof(OmlMStream));
    if(!ms) {
      logerror("Cannot allocate memory for MS %s\n", name);
      return NULL;
    }
    memset(ms, 0, sizeof(OmlMStream));

    ;
    strncpy (ms->table_name, stream_name, sizeof(ms->table_name));
    ms->table_name[sizeof(ms->table_name)-1] = '\0';

    ms->sample_interval = sample_interval;
    ms->sample_thres = sample_thres;
    ms->mp = mp;

    if (ms->sample_interval > 0) {
      if (mp->mutexP == NULL) {
        mp->mutexP = &mp->mutex;
        pthread_mutex_init(mp->mutexP, NULL);
      }
      ms->sample_thres = 0;
    }
  }
  mstring_delete (namestr);

  if(!writer) {
    logdebug("NULL writer for MS %s; it might get added later (using a configuration file?)\n", name?name:mp->name);

  } else if (oml_ms_add_writer(ms, writer))  {
    logerror("Cannot add %s's writer\n", name);
    if(new) {
      oml_free(ms);
    }
    return NULL;
  }

  if (new) {
    ms->next = mp->streams;
    mp->streams = ms;
  }
  return ms;
}

/** Add a new writer to an existing MS
 * \param ms MStream to add w to
 * \param w writer to add to MS
 *
 * \return 0 on success, -1 on error (previous writers are preserved)
 */
int
oml_ms_add_writer(OmlMStream *ms, OmlWriter *w)
{
  int n;
  OmlWriter **writers;

  if(!ms || !w) {
    logerror("%s: ms (%p) or w (%p) are invalid", __FUNCTION__, ms, w);
    return -1;
  }

  n = ms->nwriters + 1;

  writers = (OmlWriter **)oml_realloc(ms->writers, n*sizeof(OmlWriter*));
  if (!writers) {
    logerror("Cannot (re)allocate memory for %s's writers array\n", ms->table_name);
    return -1;
  }

  writers[n-1] = w;
  ms->writers = writers;
  ms->nwriters = n;

  return 0;
}

/** Destroy a Measurement Stream, and deep free allocated memory (filters.
 *
 * This function is designed so it can be used in a while loop to clean up the
 * entire linked list:
 *
 *   while( (ms=destroy_ms(ms)) );
 *
 * \param ms pointer to the OmlMStream to free
 * \return ms->next (can be NULL)
 */
OmlMStream*
destroy_ms(OmlMStream *ms)
{
  OmlMStream *next;
  OmlFilter *ft;

  if(!ms) {
    return NULL;
  }

  logdebug("Destroying MS %s at %p\n", ms->table_name, ms);

  next = ms->next;

  if (ms->sample_size > 0) {
    loginfo("Reporting last (partial) sample for MS %s\n", ms->table_name);
    filter_process(ms);
  }
  ft = ms->filters;

  while( (ft = destroy_filter(ft)) );

  oml_free(ms->writers);
  oml_free(ms);

  return next;
}

/** Loop through registered measurment points and define sample based filters with sampling rate '1' and 'FIRST' filters
 *
 * \return 0 if successful, -1 otherwise
 */
static int
default_configuration(void)
{
  OmlMP *mp;

  if (NULL == omlc_instance->default_writer) {
    if ((omlc_instance->default_writer =
          create_writer(omlc_instance->collection_uri,
            omlc_instance->default_encoding)
        ) == NULL) {
      return -1;
    }
  }

  if (0 == omlc_instance->sample_count) {
    omlc_instance->sample_count = 1;
  }

  mp = omlc_instance->mpoints;
  while (mp != NULL) {
    oml_mp_get_default_ms(mp);
    mp = mp->next;
  }
  return 0;
}

/** Get or create a default MS for the given MP, reporting all fields, using
 * the OML instance's default samples and intervals and writing to its default
 * writer.
 *
 * \param mp OmlMP for which to set the filter configuration
 * \return the OmlMstream if successful, NULL otherwise
 */
OmlMStream *
oml_mp_get_default_ms(OmlMP *mp)
{
  if(NULL == omlc_instance || NULL == mp) {
    return NULL;
  }

  logdebug("Getting default MS for MP %s\n", mp->name);
  if (!mp->default_ms) {
    if(!mp->streams) {
      create_mstream (NULL, mp,
          omlc_instance->default_writer,
          omlc_instance->sample_interval,
          omlc_instance->sample_count );
      if (NULL == mp->streams) {
        logerror("Error creating MS %s\n", mp->name);
        return NULL;
      }

      create_default_filters(mp, mp->streams);
      if (omlc_instance->sample_interval > 0) {
        filter_engine_start(mp->streams);
      }
    }
    mp->default_ms = mp->streams;

  }
  return mp->default_ms;
}

/** Create the default filters
 * \param mp OmlMP to add filters to
 * \param ns OmlMStream associated to mp
 */
void
create_default_filters(OmlMP *mp, OmlMStream *ms)
{
  int j;

  if(!mp || !ms) {
    return;
  }

  int param_count = mp->param_count;

  logdebug("Creating default filters on MP '%s' to MS '%s'\n",
      mp->name, ms->table_name);

  OmlFilter* prev = NULL;
  for (j = 0; j < param_count; j++) {
    OmlMPDef def = mp->param_defs[j];
    OmlFilter* f = create_default_filter(&def, ms, j);
    if (f) {
      if (prev == NULL) {
        ms->firstFilter = f;
      } else {
        prev->next = f;
      }
      prev = f;
    } else {
      logerror("Unable to create default filter for MP %s.\n", mp->name);
    }
  }
}

/** Create a new filter for the measurement associated with the stream
 * \param def OmlMPDef of the MP for which to create a filter
 * \param ms OmlMStream in which the filter outputs its samples
 * \param index index of the field to filter within this MP
 * \return a new filter
 */
OmlFilter*
create_default_filter(OmlMPDef *def, OmlMStream *ms, int index)
{
  const char* name = def->name;
  OmlValueT type = def->param_types;
  OMLSemDef * concepts = def->relations;
  int multiple_samples = ms->sample_thres > 1 || ms->sample_interval > 0;

  char* fname;
  if (multiple_samples && omlc_is_numeric_type (type)) {
    fname = "avg";
  } else {
    fname = "first";
  }
  OmlFilter* f = create_filter(fname, name, type, concepts, index);
  return f;
}

/** Generate the schema string describing an OMLSemDef
 * 
 * \param mpdef OmlMPDef to extract concepts
 * \return 
 */
static char*
sem_from_mpdef(OMLSemDef *dp)
{
  char *schema_str = NULL;
  MString *schema_mstr;
  if (dp) {
    schema_mstr = mstring_create();
    while (dp != NULL) {
      mstring_sprintf(schema_mstr, "{%s|%s|%s}", dp->subject,dp->predicate,dp->object);
      dp=dp->next;
    }
    schema_str = xstrdup(mstring_buf(schema_mstr));
    mstring_delete(schema_mstr);
  }
  return schema_str;
}

/** Generate the schema string describing an OmlMPDef
 *
 *  The generated schema starts with a space, so it can be directly
 *  concatenated to, e.g., the MS name.
 *
 * \param mpdef OmlMPDef to describe
 * \return an oml_malloc'd string representation of the schema of OmlMPDef, to be oml_free'd by the caller, or NULL on error
 * \see oml_malloc, oml_free
 */
static char*
schemastr_from_mpdef(OmlMPDef *mpdef)
{
  OMLSemDef*sp;
  char *schema_str;
  MString *schema_mstr;
  OmlMPDef *dp = mpdef;
  if (!mpdef && mpdef->name !=NULL) {
    return NULL;
  }

  schema_mstr = mstring_create();

  while (dp != NULL && dp->name != NULL) {
    if (dp->relations)
    {
        sp=sem_from_mpdef(dp->relations);
        if (sp)
        {
            mstring_sprintf(schema_mstr, " %s:%s:%s", dp->name,
                oml_type_to_s(dp->param_types),sp);
            oml_free(sp);
        }
        else
        {
            mstring_sprintf(schema_mstr, " %s:%s", dp->name,
                oml_type_to_s(dp->param_types));
        }
    }
    else
    {
        mstring_sprintf(schema_mstr, " %s:%s:", dp->name,
            oml_type_to_s(dp->param_types));
    }
    dp++;
  }

  schema_str = xstrdup(mstring_buf(schema_mstr)); /* XXX perhaps a bit too many memory allocations here... */
  mstring_delete(schema_mstr);
  return schema_str;
}

/** Output the headers on all streams
 *
 * The OmlWriter associated with each stream is in charge of remembering them,
 * and resending them in case of a disconnection
 *
 * \return 0 if successful
 */
static int
write_meta(void)
{
  OmlWriter* writer = omlc_instance->first_writer;
  for (; writer != NULL; writer = writer->next) {
    char s[128];
    sprintf(s, "protocol: %d", OML_PROTOCOL_VERSION);
    writer->meta(writer, s);
    sprintf(s, "domain: %s", omlc_instance->domain);
    writer->meta(writer, s);
    sprintf(s, "start-time: %d", (int)omlc_instance->start_time);
    writer->meta(writer, s);
    sprintf(s, "sender-id: %s", omlc_instance->node_name);
    writer->meta(writer, s);
    sprintf(s, "app-name: %s", omlc_instance->app_name);
    writer->meta(writer, s);
  }

  OmlMP* mp = omlc_instance->mpoints;
  int index = 0; /* Schema 0 is, well schema0 */
  while (mp != NULL) {
    OmlMStream* ms = mp->streams;
    for(; ms != NULL; ms = ms->next) {
      write_schema(ms, index++);
    }
    mp = mp->next;
  }
  omlc_instance->next_ms_idx = index;

  writer = omlc_instance->first_writer;
  for (; writer != NULL; writer = writer->next) {
    writer->header_done(writer);   // end of header
  }
  return 0;
}


/** Write the different schemas of the application
 *
 * \param ms the stream definition
 * \param index the index in the measurement points, used to initialias OmlMStream::index
 *
 * \return 0 if successful
 */
#define DEFAULT_SCHEMA_LENGTH 2048
static int
write_schema(OmlMStream *ms, int index)
{
  char s[DEFAULT_SCHEMA_LENGTH];
  const size_t bufsize = sizeof (s) / sizeof (s[0]);
  size_t schema_size = DEFAULT_SCHEMA_LENGTH;
  char* schema;
  size_t count = 0;
  size_t n = 0;
  int i;
  OMLSemDef* concepts;

  ms->index = index;
  n = snprintf(s, bufsize, "schema: %d %s ", ms->index, ms->table_name);

  if (n >= bufsize) {
    logerror("Schema generation failed because the following table name was too long: %s\n", ms->table_name);
    return -1;
  }
  schema = (char*)oml_malloc (schema_size * sizeof (char));

  strncpy (schema, s, n + 1);
  count += n;

  // Loop over all the filters
  OmlFilter* filter = ms->filters;
  for (; filter != NULL; filter = filter->next) {
    const char* prefix = filter->name;
    int j;
    for (j = 0; j < filter->output_count; j++) {
      char* name;
      OmlValueT type;
      if (filter->meta(filter, j, &name, &type, &concepts) != -1) {
        const char *type_s = oml_type_to_s(type);
        char *c = sem_from_mpdef(concepts);
        if (c)
        {
          if (name == NULL)
            n = snprintf(s, bufsize, "%s:%s:%s ", prefix, type_s, c);
          else
            n = snprintf(s, bufsize, "%s_%s:%s:%s ", prefix, name, type_s, c);
          oml_free(c);
        }
        else
        {
            if (name == NULL)
              n = snprintf(s, bufsize, "%s:%s ", prefix, type_s);
            else
              n = snprintf(s, bufsize, "%s_%s:%s ", prefix, name, type_s);
        }

        if (n >= bufsize) {
            logerror("One of the schema entries for table %s was too long:\n\t%s\t%s\n",
                   prefix, type_s);
            oml_free (schema);
            return -1;
        }

        if (count + n >= schema_size) {
            schema_size += DEFAULT_SCHEMA_LENGTH;
            char* new = (char*)oml_malloc (schema_size * sizeof (char));
            strncpy (new, schema, count);
            oml_free (schema);
            schema = new;
        }
        strncpy (&schema[count], s, n + 1);
        count += n;
      } else {
        logwarn("Filter %s failed to provide meta information for index %d.\n",
              filter->name,
              j);
      }
    }
  }

  for (i=0;i<ms->nwriters;i++) {
    if (ms->writers[i] == NULL) {
      logwarn("%s: Sending schema to NULL writer (at %d)\n", ms->table_name, i);

    } else {
      ms->writers[i]->meta( ms->writers[i], schema);
    }
  }

  oml_free (schema);
  return 0;
}

/**
 *  Validate the name of the application.
 *
 *  If the application name contains a '/', it is truncated to the
 *  sub-string following the final '/'.  If the application name
 *  contains any characters other than alphanumeric characters or an
 *  underscore, it is declared invalid.  The first character must not
 *  be a digit.  Whitespace is not allowed.  An empty string is also
 *  not allowed.
 *
 *  @param name the name of the application.
 *
 *  @return If the application name is valid, a pointer to the
 *  application name, possibly truncated, is returned.  If the
 *  application name is not valid, returns NULL.  If the returned
 *  pointer is not NULL, it may be different from name.
 *
 */
const char*
validate_app_name (const char* name)
{
  size_t len = strlen (name);
  const char* p = name + len - 1;

  while (p >= name) {
    if (*p == '/')
      break;
    p--;
  }
  /* Either 1 character before name, or '/': need to move forward 1 character */
  p++;

  if (validate_name (p))
    return p;
  else
    return NULL;
}

static struct {
  const char *name;
  void (*enable)(void);
} feature_table [] = {
  { "default-log-simple", o_set_simplified_logging },
};
static const size_t feature_count = sizeof (feature_table) / sizeof (feature_table[0]);

/*
 * Parse features and enable the ones that are recognized.  features
 * should be a semicolon-separated list of features.
 *
 */
static void
setup_features (const char * const features)
{
  size_t i = 0;
  const char *p = features, *q;

  if (features == NULL)
    return;

  while (p && *p) {
    size_t len;
    char *name;
    q    = strchr(p, ';');
    len  = q ? (size_t)(q - p) : strlen (p);
    name = oml_strndup (p, len);
    p    = q ? (q + 1) : q;

    for (i = 0; i < feature_count; i++)
      if (strcmp (name, feature_table[i].name) == 0)
        feature_table[i].enable();
    oml_free (name);
  }
}

/*
 * Generate default file name to use when no output parameters are given.
 *
 * @param app_ame	the name of the application
 * @param name		the OML ID of the instance
 * @param domain	the experimental domain
 *
 * @return	A statically allocated buffer containing the URI of the output
 */
static char*
default_uri(const char *app_name, const char *name, const char *domain)
{
  /* Use a statically allocated buffer to avoid having to free it,
   * just like other URI sources in omlc_init() */
  static char uri[256];
  int remaining = sizeof(uri) - 1; /* reserve 1 for the terminating null byte */
  char *protocol = "file:";
  char time[25];
  struct timeval tv;

  gettimeofday(&tv, NULL);
  strftime(time, sizeof(time), "%Y-%m-%dt%H.%M.%S%z", localtime(&tv.tv_sec));

  *uri = 0;
  strncat(uri, protocol, remaining);
  remaining -= sizeof(protocol);

  strncat(uri, app_name, remaining);
  remaining -= strlen(app_name);

  if (name) {
    strncat(uri, "_", remaining);
    remaining--;
    strncat(uri, name, remaining);
    remaining -= strlen(name);
  }

  if (domain) {
    strncat(uri, "_", remaining);
    remaining--;
    strncat(uri, domain, remaining);
  }

  strncat(uri, "_", remaining);
  remaining--;
  strncat(uri, time, remaining);

  return uri;
}

/*
 Local Variables:
 mode: C
 tab-width: 2
 indent-tabs-mode: nil
 End:
 vim: sw=2:sts=2:expandtab
*/

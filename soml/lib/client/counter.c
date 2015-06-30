#include <stdio.h>
#include <stdlib.h>
#include <oml2/omlc.h>

int main (int argc, const char **argv)
{
  int result = omlc_init ("Simple", &argc, argv, NULL);
  if (result == -1) {
    fprintf (stderr, "Could not initialise OML\n");
    exit (1);
  } else if (result == 1) {
    fprintf (stderr, "OML was disabled by the user, exiting\n");
    exit (0);
  }

  OmlMPDef mp_def [] = {
    { "count", OML_UINT32_VALUE },
    { "count_str", OML_STRING_VALUE },
    { "count_real", OML_DOUBLE_VALUE },
    { NULL, (OmlValueT)0 }
  };

  OmlMP *mp = omlc_add_mp ("counter", mp_def);

  if (mp == NULL) {
    fprintf (stderr, "Error: could not register Measurement Point 'counter'");
    exit (1);
  }

  omlc_start();

  int i = 0;
  for (i = 0; i < 1000; i++) {
    uint32_t count = (uint32_t)i;
    char count_str[16];
    double count_real = (double)i;
    OmlValueU values[3];

    omlc_zero_array(values, 3);

    snprintf(count_str, sizeof(count_str), "%d", i);

    omlc_set_uint32 (values[0], count);
    omlc_set_string (values[1], count_str);
    omlc_set_double (values[2], count_real);

    omlc_inject (mp, values);

    omlc_reset_string(values[1]);
  }


  omlc_close();

  return 0;
}

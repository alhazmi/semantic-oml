Example sine generator reporting over OML	{#example}
=========================================

This simple example demonstrates both a simple OML-instrumented application, as
well as the code-generating capabilities of [oml2-scaffold(1)](http://oml.mytestbed.net/doc/oml/2.12/oml2-scaffold.1.html). See
[oml-scaffold-tute] for more details.

Most of the code has been generated with the following few steps.

    $ oml2-scaffold --app generator	# Create template description file
    Created 'generator.rb'
    $ vim generator.rb
    [edit measurement points and options]
    ^]:wq
    $ oml2-scaffold --main generator.rb	# Create template C code
    Created 'generator.c' and 'version.h'
    $ vim generator.c
    [add missing includes and sine-generation logic]
    ^]:wq
    $ oml2-scaffold --make generator.rb	# Create template Makefile
    $ vim Makefile
    [add -lm to LIBS, necessary for sin(3)]
    ^]:wq

The binary can be built with a simple call to `make`. In the process,
[oml2-scaffold(1)](http://oml.mytestbed.net/doc/oml/2.12/oml2-scaffold.1.html) will be used twice more, as called by the default Makefile
template:
 - `generator_oml.h`, based on the `defMeasurement`s in the description file, which
   contains the declaration of the measurement points, as well as helper
   functions to register them to OML and inject samples;
 - `generator_popt.h`, base on the `defPropertys` in the description file, which
   contain the basic option-parsing logics for the declared properties (not the
   `--oml-*` ones, which are directly handled by the OML client library, though
   options can be given in any order on the command line).

Using diff(1) on the distributed files, as compared to those generated by the
sequence of commands above, can further show how much of the code can be
generated automatically, so the developer can concentrate on the logic of the
application, rather than the samples management and reporting tasks.

The `config_*.xml` files are examples, usable with the generator, of how OML
Measurement Streams can be directed to one or more collection point, and
filterred or further processed along the way.

To build this example without installing the OML components system wide, run
  make CC="$PWD/../../libtool compile gcc" CCLD="$PWD/../../libtool link --tag=CC gcc" \
    SCAFFOLD="ruby $PWD/../../ruby/oml2-scaffold" \
    CFLAGS="-I$PWD/../../lib/client -I$PWD/../../lib/ocomm" \
    LDFLAGS="-L$PWD/../../lib/client/.libs -L$PWD/../../lib/ocomm/.libs"

The generator can then be run with, e.g.,
  LD_LIBRARY_PATH=$PWD/../../lib/client/.libs:$PWD/../../lib/ocomm/.libs \
    ./generator -n 10 -i .5  --oml-id generator --oml-domain test


[oml-scaffold-tute]: http://oml.mytestbed.net/projects/oml/wiki/OML2_scaffold_Tutorial
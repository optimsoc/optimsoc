osd_systracelogger class
------------------------

Obtain and decode a system trace (high-level API).

System traces are typically obtained by a STM module, which in turn gathers this information using processor-specific mechanisms (such as observing nop instructions, or writes to special registers).
System traces are streams of (id, value) tuples.
Some ids have a specification-defined meaning and can be decoded in a special way.
Most commonly, id 4 is used to transmit a single printed character from the software, providing an easy way to "print" data from software through the debug system to the host PC.
`osd_systracelogger` contains functions to decode such streams of characeters and write them to a file.

Usage
^^^^^

.. code-block:: c

  #include <osd/osd.h>
  #include <osd/systracelogger.h>

Public Interface
^^^^^^^^^^^^^^^^

.. doxygenfile:: libosd/include/osd/systracelogger.h

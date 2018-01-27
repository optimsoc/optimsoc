Error Handling
--------------

All functions in this project use a common scheme for return codes and error handling.

All functions, except for getters and setters, return the type :c:type:`osd_result`.
A return code of :c:macro:`OSD_OK` indicates a successful function call, any other return value indicates an error.
The error types are given as one of the `OSD_ERROR_*`.

Use :c:macro:`OSD_SUCCEEDED` to check if a function call was successful, and :c:macro:`OSD_FAILED` to check if it failed.

Use of assertations
^^^^^^^^^^^^^^^^^^^
Using assertations within libosd causes the application using libosd to crash.
A crash can be a good way to fail early, however, it may be used only for non-recoverable errors or programming errors.
In consequence, libosd makes use of assertations to check for internal errors (i.e. bugs in libosd), or to check for violated API calling conventions.
However, assertations are not used to check for invalid or missing input data from known-to-be-unreliable sources (e.g. from the network, from a user or from the file system).

Assertations are used in the following cases:

- API calling conventions are not followed.
- Violoated pre- or postconditions: the internal state when entering or exiting a function isn't what was expected.

Assertations are *not* used in in the following scenarios:

- User input to a function is not valid. 
- Unexpected data was received from the network, from the file system, or similar sources.
- The communication broke down.

Handling memory allocation errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
In general, out of memory errors returned from `malloc()` and similar functions are fatal, i.e. cause the program using libosd to crash.
This is motivated by the fact that these errors occur very rarely and the associated error handling path is rarely executed and tested.

In some cases where larger memory allocations are required, especially contiguous chunks of memory, error handling is implemented and fallback paths are used.

Example
^^^^^^^

.. code-block:: c
  
  #include <osd/osd.h>
  #include <assert.h>
  
  osd_result rv;
  
  // common case: pass error on to calling function
  rv = osd_some_call();
  if (OSD_FAILED(rv)) {
    return rv;
  }
  
  // check for a successful call
  rv = osd_another_call();
  if (OSD_SUCCEEDED(rv)) {
    printf("Call successful!\n");
  }
  
  // A failing call may not happen and hence is a bug in OSD: use assert()
  rv = osd_must_succeed_call();
  assert(OSD_SUCCEEDED(rv));
  
  // fatal malloc: assert if the memory allocation failed
  char* some_data = malloc(sizeof(char) * 100));
  assert(some_data);

Public Interface
^^^^^^^^^^^^^^^^

.. doxygengroup:: libosd-errorhandling
  :content-only:

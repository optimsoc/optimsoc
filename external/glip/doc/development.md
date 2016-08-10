@defgroup development Extending GLIP
GLIP is written in a way to make it easily extensible. Writing a new
communication backend should be rather easy. GLIP also contains some helper
functionality, such as an optimized
[circular buffer implementation](@ref development-cbuf), which can be reused
in your code.

How to add a new backend
------------------------

Adding a new backend requires only a couple of steps, which are listed below.
In general, have a look at the existing backends for inspiration how things
are done.

-# Create a new folder for your backend at `src/sw/backend_YOURNAME`. Make sure
   to consistently use `YOURNAME` as backend name everywhere. Inside this
   folder, add a `backend_YOURNAME.c` and a `backend_YOURNAME.h` file. It is
   usually a good idea to start by copying an existing implementation and
   removing all implementation parts. In the end your backend needs to implement
   all functions specified in glip_backend_if.

-# Add the backend to `configure.ac`. This usually includes searching for
   dependencies of the backend, e.g. required libraries or header files.
   The `cypressfx2` backend serves as a good example for a backend with a
   library dependency found though pkgconfig, the `tcp` backend can be an
   example for a backend without any dependencies.
   Simply copy the code and adjust it for your backend. Don't forget to re-run
   `autogen.sh` after modifying the `configure.ac` file.

-# Add the backend to `src/Makefile.am`. Again, have a look at the existing
   backends for inspiration. Essentially, you need to list all source files of
   your backend and any libraries the backend requires (which might have been
   detected by pkgconfig during configure.

-# Register your backend in `glip-private.h`. The file contains notes on where
   to add your backend. At the top of the file, you need to conditionally
   include the `backend_YOURNAME.h` file, and further down, you need to add the
   `BACKEND_YOURNAME` macro to the glip_backends array.

That's it! Now you can run the configure script with `--enable-YOURBACKEND`,
followed by `make` to see if your code compiles.

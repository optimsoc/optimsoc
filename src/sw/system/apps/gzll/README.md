# Build software

The software build process consists of two stages. First, it is
necessary to build at least one application. Afterwards, you can then
build an image with the applications and the kernel to execute it on
the system.

## Build pingpong demo

In this example two tasks will be started in rank 0 and rank 1. They
will send some messages back and forth.

Be sure to have both the ```or1k-elf``` and the ```or1k-gzll```
toolchain in your path. Furthermore you need the
```gzll-image-create``` script in your path.

### Build applications

```
cd apps/pingpong
make
```

### Build image

```
cd images/pingpong_0to1
gzll-image-create image.ini
make
```
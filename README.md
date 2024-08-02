# libmikmod 3.3.11.1 for NDS

## Introduction

http://blog.dev-scene.com/flatware

This is an unofficial Nintendo DS port of libmikmod, a portable sound
library for playing various music modules like MOD, S3M, IT, etc.

It is based on libmikmod 3.3.11.1, commit 0e5b74433880 from its official
repository: https://sourceforge.net/p/mikmod/mikmod/ci/master/tree/libmikmod/

## Install

Simply run this to build and install the library in your BlocksDS environment:

```bash
make install
```

## Examples

Once you have installed the library, go to the folder of an example and run:

```bash
make
```

This will generate a NDS file that you can test with flashcarts or emulators.

## Using the library

For general instructions on using libmikmod see the official manual here:
http://mikmod.raphnet.net/doc/libmikmod-3.1.10/docs/mikmod.html

For details on using this port see the examples in the `examples` folder of this
repository. I suggest you use that program as a template for your own project. You
can do whatever you want with the source code of the examples, but libmikmod
itself is LGPL. In practice this means your project needs to be open source (but
it does not need to be (L)GPL).

## More information

Two drivers are provided; one software mixer and one hardware mixer. The
software driver can use a lot of CPU power depending on the module played. The
hardware driver should use less, but it does not play all modules as good as the
software mixer.

The software driver does all mixing on ARM9, and uses only sound channel 0.
Unfortunately it uses TIMER0 and TIMER1 on ARM7 for timing.

The hardware driver uses sound channels 0 to x-1, where x is the number of
channels in the module. When using the hardware driver you need to call the tick
function on ARM9 every `md_bpm * 0.4` seconds.

Both the hardware and software driver needs to send messages from ARM9 to ARM7.
The communication itself needs to be done in the application. All messages will
be 32-bit and always >= `(1 << 28)`. You should allocate one of the user libnds
FIFO channels for the player messages.

## Changelog

- 3.3.11.1 (2024/08/02)
  - Updated to libmikmod 3.3.11.1.
  - Example updated to use modern libnds.

- 3.2.0ds1 (2007/12/22)
  - Based on libmikmod 3.2.0-beta2
  - Many fixes to the hardware mixer by Andreas Back. It is almost as good as
    the software mixer now!

- 3.1.11ds1 (2006/11/07)
  - Initial Release

## Credits

Original Nintendo DS port by Sten Larsson
sten.larsson@gmail.com

Contributions by Andreas Back
bitstate@gmail.com

Port to BlocksDS and update to version 3.3.11.1 by Antonio Niño Díaz

Official MikMod homepage
http://mikmod.raphnet.net/

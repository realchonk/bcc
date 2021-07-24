# The Brainlet C Compiler
<img src="util/bcc.png" width="256"><br>

## Description
This is my third iteration of compiler design and implementation.<br>
The <strong>bcc</strong> compiler is heavily based on the predecessor [benc](https://github.com/Benni3D/benc).<br>
The compiler generates assembly for the [System V ABI](https://wiki.osdev.org/System_V_ABI), where applicable.<br>
More documentation can be found on [bcc(1)](https://stuerz.xyz/bcc.html).<br>
Planned features can be read in the [TODO](./TODO) file.

## Target architectures
- i386 (x86-32/IA-32)
- x86\_64 (amd64/EMT64)
- riscv32 (RISC-V 32bit)
- riscv64 (RISC-V 64bit)

## Building/Installation
Building the compiler for the host architecture:<br>
```make```<br>
Builting the compiler for a different target architecture (eg. i386)<br>
```make TARGET=i386 all```<br>
Installing the compiler to /usr/local:<br>
```sudo make install```<br>
Installing the compiler to a different location (eg. /usr)<br>
```make PREFIX=/usr install```<br>
Disabling floating-point support can be done with DISABLE_FP=y as a make flag.<br>

## Testing
Testing can be performed with:<br>
```make test```

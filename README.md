# The Brainlet C Compiler
<img src="util/bcc.png" width="256"><br>

## Description
This is my third iteration of compiler design and implementation.<br>
The <strong>bcc</strong> compiler is heavily based on the predecessor [benc](https://github.com/Benni3D/benc).<br>
The compiler generates assembly for the [System V ABI](https://wiki.osdev.org/System_V_ABI), where applicable.<br>
More documentation can be found using the [man(1)](https://www.man7.org/linux/man-pages/man1/man.1.html) utility with the <em>./bcc.1</em> argument.<br>
Planned features can be read in the [TODO](./TODO) file.

## Target architectures
- i386 (x86-32/IA-32)
- x86\_64 (amd64/EMT64)

## Building/Installation
Building the compiler for the host architecture:<br>
```make```<br>
Builting the compiler for a different target architecture (eg. i386)<br>
```make TARGET=i386 all```<br>
Installing the compiler to /usr/local:<br>
```sudo make install```<br>
Installing the compiler to a different location (eg. /usr)<br>
```make PREFIX=/usr install```<br>

## Testing
Testing can be performed with:<br>
```make test```

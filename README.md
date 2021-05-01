# Compiler for a C-like language

## Target architectures
- i386 (x86-32/IA-32)
- x86\_64 (amd64/EMT64)

## Building/Installation
Building the compiler for the host architecture:<br>
```make```<br>
Builting the compiler for a different target architecture:<br>
```make TARGET=... all```<br>
Installing the compiler to /usr/local:<br>
```sudo make install```<br>
Installing the compiler to a different location:<br>
```make DESTDIR=... install```<br>

# Compiler for a C-like language

## Target architectures
- i386 (x86-32/IA-32)

## Building/Installation
Building the compiler for the default target architecture:<br>
```make```<br>
Builting the compiler for a different target architecture:<br>
```make ARCH=... all```<br>
Installing the compiler to /usr/local:<br>
```sudo make install```<br>
Installing the compiler to a different location:<br>
```make DESTDIR=... install```<br>

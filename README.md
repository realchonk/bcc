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
### Configuration
NOTE: if no configure script is available, please run
<code>./autogen.sh</code><br><br>
<code>./configure</code><br><br>
Common configure options:<br>
| Option | Description |
|--------|-------------|
| --help | see all available options |
| --prefix=PREFIX  | installation path |
| --target=TARGET | target architecture (SEE Target architectures) |
| --disable-fp | disable preliminary floating-point support |
| --with-cpu=CPU | select the target CPU |
| --with-abi=ABI | select the target ABI |
<br>
For Debug build use:<br>
<code>CFLAGS="-g -Og" ./configure</code>

### Building
Just a simple<br>
<code>make</code>

### Installation
Just install to PREFIX (default: /usr/local)<br>
<code>sudo make install</code><br>
Install to a different PREFIX:<br>
<code>make DESTDIR=... install</code><br>

Note: due to linking not being stable yet, please use the *bcl* wrapper instead, which performs the linking process for now.

## Testing
Testing can be performed with:<br>
<code>make check</code><br>
If you have any issues, please paste the output.<br>

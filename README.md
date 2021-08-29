# The Brainlet C Compiler
<img src="util/bcc.png" width="256"><br>

## Description
This is my third iteration of compiler design and implementation.<br>
The <strong>bcc</strong> compiler is heavily based on the predecessor [benc](https://github.com/Benni3D/benc).<br>
The compiler generates assembly for the [System V ABI](https://wiki.osdev.org/System_V_ABI), where applicable.<br>
More documentation can be found on [bcc(1)](https://stuerz.xyz/bcc.html).<br>
Planned features can be read in the [TODO](./TODO) file.


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
| --target=TARGET | [target architecture](#target-architecture) ||
| --enable-bcl | install the deprecated wrapper script [bcl](https://github.com/Benni3D/bcc/blob/master/util/bcl) |
| --disable-fp | disable preliminary floating-point support 
| --disable-target-libbcc | don't build & install the compiler-support library |
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

Note: if bcc can't find the crt{1,i,n}.o files, please run ./util/fix\_crts.sh<br>
Note 2: if linking against the GNU C library (glibc) fails, run ./util/fix\_crts.sh -af \<prefix\>, or use the musl C library.

## Testing
Testing can be performed with:<br>
<code>make check</code><br>
If you have any issues, please paste the output.<br>

## Target architecture
The target can be specified as: [cpu](#supported-processor-architectures)-vendor-[os](#supported-operating-systems)

### Supported processor architectures
- i386 (x86-32/IA-32)
- x86\_64 (amd64/EMT64)
- riscv32 (RISC-V 32bit)
- riscv64 (RISC-V 64bit)

### Supported operating systems
- Linux (musl libc is preferred, glibc needs a work-around)
- elf (freestanding)
  
## Contributing
Feel free to create an [Issue](https://github.com/Benni3D/bcc/issues) or a [Pull Request](https://github.com/Benni3D/bcc/pulls).<br>
Patches can also be send to <benni@stuerz.xyz>.<br>
Support for operating systems can be added [here](https://github.com/Benni3D/bcc/blob/master/util/m4/ax_set_predef_macros.m4).

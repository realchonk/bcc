#ifndef __BCC_CONFIG_H__
#define __BCC_CONFIG_H__

#undef __BCC_BITS

#if defined(__i386__)
#define __BCC_BITS 32
#elif defined(__x86_64__)
#define __BCC_BITS 64
#elif defined(__riscv__)

#define __BCC_BITS __riscv_xlen

#else
#error Unsupported processor architecture
#endif

#endif /* __BCC_CONFIG_H__ */

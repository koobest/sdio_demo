/*
 *  Copyright (c) 2010 - 2011 Espressif System
 *  
 */

#include <stdbool.h>

#ifndef _C_TYPES_H_
#define _C_TYPES_H_

#ifdef __GNUC__
#include <sys/cdefs.h>
#endif /*__GNUC__*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <machine/endian.h>

typedef unsigned char       uint8;
typedef unsigned char       u8;
typedef signed char         int8;
typedef signed char         int8;
typedef signed char         s8;
typedef unsigned short      uint16;
typedef unsigned short      u16;
typedef signed short        int16;
typedef signed short        s16;
typedef unsigned int        uint32;
typedef unsigned int        u32;
typedef signed int          int32;
typedef signed int          s32;
typedef int                 int32;
typedef signed long long    int64;
typedef unsigned long long  uint64;
typedef unsigned long long  u64;
typedef float               real32;
typedef double              real64;

#define __le16      u16
#define __le32		u32
#define __le64      u64

typedef unsigned int        size_t;

//only for GNU?
#define __ATTRIB_PACK           __attribute__ ((packed))
#define __ATTRIB_PRINTF         __attribute__ ((format (printf, 1, 2)))
#define __ATTRIB_NORETURN       __attribute__ ((noreturn))
#define __ATTRIB_ALIGN(x)       __attribute__ ((aligned((x))))
#define INLINE                  __inline__

//#ifndef __GNUC__
//#ifndef __packed
//#define __packed        __attribute__((packed))
//#endif
//#endif /*__GNUC__*/

#define LOCAL       static

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#define BIT(nr)                 (1UL << (nr))

#define REG_WRITE(_r, _v)    (*(volatile uint32 *)(_r)) = (_v)
#define REG_READ(_r) (*(volatile uint32 *)(_r))

#define REG_GET_BIT(_r, _b)  (*(volatile uint32_t*)(_r) & (_b))
#define REG_SET_BIT(_r, _b)  (*(volatile uint32_t*)(_r) |= (_b))
#define REG_CLR_BIT(_r, _b)  (*(volatile uint32_t*)(_r) &= ~(_b))
#define REG_SET_BITS(_r, _b, _m) (*(volatile uint32_t*)(_r) = (*(volatile uint32_t*)(_r) & ~(_m)) | ((_b) & (_m)))
#define VALUE_GET_FIELD(_r, _f) (((_r) >> (_f##_S)) & (_f))
#define VALUE_GET_FIELD2(_r, _f) (((_r) & (_f))>> (_f##_S))
#define VALUE_SET_FIELD(_r, _f, _v) ((_r)=(((_r) & ~((_f) << (_f##_S)))|((_v)<<(_f##_S))))
#define VALUE_SET_FIELD2(_r, _f, _v) ((_r)=(((_r) & ~(_f))|((_v)<<(_f##_S))))
#define FIELD_TO_VALUE(_f, _v) (((_v)&(_f))<<_f##_S)
#define FIELD_TO_VALUE2(_f, _v) (((_v)<<_f##_S) & (_f))
#define REG_GET_FIELD(_r, _f) ((REG_READ(_r) >> (_f##_S)) & (_f))
#define REG_SET_FIELD(_r, _f, _v) (REG_WRITE((_r),((REG_READ(_r) & ~((_f) << (_f##_S)))|(((_v) & (_f))<<(_f##_S)))))

#define ASSERT( x ) do { \
    if (!(x)) {          \
        os_printf("%s %u\n", __FILE__, __LINE__); \
        while (1) { \
			asm volatile("nop"); \
		};              \
    }                   \
} while (0)

#define panic os_printf
#define DBG_PRINTF os_printf

/* #if __GNUC_PREREQ__(4, 1) */
#ifndef __GNUC__
#if 1
#define __offsetof(type, field)  __builtin_offsetof(type, field)
#else
#define __offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif
#endif /* __GNUC__ */


/* Macros for counting and rounding. */
#ifndef howmany
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - __offsetof(type,member) );})

/*
 *  We use IDE for FPGA project, thus need explicitly declare storage segment
 *   The real chip separate ROM and RAM projects
 */
#ifdef FPGA
#define ROMFN_ATTR __attribute__((section(".iram0.text")))
#define PHY_ROMFN_ATTR __attribute__((section(".iram0.text")))
#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR __attribute__((section(".dram1.bss")))
#else /* FPGA */
#define ROMFN_ATTR
#define PHY_ROMFN_ATTR
#define RTCFN_ATTR __attribute__ ((section(".iram1.text")))
#define DMEM_ATTR __attribute__((section(".bss")))
#ifdef RAM_APP
#if 1
#define SHMEM_ATTR
#define SHMEM_FROM_HOST_ATTR
#define SHMEM_TO_HOST_ATTR
#endif //0000
#else
/* this is only for ROM */
#define SHMEM_ATTR
#endif /* RAM_APP */
#endif /* !FPGA */

#ifdef ICACHE_FLASH
#define ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif /* ICACHE_FLASH */

#endif /* _C_TYPES_H_ */

/*
 * B - Basic environment and compiler independence.
 */
#if !defined(IG_B_H)
#define IG_B_H 1

#define B_TRUE 1
#define B_FALSE 0
#define B_INVALID 0
#define B_NULL ((void*)0)
#define B_UNUSED(_x) (void)(_x)
#define B_CONCAT0(_a,_b) _a##_b
#define B_CONCAT1(_a,_b) B_CONCAT0(_a,_b)
#define B_CONCAT(_a,_b) B_CONCAT1(_a,_b)
#define B_STRINGIFY0(_x) #_x
#define B_STRINGIFY1(_x) #_x
#define B_STRINGIFY(_x) #_x
#define B_ARRAYLENGTH(_x) (sizeof(_x)/sizeof((_x)[0]))

#define B_COMP_CLANG 1
#define B_COMP_MSVC 2
#define B_COMP_GCC 3

#if !defined(B_COMP)
    #if defined(__clang__)
        #define B_COMP B_COMP_CLANG
    #elif defined(_MSC_VER)
        #define B_COMP B_COMP_MSVC
    #elif defined(__GNUC__)
        #define B_COMP B_COMP_GCC
    #else
        #define B_COMP B_INVALID
    #endif
#endif
#if B_COMP == B_INVALID
    #error "B: Could not identify the compiler."
#elif B_COMP > B_COMP_GCC /* @NOTE: Keep this in sync. */
    #error "B: Invalid compiler given."
#endif

#define B_ARCH_AMD64 1
#define B_ARCH_WASM32 2

#if !defined(B_ARCH)
    #if defined(__x86_64__) || defined(_M_AMD64)
        #define B_ARCH B_ARCH_AMD64
    #elif defined(__wasm32__)
        #define B_ARCH B_ARCH_WASM32
    #else
        #define B_ARCH B_INVALID
    #endif
#endif
#if B_ARCH == B_INVALID
    #error "B: Could not identify the architecture."
#elif B_ARCH > B_ARCH_WASM32 /* @NOTE: Keep this in sync. */
    #error "B: Invalid architecture given."
#endif

#define B_PLAT_WIN32 1
#define B_PLAT_HTML5 2

#if !defined(B_PLAT)
    #if defined(_WIN32)
        #define B_PLAT B_PLAT_WIN32
    #else
        #define B_PLAT B_INVALID
    #endif
#endif
#if B_PLAT == B_INVALID
    #error "B: Could not identify the platform."
#elif B_PLAT > B_PLAT_HTML5
    #error "B: Invalid platform given."
#endif

/* C++ guard. */
#if defined(__cplusplus)
    #define B_CPP_BEGIN extern "C" {
    #define B_CPP_END }
#else
    #define B_CPP_BEGIN
    #define B_CPP_END
#endif

B_CPP_BEGIN

/* Forced export. */
#if B_COMP == B_COMP_CLANG || B_COMP == B_COMP_GCC
    #define B_EXPORT __attribute__((visibility("default")))
    #define B_IMPORT
#elif B_COMP == B_COMP_MSVC
    #define B_EXPORT __declspec(dllexport)
    #define B_IMPORT __declspec(dllimport)
#else
    #define B_EXPORT
    #define B_IMPORT
#endif

/* Static assert */
#if defined(__cplusplus) && __cplusplus >= 201103L
    #define B_STATIC_ASSERT(_expr, _name) \
        static_assert((_expr), B_STRINGIFY(_name))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define B_STATIC_ASSERT(_expr, _name) \
        _Static_assert((_expr), B_STRINGIFY(_name))
#else
    #define B_STATIC_ASSERT(_expr, _name) \
        typedef struct B_CONCAT(static_assert_failed_,_name) { \
            B_CONCAT(static_assert_failed_,_name) : !!(cond); \
        } B_CONCAT(static_assert_failed_,_name)
#endif

/* Non returning functions. */
#if defined(__cplusplus) && __cplusplus >= 201103L
    #define B_NORETURN [[noreturn]] void
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define B_NORETURN _Noreturn void
#elif B_COMP == B_COMP_CLANG || B_COMP == B_COMP_GCC
    #define B_NORETURN __attribute__((noreturn)) void
#elif B_COMP == B_COMP_MSVC
    #define B_NORETURN __declspec(noreturn) void
#else
    #define B_NORETURN void
#endif

/* Calling convention */
#if B_COMP == B_COMP_CLANG || B_COMP == B_COMP_GCC
    #define B_CALLCONV_STDCALL __attribute__((stdcall))
#elif B_COMP == B_COMP_MSVC
    #define B_CALLCONV_STDCALL __stdcall
#endif

/* Break point. */
#if B_COMP == B_COMP_CLANG || B_COMP == B_COMP_GCC
    #define B_BREAKPOINT __asm__ volatile("int $0x03")
#elif B_COMP == B_COMP_MSVC
    #define B_BREAKPOINT __debugbreak
#endif

/* Markers */
#define B_INTERNAL static

/* Fixed width integers. */
#if B_ARCH == B_ARCH_AMD64
    #define B_I8  char
    #define B_I16 short
    #define B_I32 int
    #define B_I64 long long
    #define B_F32 float
    #define B_F64 double

    #define B_IWORD B_I32
    #define B_IPOINTER B_I64
#elif B_ARCH == B_ARCH_WASM32
    #define B_I8  char
    #define B_I16 short
    #define B_I32 int
    #define B_I64 long long
    #define B_F32 float
    #define B_F64 double

    #define B_IWORD B_I32
    #define B_IPOINTER B_I32
#endif

B_STATIC_ASSERT(sizeof(B_I8) == 1, i8_is_not_8_bits_long);
B_STATIC_ASSERT(sizeof(B_I16) == 2, i16_is_not_16_bits_long);
B_STATIC_ASSERT(sizeof(B_I32) == 4, i32_is_not_32_bits_long);
B_STATIC_ASSERT(sizeof(B_I64) == 8, i64_is_not_64_bits_long);
B_STATIC_ASSERT(sizeof(B_F32) == 4, f32_is_not_32_bits_long);
B_STATIC_ASSERT(sizeof(B_F64) == 8, f64_is_not_64_bits_long);

typedef unsigned B_I8  bU8;
typedef signed   B_I8  bS8;
typedef unsigned B_I16 bU16;
typedef signed   B_I16 bS16;
typedef unsigned B_I32 bU32;
typedef signed   B_I32 bS32;
typedef unsigned B_I64 bU64;
typedef signed   B_I64 bS64;
typedef B_F32 bF32;
typedef B_F64 bF64;

typedef unsigned B_IWORD bUWord;
typedef signed   B_IWORD bSWord;

typedef unsigned B_IPOINTER bPointer;
/*
 * @TODO: This may erroneously fail on segmented architectures, do we care about
 * any of them?
 */
B_STATIC_ASSERT(
    sizeof(bPointer) == sizeof(void *),
    i_pointer_is_not_pointer_sized
);

typedef bU8 bByte;
typedef bUWord bBool;

/* Standard functions. */

void bFillMemory(void *start, bPointer length, bByte value);
void bCopyMemory(void *from, void *to, bPointer length);
/* This is very common, may as well make a macro for it. */
#define bZero(_x) (bFillMemory(&(_x), sizeof(_x), 0))

#define B_ALIGNTO(_x, _a) (((_x) + ((_a) - 1)) & (-(_a)))
#define B_ALIGN2(_x) B_ALIGNTO(_x, 2)
#define B_ALIGN4(_x) B_ALIGNTO(_x, 4)
#define B_ALIGN8(_x) B_ALIGNTO(_x, 8)
#define B_ALIGN16(_x) B_ALIGNTO(_x, 16)

B_CPP_END

#endif

#if defined(B_IMPLEMENTATION)
#undef B_IMPLEMENTATION

B_CPP_BEGIN

/* Fix some MSVC problems. */
#if B_COMP == B_COMP_MSVC || B_COMP == B_COMP_CLANG
#if B_PLAT == B_PLAT_WIN32
B_EXPORT int _fltused = 1;
#pragma intrinsic(memset)
#endif

/*
 * <rant>
 * Because clang decides to generate a call to a leaf function in here, instead
 * of inlining it, like it should. Or even to understand that this is just a
 * proxy function and optimizing calls to it out, we have to copy the memset
 * code into it.
 * </rant>
 */
#if B_COMP == B_COMP_MSVC
void *__cdecl memset(void *_Dest, int _Val, bPointer _Size) {
#elif B_COMP == B_COMP_CLANG
void * memset(void *_Dest, int _Val, unsigned long _Size) {
#endif
    bByte *const c = _Dest;
    for(bPointer i = 0; i < _Size; i++) {
        c[i] = _Val;
    }
    return _Dest;
}
#endif

/* @TODO: Optimize these with vector instructions. */

void bFillMemory(void *start, bPointer length, bByte value) {
    bByte *const c = start;
    for(bPointer i = 0; i < length; i++) {
        c[i] = value;
    }
}

void bCopyMemory(void *from, void *to, bPointer length) {
    bByte *const f = from;
    bByte *const t = to;
    for(bPointer i = 0; i < length; i++) {
        t[i] = f[i];
    }
}

B_CPP_END

#endif

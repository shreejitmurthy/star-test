/* star.h - v0.3.2
   A single-header testing suite for C/C++.

   USAGE:
        Define `STAR_NO_ENTRY` in *one* source file before including this header to disable `main()` hijacking:
            #define STAR_NO_ENTRY
            #include "star.h"
        Define `STAR_NO_COLOR` to disable ASCII coloring:
        Define `STAR_NON_FATAL` so failed assertions don't abort the test entirely.
        Define `STAR_VEROBSE` or `STAR_VERBOSE_ASSERTS` for per-assert pass output.
        
        See the README.md for all features.

   LICENSE:
       See end of file for license information.
*/

#ifndef STAR_TEST_H
#define STAR_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*star_test_func)();

typedef struct {
    const char *name;
    star_test_func func;
} _star_test_case;

static int _star_current_failed = 0;
static size_t _star_test_count = 0;
static _star_test_case _star_tests[256];

static size_t _star_asserts_total  = 0;
static size_t _star_asserts_failed = 0;

#ifdef STAR_NON_FATAL
static const int _star_fatal = 0;
#else
static const int _star_fatal = 1;
#endif

#if defined(STAR_VERBOSE_ASSERTS) || defined(STAR_VERBOSE)
static const int _star_verbose = 1;
#else
static const int _star_verbose = 0;
#endif


#if !defined(STAR_NO_COLOR)
    #define STAR_FMT_FAIL_PREFIX   "\033[1;31m[FAIL]\033[0m "
    #define STAR_FMT_TEST_FAIL     "\033[1;31m[TEST FAILED]\033[0m "
    #define STAR_FMT_PASS_PREFIX   "\033[1;32m[PASS]\033[0m "
    #define STAR_FMT_TEST_PASS     "\033[1;32m[TEST PASSED]\033[0m "
    #define STAR_FMT_SUMMARY       "\n\033[1mTechnical and Reliable Summary:\033[0m "
    #define STAR_FMT_FILELINE      "\033[2m%s:%d\033[0m: "
    #define _STAR_CUSTOM(msg)      "\033[36m" msg "\033[0m"
#else
    #define STAR_FMT_FAIL_PREFIX   "[FAIL] "
    #define STAR_FMT_TEST_FAIL     "[TEST FAILED] "
    #define STAR_FMT_PASS_PREFIX   "[PASS] "
    #define STAR_FMT_TEST_PASS     "[TEST PASSED] "
    #define STAR_FMT_SUMMARY       "\nTechnical and Reliable Summary: "
    #define STAR_FMT_FILELINE      "%s:%d: "
    #define _STAR_CUSTOM(msg)      msg
#endif /* STAR_NO_COLOR */


#define _STAR_FAIL(format, ...)                                      \
    do {                                                             \
        fprintf(stderr, STAR_FMT_FAIL_PREFIX STAR_FMT_FILELINE,      \
                __FILE__, __LINE__);                                 \
        fprintf(stderr, format "\n", ##__VA_ARGS__);                 \
    } while (0)

#define _STAR_TEST_FAIL(format, ...)                                 \
    do {                                                             \
        fprintf(stderr, STAR_FMT_TEST_FAIL);                         \
        fprintf(stderr, format "\n", ##__VA_ARGS__);                 \
    } while (0)

#define _STAR_PASS(format, ...)      printf(STAR_FMT_PASS_PREFIX format "\n", ##__VA_ARGS__)
#define _STAR_TEST_PASS(format, ...) printf(STAR_FMT_TEST_PASS format "\n", ##__VA_ARGS__)
#define _STAR_SUMMARY(format, ...)   printf(STAR_FMT_SUMMARY format "\n", ##__VA_ARGS__)

// Test "Constructor"
#define TEST(name)                                                        \
    void name();                                                          \
    __attribute__((constructor))                                          \
    void register_##name() {                                              \
        _star_tests[_star_test_count++] = (_star_test_case){#name, name}; \
    }                                                                     \
    void name()

static inline bool __star_nearly_equal(double a, double b) {
    if (a == b) return true;
    double diff = fabs(a - b);
    double norm = fabs(a) + fabs(b);
    double scale = DBL_EPSILON * norm;
    if (scale < DBL_MIN) scale = DBL_MIN;
    return diff < scale;
}

static inline void __star_increment_failed() {
    _star_asserts_failed++;
    _star_current_failed = 1;
}
    
/* ASSERTS */
static inline bool __assert_eq(double a, double b, bool negate) {
    bool equal = __star_nearly_equal(a, b);
    bool ok = negate ? !equal : equal;

    if (!ok) {
        __star_increment_failed();
    }

    return ok;
}

static inline bool __assert_streq(char* a, char* b, bool negate) {
    bool equal = (strcmp(a, b) == 0);
    bool ok = negate ? !equal : equal;

    if (!ok) {
        __star_increment_failed();
    }

    return ok;
}

static inline bool __assert_kindaeq(double a, double b, double n, bool negate) {
    bool equal = (fabs((a) - (b)) <= n) ? true : false;
    bool ok = negate ? !equal : equal;

    if (!ok) {
       __star_increment_failed();
    }

    return ok;
}

static inline double __star_kinda_degree(const void *dptr) { 
    double n = 6.9; 
    if (dptr) n = *(const double*)dptr; 
    return n; 
}

static inline int __star_find_linear(
    const void *item,
    const void *container,
    size_t count,
    size_t elem_size,
    int (*equals)(const void*, const void*)
) {
    const char *base = (const char *)container;
    for (size_t i = 0; i < count; ++i) {
        const void *elem = base + i * elem_size;
        if (equals(item, elem)) return (int)i;
    }
    return -1;
}

/* MACROS */
// Equality & Inequality
#define __STAR_EQ_IMPL(label, a, b, negate, FAIL_CALL)                    \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if (!__assert_eq((double)(a), (double)(b), (negate))) {           \
            FAIL_CALL;                                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            if (!(negate))                                                \
                _STAR_PASS(#label "(%s, %s) passed: %lf = %lf",           \
                        #a, #b, (double)(a), (double)(b));                \
            else                                                          \
                _STAR_PASS(#label "(%s, %s) passed: %lf != %lf",          \
                    #a, #b, (double)(a), (double)(b));                    \
        }                                                                 \
    } while (0)

#define ASS_EQ(a, b)                                                      \
    __STAR_EQ_IMPL(ASS_EQ, a, b, false,                                   \
        _STAR_FAIL("ASS_EQ(%s, %s) failed: %lf != %lf",                   \
                   #a, #b, (double)(a), (double)(b)))

#define ASS_EQM(a, b, m)                                                  \
    __STAR_EQ_IMPL(ASS_EQM, a, b, false,                                  \
        _STAR_FAIL("ASS_EQM(%s, %s) %s",                                  \
                   #a, #b, _STAR_CUSTOM(m)))
                   
#define ASS_NEQ(a, b)                                                     \
    __STAR_EQ_IMPL(ASS_NEQ, a, b, true,                                   \
        _STAR_FAIL("ASS_NEQ(%s, %s) failed: %lf = %lf",                   \
                   #a, #b, (double)(a), (double)(b)))

#define ASS_NEQM(a, b, m)                                                 \
    __STAR_EQ_IMPL(ASS_NEQM, a, b, true,                                  \
        _STAR_FAIL("ASS_NEQM(%s, %s) %s",                                 \
                   #a, #b, _STAR_CUSTOM(m)))

#define __STAR_STREQ_IMPL(label, a, b, negate, FAIL_CALL)                 \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if (!__assert_streq(a, b, (negate))) {                            \
            FAIL_CALL;                                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            if (!(negate))                                                \
                _STAR_PASS(#label "(%s, %s) passed: %s = %s",             \
                    #a, #b, (a), (b));                                    \
            else                                                          \
                _STAR_PASS(#label "(%s, %s) passed: %s != %s",            \
                    #a, #b, (a), (b));                                    \
        }                                                                 \
    } while (0)

#define ASS_STREQ(a, b)                                                   \
    __STAR_STREQ_IMPL(ASS_STREQ, a, b, false,                             \
        _STAR_FAIL("ASS_STREQ(%s, %s) failed: %s != %s",                  \
                    #a, #b, (a), (b)));                                   \

#define ASS_STREQM(a, b, m)                                               \
    __STAR_STREQ_IMPL(ASS_STREQM, a, b, false,                            \
        _STAR_FAIL("ASS_STREQM(%s, %s) %s",                               \
                #a, #b, _STAR_CUSTOM(m)));                                \

#define ASS_STRNEQ(a, b)                                                  \
    __STAR_STREQ_IMPL(ASS_STRNEQ, a, b, true,                             \
        _STAR_FAIL("ASS_STRNEQ(%s, %s) failed: %s != %s",                 \
                    #a, #b, (a), (b)));                                   \

#define ASS_STRNEQM(a, b, m)                                              \
    __STAR_STREQ_IMPL(ASS_STRNEQM, a, b, true,                            \
        _STAR_FAIL("ASS_STRNEQM(%s, %s) %s",                              \
                #a, #b, _STAR_CUSTOM(m)));                                \

#define __STAR_KINDAEQ_IMPL(label, a, b, dptr, negate, FAIL_CALL)              \
    do {                                                                       \
        _star_asserts_total++;                                                 \
        double n = __star_kinda_degree(dptr);                                  \
        if (!__assert_kindaeq((a), (b), n, (negate))) {                        \
            FAIL_CALL;                                                         \
            if (_star_fatal) return;                                           \
        } else if (_star_verbose) {                                            \
            if (!(negate))                                                     \
                _STAR_PASS(#label "(%s, %s) passed: %lf ≈ %lf (degree %lf)",   \
                       #a, #b, (double)(a), (double)(b), n);                   \
            else                                                               \
                _STAR_PASS(#label "(%s, %s) passed: %lf !≈ %lf (degree %lf)",  \
                       #a, #b, (double)(a), (double)(b), n);                   \
        }                                                                      \
    } while (0)

#define ASS_KINDAEQ(a, b, dptr)                                                \
    __STAR_KINDAEQ_IMPL(ASS_KINDAEQ, a, b, dptr, false,                        \
        _STAR_FAIL("ASS_KINDAEQ(%s, %s) failed: %lf !≈ %lf (degree %lf)",      \
                  #a, #b, (double)(a), (double)(b), n) /* macro trickery.. */  \
    )

#define ASS_KINDAEQM(a, b, dptr, m)                                            \
    __STAR_KINDAEQ_IMPL(ASS_KINDAEQM, a, b, dptr, false,                       \
        _STAR_FAIL("ASS_KINDAEQM(%s, %s) %s",                                  \
                  #a, #b, _STAR_CUSTOM(m))                                     \
    )

#define ASS_KINDANEQ(a, b, dptr)                                               \
    __STAR_KINDAEQ_IMPL(ASS_KINDANEQ, a, b, dptr, true,                        \
        _STAR_FAIL("ASS_KINDANEQ(%s, %s) failed: %lf ≈ %lf (degree %lf)",      \
                  #a, #b, (double)(a), (double)(b), n)                         \
    )

#define ASS_KINDANEQM(a, b, dptr, m)                                           \
    __STAR_KINDANEQ_IMPL(ASS_KINDANEQM, a, b, dptr, true,                      \
        _STAR_FAIL("ASS_KINDNAEQM(%s, %s) %s",                                 \
                  #a, #b, _STAR_CUSTOM(m))                                     \
    )

#define __STAR_BOOL_IMPL(label, expr, expect_true, FAIL_CALL)             \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if (!!(expr) != !!(expect_true)) {                                \
            FAIL_CALL;                                                    \
            __star_increment_failed();                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            _STAR_PASS(#label "(%s) passed", #expr);                      \
        }                                                                 \
    } while (0)

#define ASS_TRUE(expr)                                                    \
    __STAR_BOOL_IMPL(ASS_TRUE, expr, true,                                \
        _STAR_FAIL("ASS_TRUE(%s) failed", #expr))

#define ASS_TRUEM(expr, m)                                                \
    __STAR_BOOL_IMPL(ASS_TRUE, expr, true,                                \
        _STAR_FAIL("ASS_TRUEM(%s) %s", #expr, _STAR_CUSTOM(m)))

#define ASS_FALSE(expr)                                                   \
    __STAR_BOOL_IMPL(ASS_FALSE, expr, false,                              \
        _STAR_FAIL("ASS_FALSE(%s) failed", #expr))

#define ASS_FALSEM(expr, m)                                               \
    __STAR_BOOL_IMPL(ASS_FALSE, expr, false,                              \
        _STAR_FAIL("ASS_FALSM(%s) %s", #expr, _STAR_CUSTOM(m)))


#define __STAR_MEMCMP_IMPL(label, a, b, negate, FAIL_CALL)               \
    do {                                                                 \
        _star_asserts_total++;                                           \
        int __star_memcmp_res = memcmp(&(a), &(b), sizeof(a));           \
        bool __star_equal = (__star_memcmp_res == 0);                    \
        if (__star_equal == (negate)) {                                  \
            FAIL_CALL;                                                   \
            __star_increment_failed();                                   \
            if (_star_fatal) return;                                     \
        } else if (_star_verbose) {                                      \
            if (!(negate))                                               \
                _STAR_PASS(#label "(%s, %s) passed", #a, #b);            \
            else                                                         \
                _STAR_PASS(#label "(%s, %s) passed (not equal)", #a, #b);\
        }                                                                \
    } while (0)

#define ASS_IS(a, b)                                                     \
    __STAR_MEMCMP_IMPL(ASS_IS, a, b, false,                              \
        _STAR_FAIL("ASS_IS(%s, %s) failed", #a, #b))

#define ASS_ISM(a, b, m)                                                 \
    __STAR_MEMCMP_IMPL(ASS_IS, a, b, false,                              \
        _STAR_FAIL("ASS_ISM(%s, %s) %s", #a, #b, _STAR_CUSTOM(m)))

#define ASS_ISNT(a, b)                                                   \
    __STAR_MEMCMP_IMPL(ASS_ISNT, a, b, true,                             \
        _STAR_FAIL("ASS_ISNT(%s, %s) failed", #a, #b))

#define ASS_ISNTM(a, b, m)                                               \
    __STAR_MEMCMP_IMPL(ASS_ISNT, a, b, true,                             \
        _STAR_FAIL("ASS_ISNTM(%s, %s) %s", #a, #b, _STAR_CUSTOM(m)))


// Null / None / Undefined
#define ASS_ISNULL(expr)                                                  \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if ((expr) != NULL) {                                             \
            _STAR_FAIL("ASS_ISNULL(%s) failed", #expr);                   \
            __star_increment_failed();                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            _STAR_PASS("ASS_ISNULL(%s) passed", #expr);                   \
        }                                                                 \
    } while (0)

#define ASS_ISNULLM(expr, m)                                              \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if ((expr) != NULL) {                                             \
            _STAR_FAIL("ASS_ISNULL(%s) %s", #expr, _STAR_CUSTOM(m));      \
            __star_increment_failed();                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            _STAR_PASS("ASS_ISNULL(%s) passed", #expr);                   \
        }                                                                 \
    } while (0)

#define ASS_ISNTNULL(expr)                                                \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if ((expr) == NULL) {                                             \
            _STAR_FAIL("ASS_ISNTNULL(%s) failed", #expr);                 \
            __star_increment_failed();                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            _STAR_PASS("ASS_ISNTNULL(%s) passed", #expr);                 \
        }                                                                 \
    } while (0)

#define ASS_ISNTNULLM(expr, m)                                            \
    do {                                                                  \
        _star_asserts_total++;                                            \
        if ((expr) == NULL) {                                             \
            _STAR_FAIL("ASS_ISNTNULL(%s) %s", #expr, _STAR_CUSTOM(m));    \
            __star_increment_failed();                                    \
            if (_star_fatal) return;                                      \
        } else if (_star_verbose) {                                       \
            _STAR_PASS("ASS_ISNTNULL(%s) passed", #expr);                 \
        }                                                                 \
    } while (0)

// Comparisons
/*
 * op_code:
 *   0 = a >  b
 *   1 = a >= b
 *   2 = a <  b
 *   3 = a <= b
 *
 * PASS messages and FAIL messages are formatted by each public macro.
 */

#define __STAR_CMP_IMPL(a, b, op_code, FAIL_CALL, PASS_FMT)              \
    do {                                                                 \
        _star_asserts_total++;                                           \
        double __va = (double)(a);                                       \
        double __vb = (double)(b);                                       \
        bool __ok = false;                                               \
        switch (op_code) {                                               \
            case 0: __ok = (__va >  __vb); break;                        \
            case 1: __ok = (__va >= __vb); break;                        \
            case 2: __ok = (__va <  __vb); break;                        \
            case 3: __ok = (__va <= __vb); break;                        \
        }                                                                \
        if (!__ok) {                                                     \
            FAIL_CALL;                                                   \
            __star_increment_failed();                                   \
            if (_star_fatal) return;                                     \
        } else if (_star_verbose) {                                      \
            _STAR_PASS PASS_FMT;                                         \
        }                                                                \
    } while (0)

#define ASS_GREATER(a, b)                                                \
    __STAR_CMP_IMPL(a, b, 0,                                             \
        _STAR_FAIL("ASS_GREATER(%s, %s) failed: %lf <= %lf",             \
                   #a, #b, (double)(a), (double)(b)),                    \
        ("ASS_GREATER(%s, %s) passed: %lf > %lf",                        \
            #a, #b, __va, __vb))

#define ASS_GREATERM(a, b, m)                                            \
    __STAR_CMP_IMPL(a, b, 0,                                             \
        _STAR_FAIL("ASS_GREATERM(%s, %s) %s",                            \
                   #a, #b, _STAR_CUSTOM(m)),                             \
        ("ASS_GREATERM(%s, %s) passed: %lf > %lf",                       \
            #a, #b, __va, __vb))

#define ASS_GREATEREQ(a, b)                                              \
    __STAR_CMP_IMPL(a, b, 1,                                             \
        _STAR_FAIL("ASS_GREATEREQ(%s, %s) failed: %lf < %lf",            \
                   #a, #b, (double)(a), (double)(b)),                    \
        ("ASS_GREATEREQ(%s, %s) passed: %lf >= %lf",                     \
            #a, #b, __va, __vb))

#define ASS_GREATEREQM(a, b, m)                                          \
    __STAR_CMP_IMPL(a, b, 1,                                             \
        _STAR_FAIL("ASS_GREATEREQM(%s, %s) %s",                          \
                   #a, #b, _STAR_CUSTOM(m)),                             \
        ("ASS_GREATEREQM(%s, %s) passed: %lf >= %lf",                    \
            #a, #b, __va, __vb))

#define ASS_LESSER(a, b)                                                 \
    __STAR_CMP_IMPL(a, b, 2,                                             \
        _STAR_FAIL("ASS_LESSER(%s, %s) failed: %lf >= %lf",              \
                   #a, #b, (double)(a), (double)(b)),                    \
        ("ASS_LESSER(%s, %s) passed: %lf < %lf",                         \
            #a, #b, __va, __vb))

#define ASS_LESSERM(a, b, m)                                             \
    __STAR_CMP_IMPL(a, b, 2,                                             \
        _STAR_FAIL("ASS_LESSERM(%s, %s) %s",                             \
                   #a, #b, _STAR_CUSTOM(m)),                             \
        ("ASS_LESSERM(%s, %s) passed: %lf < %lf %s",                     \
            #a, #b, __va, __vb, _STAR_CUSTOM(m)))

#define ASS_LESSEREQ(a, b)                                               \
    __STAR_CMP_IMPL(a, b, 3,                                             \
        _STAR_FAIL("ASS_LESSEREQ(%s, %s) failed: %lf > %lf",             \
                   #a, #b, (double)(a), (double)(b)),                    \
        ("ASS_LESSEREQ(%s, %s) passed: %lf <= %lf",                      \
            #a, #b, __va, __vb))

#define ASS_LESSEREQM(a, b, m)                                           \
    __STAR_CMP_IMPL(a, b, 3,                                             \
        _STAR_FAIL("ASS_LESSEREQM(%s, %s) %s",                           \
                   #a, #b, _STAR_CUSTOM(m)),                             \
        ("ASS_LESSEREQM(%s, %s) passed: %lf <= %lf",                     \
            #a, #b, __va, __vb))

// Collections / Sequences
#define __STAR_VALUE_EQUALS(a, b) _Generic((a),                                  \
    const char*: strcmp,                                                         \
    char*: strcmp,                                                               \
    default: __star_nearly_equal                                                 \
)(a, b)

#define ASS_IN(item, container)                                                  \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int _star_found = 0;                                                     \
        for (int i = 0;                                                          \
             i < (int)(sizeof(container) / sizeof((container)[0]));              \
             i++) {                                                              \
            if (__STAR_VALUE_EQUALS((container)[i], (item))) {                   \
                _star_found = 1;                                                 \
                if (_star_verbose)                                               \
                    _STAR_PASS("ASS_IN(%s, %s) passed: %s found at index %d",    \
                              #item, #container, #item, i);                      \
                break;                                                           \
            }                                                                    \
        }                                                                        \
        if (!_star_found) {                                                      \
            _STAR_FAIL("ASS_IN(%s, %s) failed: %s not found",                    \
                      #item, #container, #item);                                 \
            __star_increment_failed();                                           \
            if (_star_fatal) return;                                             \
        }                                                                        \
    } while (0)


#define ASS_INM(item, container, m)                                              \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int _star_found = 0;                                                     \
        for (int i = 0;                                                          \
             i < (int)(sizeof(container) / sizeof((container)[0]));              \
             i++) {                                                              \
            if (__STAR_VALUE_EQUALS((container)[i], (item))) {                   \
                _star_found = 1;                                                 \
                if (_star_verbose)                                               \
                    _STAR_PASS("ASS_IN(%s, %s) passed: %s found at index %d",    \
                              #item, #container, #item, i);                      \
                break;                                                           \
            }                                                                    \
        }                                                                        \
        if (!_star_found) {                                                      \
            _STAR_FAIL("ASS_IN(%s, %s) %s",                                      \
                      #item, #container, _STAR_CUSTOM(m));                       \
            __star_increment_failed();                                           \
            if (_star_fatal) return;                                             \
        }                                                                        \
    } while (0)

#define ASS_NOTIN(item, container)                                               \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int _star_found = 0;                                                     \
        for (int i = 0;                                                          \
             i < (int)(sizeof(container) / sizeof((container)[0]));              \
             i++) {                                                              \
            if (__STAR_VALUE_EQUALS((container)[i], (item))) {                   \
                _star_found = 1;                                                 \
                _STAR_FAIL("ASS_NOTIN(%s, %s) failed: %s found at index %d",     \
                          #item, #container, #item, i);                          \
                __star_increment_failed();                                       \
                if (_star_fatal) return;                                         \
                break;                                                           \
            }                                                                    \
        }                                                                        \
        if (!_star_found && _star_verbose) {                                     \
            _STAR_PASS("ASS_NOTIN(%s, %s) passed: %s not found",                 \
                      #item, #container, #item);                                 \
        }                                                                        \
    } while (0)   

#define ASS_NOTINM(item, container, m)                                           \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int _star_found = 0;                                                     \
        for (int i = 0;                                                          \
             i < (int)(sizeof(container) / sizeof((container)[0]));              \
             i++) {                                                              \
            if (__STAR_VALUE_EQUALS((container)[i], (item))) {                   \
                _star_found = 1;                                                 \
                _STAR_FAIL("ASS_NOTIN(%s, %s) %s",                               \
                          #item, #container, _STAR_CUSTOM(m));                   \
                __star_increment_failed();                                       \
                if (_star_fatal) return;                                         \
                break;                                                           \
            }                                                                    \
        }                                                                        \
        if (!_star_found && _star_verbose) {                                     \
            _STAR_PASS("ASS_NOTIN(%s, %s) passed: %s not found",                 \
                      #item, #container, #item);                                 \
        }                                                                        \
    } while (0)

#define ASS_INBIN(item, container)                                               \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int n = (int)sizeof(container) / sizeof((container)[0]);                 \
        int low = 0;                                                             \
        int high = n - 1;                                                        \
        int _star_found = 0;                                                     \
        while (low <= high) {                                                    \
            int mid = low + (high - low) / 2;                                    \
            if (__STAR_VALUE_EQUALS((container)[mid], (item))) {                 \
                _star_found = 1;                                                 \
                if (_star_verbose)                                               \
                    _STAR_PASS("ASS_INBIN(%s, %s) passed, %s found at index %d", \
                        #item, #container, #item, mid);                          \
                break;                                                           \
            } else if ((container)[mid] < (item)) {                              \
                low = mid + 1;                                                   \
            } else {                                                             \
                high = mid - 1;                                                  \
            }                                                                    \
        }                                                                        \
        if (!_star_found) {                                                      \
            _STAR_FAIL("ASS_INBIN(%s, %s) failed: %s not found",                 \
                #item, #container, #item);                                       \
                __star_increment_failed();                                       \
            if (_star_fatal) return;                                             \
        }                                                                        \
    } while (0)

#define ASS_INBINM(item, container, m)                                           \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int n = (int)sizeof(container) / sizeof((container)[0]);                 \
        int low = 0;                                                             \
        int high = n - 1;                                                        \
        int _star_found = 0;                                                     \
        while (low <= high) {                                                    \
            int mid = low + (high - low) / 2;                                    \
            if (__STAR_VALUE_EQUALS((container)[mid], (item))) {                 \
                _star_found = 1;                                                 \
                if (_star_verbose)                                               \
                    _STAR_PASS("ASS_INBINM(%s, %s) passed, %s found at index %d", \
                        #item, #container, #item, mid);                          \
                break;                                                           \
            } else if ((container)[mid] < (item)) {                              \
                low = mid + 1;                                                   \
            } else {                                                             \
                high = mid - 1;                                                  \
            }                                                                    \
        }                                                                        \
        if (!_star_found) {                                                      \
            _STAR_FAIL("ASS_INBINM(%s, %s) %s",                                  \
                #item, #container, _STAR_CUSTOM(m));                             \
            __star_increment_failed();                                           \
            if (_star_fatal) return;                                             \
        }                                                                        \
    } while (0)

#define ASS_NOTINBIN(item, container)                                            \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int n = (int)sizeof(container) / sizeof((container)[0]);                 \
        int low = 0;                                                             \
        int high = n - 1;                                                        \
        int _star_found = 0;                                                     \
        while (low <= high) {                                                    \
            int mid = low + (high - low) / 2;                                    \
            if (__STAR_VALUE_EQUALS((container)[mid], (item))) {                 \
                _star_found = 1;                                                 \
                _STAR_FAIL("ASS_INBIN(%s, %s) passed: %s found",                 \
                    #item, #container, #item);                                   \
                __star_increment_failed();                                       \
                if (_star_fatal) return;                                         \
                break;                                                           \
            } else if ((container)[mid] < (item)) {                              \
                low = mid + 1;                                                   \
            } else {                                                             \
                high = mid - 1;                                                  \
            }                                                                    \
        }                                                                        \
        if (!_star_found && _star_verbose) {                                     \
            _STAR_PASS("ASS_NOTINBINM(%s, %s) passed: %s not found",             \
                      #item, #container, #item);                                 \
        }                                                                        \
    } while (0)


#define ASS_NOTINBINM(item, container, m)                                        \
    do {                                                                         \
        _star_asserts_total++;                                                   \
        int n = (int)sizeof(container) / sizeof((container)[0])                  \
        int low = 0;                                                             \
        int high = n - 1;                                                        \
        int _star_found = 0;                                                     \
        while (low <= high) {                                                    \
            int mid = low + (high - low) / 2;                                    \
            if (__STAR_VALUE_EQUALS((container[mid], (item)))) {                 \
                _star_found = 1;                                                 \
                _STAR_FAIL("ASS_NOTINBINM(%s, %s) %s",                           \
                          #item, #container, _STAR_CUSTOM(m));                   \
                __star_increment_failed();                                       \
                if (_star_fatal) return;                                         \
                break;                                                           \
            } else if ((container)[mid] < (item)) {                              \
                low = mid + 1;                                                   \
            } else {                                                             \
                high = mid - 1;                                                  \
            }                                                                    \
        }                                                                        \
        if (!_star_found && _star_verbose) {                                     \
            _STAR_PASS("ASS_NOTINBINM(%s, %s) passed: %s not found",             \
                      #item, #container, #item);                                 \
        }                                                                        \
    } while (0)

// Forced fail
#define DIE()                \
    do {                     \
        _STAR_FAIL("DIE()"); \
    } while (0)


static int __star_run_internal(bool verbose_start) {
    if (verbose_start) printf("\033[1mRunning %zu tests...\033[0m\n", _star_test_count);

    int passed_tests = 0;
    int failed_tests = 0;

    for (int i = 0; i < (int)_star_test_count; i++) {
        _star_current_failed = 0;

        size_t before_total  = _star_asserts_total;
        size_t before_failed = _star_asserts_failed;

        _star_tests[i].func();

        size_t test_total  = _star_asserts_total  - before_total;
        size_t test_failed = _star_asserts_failed - before_failed;
        size_t test_passed = test_total - test_failed;

        if (_star_current_failed) {
            _STAR_TEST_FAIL("%s: %zu/%zu assertions passed (%zu failed)", _star_tests[i].name, test_passed, test_total, test_failed);
            failed_tests++;
        } else {
            _STAR_TEST_PASS("%s: %zu/%zu assertions passed", _star_tests[i].name, test_passed, test_total);
            passed_tests++;
        }
    }

    size_t total_passed_asserts = _star_asserts_total - _star_asserts_failed;

    if (verbose_start) _STAR_SUMMARY("%d/%zu tests passed, %d failed " "(%zu/%zu assertions passed)", 
        passed_tests, _star_test_count, failed_tests, total_passed_asserts, _star_asserts_total);

    return failed_tests ? 1 : 0;
}

/* Run Functionality */
#if defined(STAR_NO_ENTRY)
static inline int star_run(int verbose_start) {
    return __star_run_internal(verbose_start);
}
#else
int main(int argc, char** argv) {
    return __star_run_internal(true);
}
#endif /* STAR_NO_ENTRY */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STAR_TEST_H */

/*
    Revision history:
        0.6.5  (2025-11-29)  Refactoring kinda, is, and comparison macros. 
        0.6.4  (2025-11-29)  Refactoring string assertion macros.
        0.6.3  (2025-11-27)  Basic refactoring assertion macros for improved readability and maintainability.
                             Done N/EQ/M and KINDA/N/EQ/M.
        0.6.2  (2025-11-27)  Fixed KINDANEQ/M logic to properly fail and append to global asserts + refactored
                             error message macros improved readability and consistency.
        0.6.1  (2025-11-25)  Added binary search collection asserts and custom messages.
        0.6.0  (2025-11-24)  Changes:
                                - 0.5.1: Custom message color is now normal intensity cyan. 
                                - 0.5.2: Consistent `bool` instead of `int` for boolean return values.
                                - 0.6.0: Added some basic collection assertions
        0.5.0  (2025-11-24)  Changes:
                                - 0.4.1: `STAR_VERBOSE` also works in addition to `STAR_VERBOSE_ASSERTS`.
                                - 0.4.2: Now using epsilon-based floating point comparison.
                                - 0.4.3: Bold coloring for color-enabled output on assert outcome (FAIL, PASS).
                                - 0.5.0: Support for custom messages on failed assertions. Custom messages have
                                         high intensity cyan color.
                                         (squeeze) Also added comparison assertions and forced failure.
        0.4.0  (2025-11-23)  Many changes:
                                - 0.3.2: automatic file-line display in _STAR_FAIL + top-file description.
                                - 0.3.1: global test count is now size_t, user-irrelevant identifiers
                                         now prefixed with `_star`.
                                - 0.3.3: Non-fatal option to *not* abort after failing an assert.
                                - 0.3.4: Verbose asserts to indicate which asserts passed (even if test failed).
                                - 0.3.5: More informative output with passed tests and asserts.
                                - 0.4.0: Added object and null checking asserts.
        0.3.0  (2025-11-22)  Added more equality + bool asserts.                      
        0.2.0  (2025-11-22)  Created test running and two asserts. `main()` hijacking toggleable and
                             ASCII coloring also added.
        0.1.0  (2025-11-22)  First push.
*/

/*
    MIT License

    Copyright (c) 2025 Shreejit Murthy

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

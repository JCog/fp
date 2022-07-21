#ifndef ADEX_H
#define ADEX_H
#include "common.h"
#include <vector/vector.h>

enum AdexError {
    ADEX_ERROR_SUCCESS,
    ADEX_ERROR_MEMORY,
    ADEX_ERROR_SYNTAX,
    ADEX_ERROR_ARITHMETIC,
    ADEX_ERROR_ADDRESS,
};

struct Adex {
    struct vector expr;
};

enum AdexError adexParse(struct Adex *adex, const char *str);
enum AdexError adexEval(struct Adex *adex, u32 *result);
void adexDestroy(struct Adex *adex);

extern const char *adexErrorName[];

#endif

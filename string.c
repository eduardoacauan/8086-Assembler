#include "asm.h"
#include <assert.h>
#include <memory.h>
typedef struct CString CString;

static struct CString {
    char    *str;
    size_t   len;
    CString *next;
}*buckets[1024];

static unsigned long table[] = {
        28100,  23902,  4932,   3168,   5581,   2995,   25135,  10789,  21553,  25523,
        4588,   8179,   24311,  3350,   3558,   17372,  10999,  1058,   4313,   5884,
        24815,  2674,   25222,  23645,  10450,  5367,   7595,   5912,   16227,  23356,
        4658,   13245,  2563,   27630,  6057,   24821,  7367,   27291,  25586,  23398,
        26318,  23226,  14908,  12503,  30930,  7148,   1586,   16021,  6652,   31617,
        14193,  28437,  7933,   32574,  28992,  31450,  14170,  19805,  25211,  12441,
        9479,   21974,  22993,  2204,   4664,   8413,   27184,  1892,   31092,  26284,
        13180,  13202,  21483,  21634,  9095,   8968,   521,    8940,   17214,  25419,
        8779,   24899,  25632,  16683,  19026,  6774,   16885,  13223,  22070,  14089,
        27836,  28171,  16924,  16478,  3813,   23899,  16907,  26932,  25536,  5364,
        3067,   32621,  29238,  28273,  2095,   11028,  24503,  24780,  23493,  7838,
        28531,  21189,  396,    7280,   461,    27784,  25877,  30114,  10252,  11402,
        13325,  26115,  8276,   17332,  22985,  29021,  19926,  31939,  22819,  9722,
        323,    13039,  30853,  26726,  9606,   7114,   29763,  11383,  9394,   26732,
        11054,  16267,  6457,   461,    31078,  2065,   8032,   25714,  13679,  24139,
        23279,  24396,  903,    25169,  1325,   14813,  29495,  29530,  5899,   1436,
        6953,   11319,  25504,  7835,   10041,  2336,   14037,  8039,   3278,   6835,
        19554,  19503,  22891,  27194,  13049,  14633,  20353,  29485,  28608,  12338,
        17800,  22008,  30213,  6581,   31565,  10237,  18222,  27853,  22926,  1245,
        20206,  27310,  20420,  17651,  20415,  10561,  17472,  32034,  29580,  12558,
        19065,  11217,  21677,  12895,  5029,   14131,  4889,   5538,   19846,  8396,
        18903,  27408,  25479,  4657,   30902,  32140,  22023,  9999,   13886,  3278,
        7157,   6254,   15846,  29376,  17053,  11575,  11912,  14376,  16321,  5108,
        3345,   23688,  10732,  2520,   24445,  21826,  19203,  29439,  29699,  2048,
        16669,  18422,  10380,  17491,  8596,   31870,  18026,  155,    21085,  17293,
        17991,  24102,  3476,   21654,  4187
};

const char *String(const char *str)
{
    const char *ptr = str;
    assert(str);

    while(*++ptr);

    return StringN(str, ptr - str);
}

const char *IString(const char * str)
{
    const char *ptr = str;
    assert(str);

    while (*++ptr);

    return ContainsIStr(str, ptr - str);
}

const char *StringN(const char *str, size_t len)
{
    unsigned    hash = 0;
    const char *end = str;
    CString    *tmps;

    assert(str && len);

    for(size_t i = 0; i < len; i++)
        hash = (hash << 1) + table[*(unsigned char*)end++];

    hash &= 1023;

    for(tmps = buckets[hash]; tmps; tmps = tmps->next){
        const char *s1, *s2;

        if(tmps->len != len)
            continue;

        s1 = str;
        s2 = tmps->str;

        do
            if(s1 == end)
                return tmps->str;
        while(*s1++ == *s2++);
    }

    tmps = (CString*)Alloc(sizeof(CString), ARENA_1);

    tmps->str = (char*)Alloc(sizeof(char) * len, ARENA_1);
    tmps->len = len;

    memcpy(tmps->str, str, len);

    tmps->str[len] = '\0';

    tmps->next = buckets[hash];

    buckets[hash] = tmps;

    return tmps->str;
}

const char *ContainsIStr(const char *str, size_t len)
{
    unsigned    hash = 0;
    const char *end = str;
    CString    *tmps;

    assert(str && len);

    for(size_t i = 0; i < len; i++)
        hash = (hash << 1) + table[*(unsigned char*)end++ & UPPER];

    hash &= 1023;

    for(tmps = buckets[hash]; tmps; tmps = tmps->next){
        const char *s1, *s2;

        if(tmps->len != len)
            continue;

        s1 = str;
        s2 = tmps->str;

        do
            if(s1 == end)
                return tmps->str;
        while((*s1++ & 0xDF) == (*s2++ & 0xDF));
    }

    return NULL;
}

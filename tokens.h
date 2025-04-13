#pragma once

#define KEYWORD  0x10000000
#define INTERNAL 0x20000000
#define PAIR     0x00010000
#define HIGH     0x00100000
#define LOW      0x01000000

#define GET_PAIR 16
#define GET_HIGH 20
#define GET_LOW  24

#define TK_ERROR  -1
#define TK_EOF     0 
#define TK_NUM     1
#define TK_STR     2 
#define TK_SHL     3 
#define TK_SHR     4
#define TK_LABEL   5
#define TK_GL_LB   6 // global label
#define TK_ID      7

#define KW_AX       (PAIR + KEYWORD + 0)
#define KW_CX       (PAIR + KEYWORD + 1)
#define KW_DX       (PAIR + KEYWORD + 2)
#define KW_BX       (PAIR + KEYWORD + 3)

#define KW_AL       (LOW + KEYWORD +  0)
#define KW_CL       (LOW + KEYWORD +  1)
#define KW_DL       (LOW + KEYWORD +  2)
#define KW_BL       (LOW + KEYWORD +  3)

#define KW_AH       (HIGH + KEYWORD + 4)
#define KW_CH       (HIGH + KEYWORD + 5)
#define KW_DH       (HIGH + KEYWORD + 6)
#define KW_BH       (HIGH + KEYWORD + 7)


#define KW_MOV      (KEYWORD + 12)
#define KW_INT      (KEYWORD + 13)
#define KW_XOR      (KEYWORD + 14)
#define KW_AND      (KEYWORD + 15)
#define KW_ORG      (KEYWORD + 16)
#define KW_ADD      (KEYWORD + 17)
#define KW_RET      (KEYWORD + 18)

#define KW_TRACE    (INTERNAL + 0)
#define KW_NOWARNS  (INTERNAL + 1)

#define AX 0
#define CX 1
#define DX 2
#define BX 3

#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7
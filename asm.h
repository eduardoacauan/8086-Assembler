#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <stdarg.h>
#include "tokens.h"

/********************************
* ARENA 1 --> GENERAL           *
* ARENA 2 --> INSTRUCTIONS      *
* ARENA 3 --> FILE BUFFER       *
* ARENA 4 --> 8086 MACHINE CODE *
*********************************/
// Flags:   2 bytes
// [ 0000 0000 | 0000 0000 ]
//     A1/A2      general flags
//*******************************
#define ARENA_1     0
#define ARENA_2     1
#define ARENA_3     2
#define ARENA_4     3
#define ARENA_SIZE (5 * 1024)
#define UPPER       0xDF
#define INVALID     0
#define LETTER      1
#define DIGIT       2
#define HEX         4
#define OTHER       8
#define BLANK       16
#define NEWLINE     32
#define STROPEN     64
#define CHROPEN     128

#define ASMCTRL_ERROR         0
#define ASMCTRL_DSBL_WARNINGS 1
#define ASMCTRL_SHOW_OUTPUT   2
#define ASMCTRL_LEX_SEMI      3
#define ASMCTRL_VARIABLE_LEN  4
#define ASMCTRL_NEAR_256      5

#define ASMCTRL_A2_MEMORY    8
#define ASMCTRL_A2_IMMEDIATE 9
#define ASMCTRL_A2_REGISTER  10
#define ASMCTRL_A1_REGISTER  12
#define ASMCTRL_A1_MEMORY    13

// if ASMCTRL_A1_REGISTER + ASMCTRL_A2_IMMEDIATE -> (register: ax), ip += 3 (0xB8 0x00 0x00)
// if ASMCTRL_A1_REGISTER + ASMCTRL_A2_REGISTER  ->                 ip += 2 (0x89 0x00(byte for reg))

typedef struct CFile        CFile;
typedef struct CInstruction CInstruction;
typedef struct CInsArg      CInsArg;
typedef struct CLabel       CLabel;
typedef struct CHashTable   CHashTable;
typedef struct CAsmCtrl     CAsmCtrl;
typedef struct CStack       CStack;

typedef enum   Instruction  Instruction;
typedef enum   InsArg       InsArg;

enum Instruction {
    INS_INOP = -1,
    INS_MOV,
    INS_ADD,
    INS_XOR,
    INS_INT,
    INS_DB,
    INS_DW,
    INS_SHL,
    INS_SHR,
    INS_IADD,
    INS_ISUB,
    INS_IMUL,
    INS_IDIV,
    INS_ISHL,
    INS_ISHR,
    INS_IAND,
    INS_IXOR,
    INS_IOR,
    INS_OR,
    INS_REGISTER,
    INS_IMMEDIATE,
    INS_FWD_REF, // forward reference
    INS_RET,
    MAX
};

enum InsArg {
    REGISTER,
    IMMEDIATE,
    IDENTIFIER,
    MEMORY
};

union align {
	char*  p;
	long   i;
	double d;
	int   (*f)(void);
};

typedef uint8_t  byte;
typedef uint16_t word;

struct CFile {
    char*    path;
    char*    src;
    size_t   fileName_length;
    size_t   line;
    size_t   fsize;
    size_t   warns;
    size_t   errors;
    CFile*   prev;
};

struct CLabel {
    const char* lb_name;
    word        lb_addr;
    CLabel*     lb_next;
};

struct CStack {
    CInstruction* area[0xFF];
    int           ptr;
};

struct CInsArg {
    InsArg type;
    union {
        int         value;
        const char* str;
        CLabel*     lb;
    };
};

struct CInstruction {
    Instruction   type;
    CInsArg       arg1;
    CInsArg       arg2;
    size_t        line;
    size_t        ins_size;
    CInstruction* prev;
    CInstruction* next;
};

struct CAsmCtrl {
    CHashTable*   labels;
    CHashTable*   gl_labels;
    CHashTable*   keywords;
    CFile        *file;
    int           token;
    int           flags;
    size_t        ip;
    size_t        org;
    size_t        r1;
    size_t        r2;
    CInstruction* first;
    CInstruction* last;
    CStack       *stack;
    CLabel       *first_lb;
    CLabel       *last_lb;
    byte          opPrec[0x7F];
    byte         *machine_code;
    union {
        char* str;
        int   val;
    };
};

extern const char* ins_name[MAX];

extern void*       Alloc(size_t size, size_t idx);
extern void        AFree(void);

extern void        NewTable(CHashTable** ht);
extern void        Insert(CHashTable* ht, const char *key, void *item);
extern void*       Get(CHashTable* ht, const char *key);

extern const char* StringN(const char* str, size_t len);
extern const char* IString(const char* str);
extern const char* String(const char* str);

extern CFile*      NewFile(const char* path, int check_ext);
extern void        PushFile(CFile **file, const char *path);
extern void        PopFile(CFile **file);
extern CAsmCtrl*   NewAsmControl(const char* path);

extern int         IStrCmp(const char* s1, const char* s2);
extern void        AssembleFile(const char* path);
extern const char* ContainsIStr(const char* str, size_t len);
extern int         Lex(CAsmCtrl *ac);

extern int         Bt(int  flag, int pos);
extern void        Bs(int* flag, int pos);
extern void        Bo(int* flag, int pos);

extern void        Error(CAsmCtrl* ac, size_t line, const char* msg, ...);
extern void        Warn(CAsmCtrl* ac, size_t line, const char* msg, ...);

extern void        ParseFile(CAsmCtrl *ac);
extern void        Expect(CAsmCtrl *ac, int token);
extern CLabel     *NewLabel(CAsmCtrl *ac);

extern void         AddIns(CAsmCtrl *ac, CInstruction *tmpi);
extern CInstruction* NewIns(CAsmCtrl* ac, Instruction type, CInsArg* a1, CInsArg* a2);

extern void         PrsExpr(CAsmCtrl *ac, int prec);
extern void         Print(CAsmCtrl *ac);

extern void          FoldInstructions(CAsmCtrl *ac);

extern void          Push(CAsmCtrl *ac, CInstruction *ins);
extern CInstruction *Pop(CAsmCtrl *ac);

extern void          Gen8086(CAsmCtrl *ac);
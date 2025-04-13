#include "asm.h"

static int LexHex(CAsmCtrl* ac);

static unsigned char table[] = {
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ BLANK,
    /*'0'*/ NEWLINE,
    /*'0'*/ INVALID,
    /*'0'*/ BLANK,
    /*'0'*/ BLANK,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ INVALID,
    /*'0'*/ BLANK,
    /*'!'*/ OTHER,
    /*'"'*/ OTHER,
    /*'#'*/ OTHER,
    /*'$'*/ OTHER,
    /*'%'*/ OTHER,
    /*'&'*/ OTHER,
    /*'''*/ OTHER,
    /*'('*/ OTHER,
    /*')'*/ OTHER,
    /*'*'*/ OTHER,
    /*'+'*/ OTHER,
    /*','*/ OTHER,
    /*'-'*/ OTHER,
    /*'.'*/ OTHER,
    /*'/'*/ OTHER,
    /*'0'*/ DIGIT,
    /*'1'*/ DIGIT,
    /*'2'*/ DIGIT,
    /*'3'*/ DIGIT,
    /*'4'*/ DIGIT,
    /*'5'*/ DIGIT,
    /*'6'*/ DIGIT,
    /*'7'*/ DIGIT,
    /*'8'*/ DIGIT,
    /*'9'*/ DIGIT,
    /*':'*/ OTHER,
    /*';'*/ OTHER,
    /*'<'*/ OTHER,
    /*'='*/ OTHER,
    /*'>'*/ OTHER,
    /*'?'*/ OTHER,
    /*'@'*/ INVALID,
    /*'A'*/ LETTER | HEX,
    /*'B'*/ LETTER | HEX,
    /*'C'*/ LETTER | HEX,
    /*'D'*/ LETTER | HEX,
    /*'E'*/ LETTER | HEX,
    /*'F'*/ LETTER | HEX,
    /*'G'*/ LETTER,
    /*'H'*/ LETTER,
    /*'I'*/ LETTER,
    /*'J'*/ LETTER,
    /*'K'*/ LETTER,
    /*'L'*/ LETTER,
    /*'M'*/ LETTER,
    /*'N'*/ LETTER,
    /*'O'*/ LETTER,
    /*'P'*/ LETTER,
    /*'Q'*/ LETTER,
    /*'R'*/ LETTER,
    /*'S'*/ LETTER,
    /*'T'*/ LETTER,
    /*'U'*/ LETTER,
    /*'V'*/ LETTER,
    /*'W'*/ LETTER,
    /*'X'*/ LETTER,
    /*'Y'*/ LETTER,
    /*'Z'*/ LETTER,
    /*'['*/ OTHER,
    /*'\'*/ OTHER,
    /*']'*/ OTHER,
    /*'^'*/ OTHER,
    /*'_'*/ LETTER,
    /*'`'*/ INVALID,
    /*'a'*/ LETTER | HEX,
    /*'b'*/ LETTER | HEX,
    /*'c'*/ LETTER | HEX,
    /*'d'*/ LETTER | HEX,
    /*'e'*/ LETTER | HEX,
    /*'f'*/ LETTER | HEX,
    /*'g'*/ LETTER,
    /*'h'*/ LETTER,
    /*'i'*/ LETTER,
    /*'j'*/ LETTER,
    /*'k'*/ LETTER,
    /*'l'*/ LETTER,
    /*'m'*/ LETTER,
    /*'n'*/ LETTER,
    /*'o'*/ LETTER,
    /*'p'*/ LETTER,
    /*'q'*/ LETTER,
    /*'r'*/ LETTER,
    /*'s'*/ LETTER,
    /*'t'*/ LETTER,
    /*'u'*/ LETTER,
    /*'v'*/ LETTER,
    /*'w'*/ LETTER,
    /*'x'*/ LETTER,
    /*'y'*/ LETTER,
    /*'z'*/ LETTER,
    /*'{'*/ INVALID,
    /*'|'*/ OTHER,
    /*'}'*/ INVALID,
    /*'~'*/ OTHER
};

int Lex(CAsmCtrl* ac)
{
    if (!ac)
        return TK_ERROR;

    if (!*ac->file->src)
        return ac->token = TK_EOF;

    if (table[*ac->file->src] & INVALID) {
        Error(ac, 0, "Invalid token '%c'", *ac->file->src++);
        return ac->token = TK_ERROR;
    }

    while (1) {
        while (table[*ac->file->src] & BLANK)
            ac->file->src++;
        switch (*ac->file->src++) {
            case 0:
                ac->file->src--;
                return ac->token = TK_EOF;
            case ';':
                if (Bt(ac->flags, ASMCTRL_LEX_SEMI)) {
                    Bo(&ac->flags, ASMCTRL_LEX_SEMI);
                    return ac->token = ';';
                }
                while (*ac->file->src && table[*ac->file->src] ^ NEWLINE)
                    ac->file->src++;
                continue;
            case '\n':
                ac->file->line++;
                continue;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                ac->file->src--;
                ac->val = 0;

                if (*ac->file->src == '0' && (ac->file->src[1] & UPPER) == 'X')
                    return LexHex(ac);

                do {
                    ac->val = ac->val * 10 + (*ac->file->src++ & 0xF);
                } while (*ac->file->src && table[*ac->file->src] & DIGIT);

                return ac->token = TK_NUM;
            case '+':
            case '-':
            case '*':
            case '&':
            case '^':
            case '|':
            case '~':
            case '(':
            case ')':
            case '[':
            case ']':
            case ',':
            case '$':
                return ac->token = *(ac->file->src - 1);
            case '<':
                if (*ac->file->src == '<') {
                    ac->file->src++;
                    return ac->token = TK_SHL;
                }
                return ac->token = '<';
            case '>':
                if (*ac->file->src == '>') {
                    ac->file->src++;
                    return ac->token = TK_SHR;
                }
                return ac->token = '>';
            case '_':
            case 'a':
            case 'A':
            case 'b':
            case 'B':
            case 'c':
            case 'C':
            case 'd':
            case 'D':
            case 'e':
            case 'E':
            case 'f':
            case 'F':
            case 'g':
            case 'G':
            case 'h':
            case 'H':
            case 'i':
            case 'I':
            case 'j':
            case 'J':
            case 'k':
            case 'K':
            case 'l':
            case 'L':
            case 'm':
            case 'M':
            case 'n':
            case 'N':
            case 'o':
            case 'O':
            case 'p':
            case 'P':
            case 'q':
            case 'Q':
            case 'r':
            case 'R':
            case 's':
            case 'S':
            case 't':
            case 'T':
            case 'u':
            case 'U':
            case 'v':
            case 'V':
            case 'w':
            case 'W':
            case 'x':
            case 'X':
            case 'y':
            case 'Y':
            case 'z':
            case 'Z':
                ac->file->src--;
                ac->str = ac->file->src;

                while (*ac->file->src && table[*ac->file->src] & (LETTER + DIGIT))
                    ac->file->src++;

                if (ac->token = Get(ac->keywords, ContainsIStr(ac->str, ac->file->src - ac->str)))
                    return ac->token;

                ac->str = StringN(ac->str, ac->file->src - ac->str);

                if (*ac->file->src == ':') {
                    ac->file->src++;
                    if (*ac->file->src == ':')
                        return ac->token = TK_GL_LB;
                    return ac->token = TK_LABEL;
                }
                return ac->token = TK_ID;
        }
    }
}

static int LexHex(CAsmCtrl *ac)
{
    if (!ac)
        return TK_ERROR;

    ac->file->src += 2;

    do {
        if (table[*ac->file->src] & DIGIT) {
            ac->val = (ac->val << 4) | (*ac->file->src++ & 0xF);
            continue;
        }

        if (table[*ac->file->src] & HEX) {
            ac->val = (ac->val << 4) | ((*ac->file->src++ & UPPER) - 'A' + 10);
            continue;
        }

        Error(ac, 0, "Invalid hex digit '%c'", *ac->file->src++);
    } while (*ac->file->src && table[*ac->file->src] & (LETTER + DIGIT));

    return ac->token = TK_NUM;
}
#include "asm.h"

static void PrsPrefix(CAsmCtrl* ac);

static int  TokenToInsType(CAsmCtrl *ac, int token);

void PrsExpr(CAsmCtrl* ac, int prec)
{
	if (!ac)
		return;

	PrsPrefix(ac);

	while (prec < ac->opPrec[ac->token & 0x7F]) {
		int op = ac->token;

		Lex(ac);

		PrsExpr(ac, ac->opPrec[op]);

		AddIns(ac, NewIns(ac, TokenToInsType(ac, op), NULL, NULL));
	}
}

static void PrsPrefix(CAsmCtrl* ac)
{
	CInsArg arg;

	if (!ac)
		return;

	switch (ac->token) {
		case TK_NUM:
			arg.value = ac->val;
			arg.type  = IMMEDIATE;
			if (Bt(ac->flags, ASMCTRL_VARIABLE_LEN) && arg.value > 0xFF)
				Bs(&ac->flags, ASMCTRL_NEAR_256);
			AddIns(ac, NewIns(ac, INS_IMMEDIATE, &arg, NULL));
			Lex(ac);
			return;
		case '(':
			Lex(ac);
			PrsExpr(ac, 0);
			Expect(ac, ')');
			Lex(ac);
			return;
		case TK_ID:
			if (arg.lb = Get(ac->labels, ac->str)) {
				arg.value = arg.lb->lb_addr;
				arg.type = IMMEDIATE;
				AddIns(ac, NewIns(ac, INS_IMMEDIATE, &arg, NULL));
				Lex(ac);
				return;
			}

			arg.str  = ac->str; // later
			arg.type = IDENTIFIER;

			AddIns(ac, NewIns(ac, INS_FWD_REF, &arg, NULL));
			Lex(ac);
			return;
		case '$':
			arg.value = (ac->ip + ac->org) & 0xFFFF;
			arg.type = IMMEDIATE;
			AddIns(ac, NewIns(ac, INS_IMMEDIATE, &arg, NULL));
			Lex(ac);
			return;
	}

	Error(ac, 0, "Invalid expression");
}

static int TokenToInsType(CAsmCtrl* ac, int token)
{
	if (!ac)
		return INS_IADD;

	switch (token) {
		case '+':
			return INS_IADD;
		case '-':
			return INS_ISUB;
		case '*':
			return INS_IMUL;
		case '/':
			return INS_IDIV;
		case TK_SHL:
			return INS_SHL;
		case TK_SHR:
			return INS_SHR;
		case '&':
			return INS_IAND;
		case '^':
			return INS_IXOR;
		case '|':
			return INS_IOR;
	}

	Error(ac, 0, "Invalid operator '%c'", (char)token);
	return INS_IADD;
}
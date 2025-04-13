#include "asm.h"

static void PrsIns(CAsmCtrl *ac);

static void PrsMnemonicA(CAsmCtrl* ac, Instruction type);
static void PrsOperand1(CAsmCtrl *ac);

static void ParseRegister(CAsmCtrl* ac, CInstruction *tmpi);
static void PrsOperand2(CAsmCtrl *ac);

static void CalculateIP(CAsmCtrl* ac)
{
	if (!ac)
		return;

	switch (ac->flags & 0xFF00) {
		case (1 << ASMCTRL_A1_REGISTER) + (1 << ASMCTRL_A2_IMMEDIATE):
			if ((ac->r1 >> GET_PAIR) & 1) {
				if (Bt(ac->flags, ASMCTRL_VARIABLE_LEN) && Bt(ac->flags, ASMCTRL_NEAR_256)) {
					ac->ip += 2;
					Bo(&ac->flags, ASMCTRL_NEAR_256);
					break;
				}
				ac->ip += 3;
				break;
			}
			ac->ip += 2;
			break;
		case (1 << ASMCTRL_A1_REGISTER) + (1 << ASMCTRL_A2_REGISTER) : {
			size_t r1_size = (ac->r1 >> GET_PAIR) & 1 ? 16 : 8;
			size_t r2_size = (ac->r2 >> GET_PAIR) & 1 ? 16 : 8;

			if (r1_size != r2_size)
				Error(ac, 0, "Register size mismatch");
			ac->ip += 2; // 89 00
			break;
		}
	}

	ac->flags &= 0x00FF;
}

void ParseFile(CAsmCtrl *ac)
{
	if (!ac)
		return;

	Lex(ac);

	while (ac->token != TK_EOF)
		PrsIns(ac);
}

static void PrsIns(CAsmCtrl* ac)
{
	if (!ac)
		return;

	switch (ac->token) {
		case KW_TRACE:
			Bs(&ac->flags, ASMCTRL_SHOW_OUTPUT);
			Bs(&ac->flags, ASMCTRL_LEX_SEMI);
			Lex(ac);
			Expect(ac, ';');
			Lex(ac);
			return;
		case TK_LABEL: {
			CLabel* lb = NewLabel(ac);

			Insert(ac->labels, ac->str, lb);

			Lex(ac);
			return;
		}
		case KW_XOR:
			PrsMnemonicA(ac, INS_XOR);
			return;
		case KW_MOV:
			PrsMnemonicA(ac, INS_MOV);
			return;
		case KW_ADD:
			Bs(&ac->flags, ASMCTRL_VARIABLE_LEN);
			PrsMnemonicA(ac, INS_ADD);
			Bo(&ac->flags, ASMCTRL_VARIABLE_LEN);
			return;
		case KW_INT:
			Lex(ac);
			PrsExpr(ac, 0);
			AddIns(ac, NewIns(ac, INS_INT, NULL, NULL));
			ac->ip+=2;
			return;
		case KW_ORG:
			Lex(ac);
			Expect(ac, TK_NUM);
			ac->org = ac->val & 0xFFFF;
			Lex(ac);
			return;
		case KW_NOWARNS:
			Bs(&ac->flags, ASMCTRL_DSBL_WARNINGS);
			Bs(&ac->flags, ASMCTRL_LEX_SEMI);
			Lex(ac);
			Expect(ac, ';');
			Lex(ac);
			return;
		case KW_RET:
			AddIns(ac, NewIns(ac, INS_RET, NULL, NULL));
			ac->ip++;
			Lex(ac);
			return;
		default:
			Error(ac, 0, "Invalid instruction");
			Lex(ac);
			return;
	}
}

static void PrsMnemonicA(CAsmCtrl* ac, Instruction type)
{
	if (!ac)
		return;

	Lex(ac);

	PrsOperand1(ac);

	Expect(ac, ',');

	Lex(ac);

	PrsOperand2(ac);

	AddIns(ac, NewIns(ac, type, NULL, NULL));

	CalculateIP(ac);
}

static void PrsOperand1(CAsmCtrl* ac)
{
	CInstruction* tmpi;

	if (!ac)
		return;

	tmpi = (CInstruction*)Alloc(sizeof(CInstruction), ARENA_2);

	memset(tmpi, 0, sizeof(CInstruction));

	tmpi->line = ac->file->line;

	if (ac->token >= KW_AX && ac->token <= KW_BL) {
		ac->r1 = ac->token;
		ParseRegister(ac, tmpi);
		Bs(&ac->flags, ASMCTRL_A1_REGISTER);
		return;
	}

	Error(ac, 0, "Invalid operand");
	Lex(ac);
}

static void PrsOperand2(CAsmCtrl* ac)
{
	CInstruction* tmpi;

	if (!ac)
		return;

	tmpi = (CInstruction*)Alloc(sizeof(CInstruction), ARENA_2);

	memset(tmpi, 0, sizeof(CInstruction));

	tmpi->line = ac->file->line;

	if (ac->token >= KW_AX && ac->token <= KW_BL) {
		ac->r2 = ac->token;
		ParseRegister(ac, tmpi);
		Bs(&ac->flags, ASMCTRL_A2_REGISTER);
		return;
	}

	switch (ac->token) {
		case TK_NUM:
		case '(':
		case TK_ID:
		case '$':
			PrsExpr(ac, 0);
			Bs(&ac->flags, ASMCTRL_A2_IMMEDIATE);
			return;
	}

	Error(ac, 0, "Invalid operand");
	Lex(ac);
}

static void ParseRegister(CAsmCtrl* ac, CInstruction *tmpi)
{
	tmpi->type = INS_REGISTER;
	tmpi->arg1.type = REGISTER;
	tmpi->arg1.value = ac->token;
	AddIns(ac, tmpi);
	Lex(ac);
	return;
}
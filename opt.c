#include "asm.h"

static void ConstantFold(CAsmCtrl *ac, CInstruction *op);
static void SetNOP(CAsmCtrl *ac, CInstruction *ins);

static void FoldIns2Args(CAsmCtrl *ac, CInstruction *ins);
static void FoldIns1Arg(CAsmCtrl *ac, CInstruction *ins);

void FoldInstructions(CAsmCtrl* ac)
{
	CInstruction* first;

	if (!ac)
		return;

	first = ac->first;

	while (first) {
		switch (first->type) {
			case INS_FWD_REF: {
				const char* str = first->arg1.str;
				Push(ac, first);
				if (!(first->arg1.lb = Get(ac->labels, str))) {
					Error(ac, first->line, "Unresolved reference to label '%s'", str);
					break;
				}
				first->arg1.value = first->arg1.lb->lb_addr;
				first->arg1.type  = INS_IMMEDIATE;
				first->type       = INS_IMMEDIATE;
				break;
			}
			case INS_IMMEDIATE:
			case INS_REGISTER:
				Push(ac, first);
				break;
			case INS_IADD:
			case INS_IAND:
			case INS_IXOR:
			case INS_ISUB:
			case INS_IMUL:
			case INS_IDIV:
			case INS_IOR:
				ConstantFold(ac, first);
				break;
			case INS_MOV:
			case INS_XOR:
			case INS_ADD:
				FoldIns2Args(ac, first);
				break;
			case INS_INT:
				FoldIns1Arg(ac, first);
				break;
		}
		first = first->next;
	}
}

static void FoldIns2Args(CAsmCtrl* ac, CInstruction* ins)
{
	CInstruction* a1, * a2;

	if (!ac || !ins)
		return;

	a2 = Pop(ac);
	a1 = Pop(ac);

	ins->arg1 = a1->arg1;
	ins->arg2 = a2->arg1;

	SetNOP(ac, a1);
	SetNOP(ac, a2);
}

static void ConstantFold(CAsmCtrl* ac, CInstruction* op)
{
	CInstruction* a1, * a2;

	if (!ac || !op)
		return;

	a2 = Pop(ac);
	a1 = Pop(ac);

	switch (op->type) {
		case INS_IADD:
			a1->arg1.value += a2->arg1.value;
			break;
		case INS_ISUB:
			a1->arg1.value -= a2->arg1.value;
			break;
		case INS_IMUL:
			a1->arg1.value *= a2->arg1.value;
			break;
		case INS_ISHL:
			a1->arg1.value <<= a2->arg1.value;
			break;
		case INS_ISHR:
			a1->arg1.value >>= a2->arg1.value;
			break;
		case INS_IDIV:
			if (!a2->arg1.value) {
				Error(ac, a2->line, "Division by zero");
				a2->arg1.value = 1;
			}
			a1->arg1.value /= a2->arg1.value;
			break;
	}

	Push(ac,   a1);
	SetNOP(ac, a2);
	SetNOP(ac, op);
}

static void SetNOP(CAsmCtrl* ac, CInstruction* ins)
{
	if (!ac || !ins)
		return;

	if(ins->prev)
		ins->prev->next = ins->next;
	ins->next->prev = ins->prev;

	ins->type = INS_INOP;
}

static void FoldIns1Arg(CAsmCtrl* ac, CInstruction* ins)
{
	CInstruction* a1;

	if (!ac || !ins)
		return;

	a1 = Pop(ac);

	ins->arg1 = a1->arg1;

	SetNOP(ac, a1);
}
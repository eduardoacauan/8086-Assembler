#include "asm.h"

const char* ins_name[] = {
	{"MOV\t"},           {"ADD\t"},   {"XOR\t"},    {"INT\t"},
	{"BYTE\t"},          {"WORD\t"},  {"SHL\t"},    {"SHR\t"},
	{"IADD\t"},          {"ISUB\t"},  {"IMUL\t"},   {"IDIV\t"},
	{"ISHL\t"},          {"ISHR\t"},  {"IAND\t"},   {"IXOR\t"},
	{"IOR\t"},           {"OR\t"},    {"REGISTER"}, {"IMMEDIATE"},
	{"FORWARD_REF\b\b"}, {"RET\t"}
};

static void FillKeywords(CAsmCtrl* ac);
static void PrintIns(CAsmCtrl *ac, CInstruction *ins);
static void PrintLabels(CAsmCtrl *ac);
static void WriteToFile(CAsmCtrl *ac);

__forceinline static void AssembledUnsuccessfully(CAsmCtrl* ac);
__forceinline static void Assembledsuccessfully(CAsmCtrl* ac);

static void WriteToFile(CAsmCtrl* ac)
{
	FILE* file;

	if (!ac)
		return;

	*(int*)&ac->file->path[ac->file->fileName_length - 4] = 'nib.';

	file = fopen(ac->file->path, "wb");

	if (!file) {
		fprintf(stderr, "Error writing machine code to file '%s'\n", ac->file->path);
		return;
	}

	fwrite(ac->machine_code, sizeof(byte), ac->ip, file);

	fclose(file);
}

int IStrCmp(const char* s1, const char* s2)
{
	if (!s1 || !s2)
		return 0;

	while (*s1 && *s2) {
		if ((*s1++ & UPPER) != (*s2++ & UPPER))
			return 0;
	}

	return !*s1 && !*s2;
}

CAsmCtrl* NewAsmControl(const char* path)
{
	CAsmCtrl* ac;

	if (!path)
		return NULL;

	ac = (CAsmCtrl*)Alloc(sizeof(CAsmCtrl), ARENA_1);

	memset(ac, 0, sizeof(CAsmCtrl));

	ac->file = NewFile(path, 1);

	if (!ac->file)
			return NULL;

	ac->stack = (CStack*)Alloc(sizeof(CStack), ARENA_1);

	memset(ac->stack,  0, sizeof(CStack));
	memset(ac->opPrec, 0, sizeof(ac->opPrec));

	NewTable(&ac->labels);
	NewTable(&ac->gl_labels);
	NewTable(&ac->keywords);

	ac->opPrec['+']    = 10;
	ac->opPrec['-']    = 10;
	ac->opPrec['*']    = 15;
	ac->opPrec['/']    = 15;
	ac->opPrec['&']    = 15;
	ac->opPrec['|']    = 10;
	ac->opPrec[TK_SHL] = 15;
	ac->opPrec[TK_SHR] = 15;

	FillKeywords(ac);

	return ac;
}

void AssembleFile(const char* path)
{
	CAsmCtrl* ac;

	ac = NewAsmControl(path);

	if (!ac)
		return;

	ParseFile(ac);

	if (Bt(ac->flags, ASMCTRL_ERROR)) {
		AssembledUnsuccessfully(ac);
		return;
	}

	Print(ac);
	FoldInstructions(ac);
	if (Bt(ac->flags, ASMCTRL_ERROR)) {
		AssembledUnsuccessfully(ac);
		return;
	}

	Print(ac);
	Gen8086(ac);
	Assembledsuccessfully(ac);
	WriteToFile(ac);
}

int Bt(int flag, int pos)
{
	return (flag >> pos) & 1;
}

void Bs(int *flag, int pos)
{
	if (!flag)
		return;
	*flag |= (1 << pos);
}

void Bo(int *flag, int pos)
{
	if (!flag)
		return;
	*flag &= ~(1 << pos);
}

void Error(CAsmCtrl* ac, size_t line, const char* msg, ...)
{
	va_list ap;
	size_t  tmpl;

	if (!ac)
		return;

	if (!line)
		tmpl = ac->file->line;
	else
		tmpl = line;

	va_start(ap, msg);
	printf("Error(%s, %zd): ", ac->file->path, tmpl);
	vprintf(msg, ap);

	va_end(ap);

	ac->file->errors++;

	printf("\n");

	Bs(&ac->flags, ASMCTRL_ERROR);
}

void Warn(CAsmCtrl* ac, size_t line, const char* msg, ...)
{
	va_list ap;
	size_t  tmpl;

	if (!ac)
		return;

	if (!line)
		tmpl = ac->file->line;
	else
		tmpl = line;

	va_start(ap, msg);
	printf("Warning(%s, %zd): ", ac->file->path, tmpl);
	vprintf(msg, ap);

	va_end(ap);

	ac->file->warns++;

	printf("\n");
}

static void FillKeywords(CAsmCtrl* ac)
{
	if (!ac)
		return;

	Insert(ac->keywords, String("MOV"),     (void*)KW_MOV);
	Insert(ac->keywords, String("XOR"),     (void*)KW_XOR);
	Insert(ac->keywords, String("AND"),     (void*)KW_AND);
	Insert(ac->keywords, String("INT"),     (void*)KW_INT);
	Insert(ac->keywords, String("TRACE"),   (void*)KW_TRACE);
	Insert(ac->keywords, String("NOWARNS"), (void*)KW_NOWARNS);
	Insert(ac->keywords, String("AX"),      (void*)KW_AX);
	Insert(ac->keywords, String("BX"),      (void*)KW_BX);
	Insert(ac->keywords, String("CX"),      (void*)KW_CX);
	Insert(ac->keywords, String("DX"),      (void*)KW_DX);
	Insert(ac->keywords, String("AH"),      (void*)KW_AH);
	Insert(ac->keywords, String("BH"),      (void*)KW_BH);
	Insert(ac->keywords, String("CH"),      (void*)KW_CH);
	Insert(ac->keywords, String("DH"),      (void*)KW_DH);
	Insert(ac->keywords, String("AL"),      (void*)KW_AL);
	Insert(ac->keywords, String("BL"),      (void*)KW_BL);
	Insert(ac->keywords, String("CL"),      (void*)KW_CL);
	Insert(ac->keywords, String("DL"),      (void*)KW_DL);
	Insert(ac->keywords, String("ORG"),     (void*)KW_ORG);
	Insert(ac->keywords, String("ADD"),     (void*)KW_ADD);
	Insert(ac->keywords, String("RET"),     (void*)KW_RET);
}

void Expect(CAsmCtrl* ac, int token)
{
	if (!ac || ac->token == token)
		return;

	if (token == TK_NUM) {
		Error(ac, 0, "Constant expected");
		return;
	}

	if (token == TK_ID) {
		Error(ac, 0, "Identifier expected");
		return;
	}

	Error(ac, 0, "'%c' expected", (char)token);
}

CLabel* NewLabel(CAsmCtrl* ac)
{
	CLabel* lb;

	if (!ac)
		return NULL;

	lb = (CLabel*)Alloc(sizeof(CLabel), ARENA_1);

	lb->lb_addr = (ac->ip + ac->org) & 0xFFFF;
	lb->lb_name = ac->str;
	lb->lb_next = NULL;
	
	if (!ac->first_lb) {
		ac->first_lb = lb;
		ac->last_lb  = lb;
		return lb;
	}

	ac->last_lb->lb_next = lb;
	ac->last_lb          = ac->last_lb->lb_next;

	return lb;
}

void AddIns(CAsmCtrl* ac, CInstruction* tmpi)
{
	if (!ac || !tmpi)
		return;

	if (!ac->first) {
		ac->first = tmpi;
		ac->last  = tmpi;
		return;
	}

	tmpi->prev     = ac->last;
	ac->last->next = tmpi;
	ac->last       = ac->last->next;
}

CInstruction* NewIns(CAsmCtrl* ac, Instruction type, CInsArg* a1, CInsArg* a2)
{
	CInstruction* tmpi;

	if (!ac)
		return NULL;

	tmpi = (CInstruction*)Alloc(sizeof(CInstruction), ARENA_2);

	tmpi->type     = type;
	tmpi->line     = ac->file->line;
	tmpi->next     = NULL;
	tmpi->prev     = NULL;
	tmpi->ins_size = 0;

	if (a1)
		tmpi->arg1 = *a1;
	else {
		tmpi->arg1.value = 0;
		tmpi->arg1.type  = 0;
	}

	if (a2)
		tmpi->arg2 = *a2;
	else {
		tmpi->arg2.value = 0;
		tmpi->arg2.type  = 0;
	}

	return tmpi;

}

void Print(CAsmCtrl* ac)
{
	CInstruction* first;
	static unsigned pass_count = 0;

	if (!ac || !Bt(ac->flags, ASMCTRL_SHOW_OUTPUT))
		return;

	printf("PASS %02X\n", pass_count++);

	first = ac->first;

	while (first) {
		PrintIns(ac, first);
		first = first->next;
	}

	if (pass_count != 1)
		return;

	PrintLabels(ac);
}

static void PrintLabels(CAsmCtrl* ac)
{
	CLabel* first;

	if (!ac || !ac->first_lb)
		return;
	
	first = ac->first_lb;
	printf("Labels: ");
	for (size_t j = 0; first; j++, first = first->lb_next) {
		if (j == 8) {
			j = 0;
			printf("\n");
		}
		printf("['%s', %08X]", first->lb_name, first->lb_addr);
		if (first->lb_next)
			printf(", ");
	}
	printf("\n");
}

static void PrintIns(CAsmCtrl* ac, CInstruction* ins)
{
	if (!ac || !ins || ins->type == INS_INOP)
		return;

	printf("\t\t%s\t\t%08X\n", ins_name[ins->type], ins->arg1.value);
}


void Push(CAsmCtrl* ac, CInstruction* ins)
{
	if (!ac || !ins || !ac->stack || ac->stack->ptr >= 0xFF)
		return;

	ac->stack->area[++ac->stack->ptr] = ins;
}

CInstruction* Pop(CAsmCtrl* ac)
{
	if (!ac || !ac->stack || ac->stack->ptr <= 0)
		return NULL;

	return ac->stack->area[ac->stack->ptr--];
}

__forceinline static void AssembledUnsuccessfully(CAsmCtrl* ac)
{
	printf("\nFile '%s' assembled unsucessfully\nErrors: %zd Warnings: %zd\nSize: %zd bytes", ac->file->path, ac->file->errors, ac->file->warns, ac->ip);
}

__forceinline static void Assembledsuccessfully(CAsmCtrl* ac)
{
	printf("\nFile '%s' assembled sucessfully\nErrors: %zd Warnings: %zd\nSize: %zd bytes", ac->file->path, ac->file->errors, ac->file->warns, ac->ip);
}
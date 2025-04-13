#include "asm.h"

static void w8(CAsmCtrl* ac, byte value);
static void w16(CAsmCtrl* ac, word value);

static void GenMOV(CAsmCtrl* ac, CInstruction* first);
static void GenADD(CAsmCtrl* ac, CInstruction* first);

void Gen8086(CAsmCtrl* ac)
{
	byte         * start;
	CInstruction * first;
	
	if (!ac)
		return;

	ac->machine_code = (byte*)Alloc(sizeof(byte) * ac->ip, ARENA_4);

	start = ac->machine_code;

	first = ac->first;

	while (first) {
		switch (first->type) {
			case INS_MOV:
				GenMOV(ac, first);
				break;
			case INS_ADD:
				GenADD(ac, first);
				break;
			case INS_INT:
				w8(ac, 0xCD);
				w8(ac, first->arg1.value & 0xFF);
				break;
			case INS_RET:
				w8(ac, 0xC3);
				break;
		}
		first = first->next;
	}

	ac->ip = ac->machine_code - start;
	ac->machine_code = start;
}

static void GenMOV(CAsmCtrl* ac, CInstruction* first)
{
	if (!ac || !first)
		return;

	switch (first->arg1.type + first->arg2.type) {
		case REGISTER + IMMEDIATE:
			if ((first->arg1.value >> GET_PAIR) & 1) {
				w8(ac, 0xB8 + (first->arg1.value & 0x7));
				w16(ac, first->arg2.value & 0xFFFF);
				return;
			}
			w8(ac, 0xB0 + (first->arg1.value & 0x7));
			w8(ac, first->arg2.value & 0xFF);
			return;
		case REGISTER + REGISTER: {
			byte reg_prefix = ((first->arg1.value >> GET_PAIR) & 1) ? 0x89 : 0x88;

			w8(ac, reg_prefix);
			w8(ac, (0x03 << 6) | ((first->arg2.value & 0x7) << 3) | (first->arg1.value & 0xF));
			return;
		}
	}
}

static void GenADD(CAsmCtrl* ac, CInstruction* first)
{	
	if (!ac || !first)
		return;

	switch (first->arg1.type + first->arg2.type) {
		case REGISTER + IMMEDIATE: {
			byte modrm;
			modrm = (0x03 << 6) | (0x00 << 3) | (first->arg1.value & 0x7);
			if ((first->arg1.value >> GET_PAIR) & 1) {

				if ((first->arg1.value & 7) == AX && first->arg2.value > 0xFF) {
					w8(ac, 0x05);
					w16(ac, first->arg2.value & 0xFFFF);
					return;
				}

				if (first->arg2.value <= 0xFF) {
					w8(ac, 0x83);
					w8(ac, modrm);
					w8(ac, first->arg2.value & 0xFF);
					return;
				}
				w8(ac, 0x81);
				w8(ac, modrm);
				w16(ac, first->arg2.value & 0xFFFF);
				return;
			}

			if ((first->arg1.value & 7) == AL) {
				w8(ac, 0x04);
				w8(ac, first->arg2.value & 0xFF);
				return;
			}
			w8(ac, 0x80);
			w8(ac, modrm);
			w8(ac, first->arg2.value & 0xFF);
			return;
		}
		case REGISTER + REGISTER: {
			byte reg_prefix = ((first->arg1.value >> GET_PAIR) & 1) ? 0x01 : 0x00;

			w8(ac, reg_prefix);
			w8(ac, (0x03 << 6) | ((first->arg2.value & 0x7) << 3) | (first->arg1.value & 0xF));
			return;
		}
	}
}

static void w8(CAsmCtrl* ac, byte value)
{
	if (!ac || !ac->machine_code)
		return;

	*ac->machine_code++ = value;
}

static void w16(CAsmCtrl* ac, word value)
{
	if (!ac || !ac->machine_code)
		return;

	*ac->machine_code++ = value & 0xFF;
	*ac->machine_code++ = (value >> 8) & 0xFF;
}
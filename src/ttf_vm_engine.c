#ifndef TTF_VM_ENGINE
#define TTF_VM_ENGINE
#define ENUM_RANGE2(token, num) token##_0 = num, token##_1
#define ENUM_RANGE4(token, num) token##_0 = num, token##_1, token##_2, token##_3

#define ENUM_NAME(enm) [enm] = #enm
#define ENUM_NAME2(enm) [enm##_0] = #enm"_0", [enm##_1] = #enm"_1"

#define TTF_OUT_DEBUG
#define STACK_SIZE 1024
#define TTF_DISASSEMBLE

typedef enum opcode_type {
	ENUM_RANGE2(SVTCA, 0x00), //0x0, 0x1
	ENUM_RANGE2(SPVTCA, 0x02), // 0x2 0x3
	ENUM_RANGE2(SFVTCA, 0x04), // 0x4 0x5

	ENUM_RANGE2(SPVTL, 0x06), // 0x6 0x7
	ENUM_RANGE2(SFVTL, 0x08), // 0x6 0x7
	SPVFS,
	SFVFS,
	GPV,
	GFV,
	SFVTPV,
	ISECT,
	SRP0,
	SRP1,
	SRP2,
	SZP0,
	SZP1,
	SZP2,
	SZPS,
	SLOOP,
	RTG,
	RTHG,
	SMD,
	ELSE,
	JMPR,
	SCVTCI,
	SSWCI,
	SSW,
	DUP = 0x20,
	POP,
	CLEAR,
	SWAP,
	DEPTH,
	CINDEX,
	MINDEX,
	ALIGNPTS,
	UTP = 0x29,
	LOOPCALL,
	CALL,
	FDEF,
	ENDF,
	ENUM_RANGE2(MDAP, 0x2E),

	ENUM_RANGE2(IUP, 0x30),
	ENUM_RANGE2(SHP, 0x32),
	ENUM_RANGE2(SHC, 0x34),
	ENUM_RANGE2(SHZ, 0x36),
	SHPIX,

	IP = 0x39,
	ENUM_RANGE2(MSIRP, 0x3A),
	ALIGNRP = 0x3C,
	RTDG = 0x3D,
	ENUM_RANGE2(MIAP, 0x3E),

	PUSH_N_BYTES,
	PUSH_N_WORDS,
	WS,
	RS,
	WCVTP,
	RCVT,

	ENUM_RANGE2(GC, 0x46),
	SCFS,
	ENUM_RANGE2(MD, 0x49),
	MPPEM,
	MPS,
	FLIPON,
	FLIPOFF,
	TTF_DEBUG = 0x4F,
	
	LT = 0x50,
	LTEQ,
	GT,
	GTEQ,
	EQ,
	NEQ,
	ODD,
	EVEN,
	IF = 0x58,
	EIF,
	AND = 0x5A,
	OR,
	NOT,
	DELTAP1,
	SDB,
	SDS,

	ADD = 0x60,
	SUB,
	DIV,
	MUL,
	ABS,
	NEG,
	FLOOR,
	CELING,
	ROUND_0 = 0x68,
	ROUND_1 = 0x69,
	ROUND_2 = 0x6A,
	ROUND_3 = 0x6B,

	NROUND_0 = 0x6C,
	NROUND_1 = 0x6D,
	NROUND_2 = 0x6E,
	NROUND_3 = 0x6F,

	DELTAP2 = 0x71,
	DELTAP3,
	DELTAC1,
	DELTAC2,
	DELTAC3,
	SROUND,
	S45ROUND,
	JROT,
	JROF,
	ROFF,
	RUDG = 0x7C,
	RDTG = 0x7D,
	SANGW = 0x7E,


	FLIPPT = 0x80,
	FLIPRGON,
	FLIPRGOFF,
	SCANCTRL = 0x85,
	ENUM_RANGE2(SDPVTL, 0x86),
	GETINFO,
	IDEF = 0x89,
	ROLL = 0x8A,
	MAX,
	MIN,
	SCANTYPE = 0x8D,
	INSTCTRL = 0x8E,

	GETVARIATION = 0x91,

	// PUSH Byte
	PUSHB_0     = 0xB0,
	PUSHB_1     = 0xB1,
	PUSHB_2     = 0xB2,
	PUSHB_3     = 0xB3,
	PUSHB_4     = 0xB4,
	PUSHB_5     = 0xB5,
	PUSHB_6     = 0xB6,
	PUSHB_7     = 0xB7,

	// Push Word
	PUSHW_8     = 0xB8,
	PUSHW_9     = 0xB9,
	PUSHW_A     = 0xBA,
	PUSHW_B     = 0xBB,
	PUSHW_C     = 0xBC,
	PUSHW_D     = 0xBD,
	PUSHW_E     = 0xBE,
	PUSHW_F     = 0xBF,

	MDRP_C0 = 0xC0,
	MDRP_C1 = 0xC1,
	MDRP_C2 = 0xC2,
	MDRP_C3 = 0xC3,
	MDRP_C4 = 0xC4,
	MDRP_C5 = 0xC5,
	MDRP_C6 = 0xC6,
	MDRP_C7 = 0xC7,
	MDRP_C8 = 0xC8,
	MDRP_C9 = 0xC9,
	MDRP_CA = 0xCA,
	MDRP_CB = 0xCB,
	MDRP_CC = 0xCC,
	MDRP_CD = 0xCD,
	MDRP_CE = 0xCE,
	MDRP_CF = 0xCF,
	MDRP_D0 = 0xD0,
	MDRP_D1 = 0xD1,
	MDRP_D2 = 0xD2,
	MDRP_D3 = 0xD3,
	MDRP_D4 = 0xD4,
	MDRP_D5 = 0xD5,
	MDRP_D6 = 0xD6,
	MDRP_D7 = 0xD7,
	MDRP_D8 = 0xD8,
	MDRP_D9 = 0xD9,
	MDRP_DA = 0xDA,
	MDRP_DB = 0xDB,
	MDRP_DC = 0xDC,
	MDRP_DD = 0xDD,
	MDRP_DE = 0xDE,
	MDRP_DF = 0xDF,

	MIRP_E0 = 0xE0,
	MIRP_E1 = 0xE1,
	MIRP_E2 = 0xE2,
	MIRP_E3 = 0xE3,
	MIRP_E4 = 0xE4,
	MIRP_E5 = 0xE5,
	MIRP_E6 = 0xE6,
	MIRP_E7 = 0xE7,
	MIRP_E8 = 0xE8,
	MIRP_E9 = 0xE9,
	MIRP_EA = 0xEA,
	MIRP_EB = 0xEB,
	MIRP_EC = 0xEC,
	MIRP_ED = 0xED,
	MIRP_EE = 0xEE,
	MIRP_EF = 0xEF,
	MIRP_F0 = 0xF0,
	MIRP_F1 = 0xF1,
	MIRP_F2 = 0xF2,
	MIRP_F3 = 0xF3,
	MIRP_F4 = 0xF4,
	MIRP_F5 = 0xF5,
	MIRP_F6 = 0xF6,
	MIRP_F7 = 0xF7,
	MIRP_F8 = 0xF8,
	MIRP_F9 = 0xF9,
	MIRP_FA = 0xFA,
	MIRP_FB = 0xFB,
	MIRP_FC = 0xFC,
	MIRP_FD = 0xFD,
	MIRP_FE = 0xFE,
	MIRP_FF = 0xFF,


} opcode_type;


const char* opcodes_names[] = {
	ENUM_NAME2(SVTCA),
	ENUM_NAME2(SPVTCA),
	ENUM_NAME2(SFVTCA),
	ENUM_NAME2(SPVTL),


	// [SVTCA_0] = "SVCTA_X",
	// [SVTCA_1] = "SVCTA_Y",
	// 
	// [SPVTCA_0] = "SPVTCA_X",
	// [SPVTCA_1] = "SPVTCA_Y",
	// 
	// [SFVTCA_0] = "SFVTCA_X",
	// [SFVTCA_1] = "SFVTCA_Y",
	// 
	// [SPVTL_0] = "SPVTL_0",
	// [SPVTL_1] = "SPVTL_1",

	ENUM_NAME(SPVFS),
	ENUM_NAME(SFVFS),
	ENUM_NAME(GPV),
	ENUM_NAME(GFV),
	ENUM_NAME(SFVTPV),
	ENUM_NAME(ISECT),

	ENUM_NAME(SRP0),
	ENUM_NAME(SRP1),
	ENUM_NAME(SRP2),
	ENUM_NAME(SZP0),
	ENUM_NAME(SZP1),
	ENUM_NAME(SZP2),
	ENUM_NAME(SZPS),
	ENUM_NAME(SLOOP),
	ENUM_NAME(RTG),
	ENUM_NAME(RTHG),
	ENUM_NAME(SMD),
	ENUM_NAME(ELSE),
	ENUM_NAME(JMPR),
	ENUM_NAME(SCVTCI),
	ENUM_NAME(SSWCI),
	ENUM_NAME(SSW),
	ENUM_NAME(DUP),
	ENUM_NAME(POP),
	ENUM_NAME(CLEAR),
	ENUM_NAME(SWAP),
	ENUM_NAME(DEPTH),
	ENUM_NAME(CINDEX),
	ENUM_NAME(MINDEX),
	ENUM_NAME(ALIGNPTS),


	ENUM_NAME(UTP),
	ENUM_NAME(LOOPCALL),
	ENUM_NAME(CALL),
	ENUM_NAME(FDEF),
	ENUM_NAME(ENDF),
	ENUM_NAME2(MDAP),

	ENUM_NAME2(IUP),
	ENUM_NAME2(SHP),
	ENUM_NAME2(SHC),
	ENUM_NAME2(SHZ),
	ENUM_NAME(SHPIX),

	ENUM_NAME(IP),
	ENUM_NAME2(MSIRP),
	ENUM_NAME(ALIGNRP),
	ENUM_NAME(RTDG),
	ENUM_NAME2(MIAP),

	ENUM_NAME(PUSH_N_WORDS),
	ENUM_NAME(WS),
	ENUM_NAME(RS),
	ENUM_NAME(WCVTP),
	ENUM_NAME(RCVT),

	ENUM_NAME2(GC),
	ENUM_NAME(SCFS),
	ENUM_NAME2(MD),
	ENUM_NAME(MPPEM),
	ENUM_NAME(MPS),
	ENUM_NAME(FLIPON),
	ENUM_NAME(FLIPOFF),
	ENUM_NAME(TTF_DEBUG),
	
	ENUM_NAME(LT),
	ENUM_NAME(LTEQ),
	ENUM_NAME(GT),
	ENUM_NAME(GTEQ),
	ENUM_NAME(EQ),
	ENUM_NAME(NEQ),
	ENUM_NAME(ODD),
	ENUM_NAME(EVEN),
	ENUM_NAME(IF),
	ENUM_NAME(EIF),
	ENUM_NAME(AND),
	ENUM_NAME(OR),
	ENUM_NAME(NOT),
	ENUM_NAME(DELTAP1),
	ENUM_NAME(SDB),
	ENUM_NAME(SDS),

	ENUM_NAME(ADD),
	ENUM_NAME(SUB),
	ENUM_NAME(DIV),
	ENUM_NAME(MUL),
	ENUM_NAME(ABS),
	ENUM_NAME(NEG),
	ENUM_NAME(FLOOR),
	ENUM_NAME(CELING),
	ENUM_NAME(ROUND_0),
	ENUM_NAME(ROUND_1),
	ENUM_NAME(ROUND_2),
	ENUM_NAME(ROUND_3),

	ENUM_NAME(NROUND_0),
	ENUM_NAME(NROUND_1),
	ENUM_NAME(NROUND_2),
	ENUM_NAME(NROUND_3),

	ENUM_NAME(DELTAP2),
	ENUM_NAME(DELTAP3),
	ENUM_NAME(DELTAC1),
	ENUM_NAME(DELTAC2),
	ENUM_NAME(DELTAC3),
	ENUM_NAME(SROUND),
	ENUM_NAME(S45ROUND),
	ENUM_NAME(JROT),
	ENUM_NAME(JROF),
	ENUM_NAME(ROFF),
	ENUM_NAME(RUDG),
	ENUM_NAME(RDTG),
	ENUM_NAME(SANGW),


	ENUM_NAME(FLIPPT),
	ENUM_NAME(FLIPRGON),
	ENUM_NAME(FLIPRGOFF),
	ENUM_NAME(SCANCTRL),
	ENUM_NAME2(SDPVTL),
	ENUM_NAME(GETINFO),
	ENUM_NAME(IDEF),
	ENUM_NAME(ROLL),
	ENUM_NAME(MAX),
	ENUM_NAME(MIN),
	ENUM_NAME(SCANTYPE),
	ENUM_NAME(INSTCTRL),

	ENUM_NAME(GETVARIATION),

	ENUM_NAME(PUSHB_0),
	ENUM_NAME(PUSHB_1),
	ENUM_NAME(PUSHB_2),
	ENUM_NAME(PUSHB_3),
	ENUM_NAME(PUSHB_4),
	ENUM_NAME(PUSHB_5),
	ENUM_NAME(PUSHB_6),
	ENUM_NAME(PUSHB_7),

	ENUM_NAME(PUSHW_8),
	ENUM_NAME(PUSHW_9),
	ENUM_NAME(PUSHW_A),
	ENUM_NAME(PUSHW_B),
	ENUM_NAME(PUSHW_C),
	ENUM_NAME(PUSHW_D),
	ENUM_NAME(PUSHW_E),
	ENUM_NAME(PUSHW_F),

	ENUM_NAME(MDRP_C0),
	ENUM_NAME(MDRP_C1),
	ENUM_NAME(MDRP_C2),
	ENUM_NAME(MDRP_C3),
	ENUM_NAME(MDRP_C4),
	ENUM_NAME(MDRP_C5),
	ENUM_NAME(MDRP_C6),
	ENUM_NAME(MDRP_C7),
	ENUM_NAME(MDRP_C8),
	ENUM_NAME(MDRP_C9),
	ENUM_NAME(MDRP_CA),
	ENUM_NAME(MDRP_CB),
	ENUM_NAME(MDRP_CC),
	ENUM_NAME(MDRP_CD),
	ENUM_NAME(MDRP_CE),
	ENUM_NAME(MDRP_CF),
	ENUM_NAME(MDRP_D0),
	ENUM_NAME(MDRP_D1),
	ENUM_NAME(MDRP_D2),
	ENUM_NAME(MDRP_D3),
	ENUM_NAME(MDRP_D4),
	ENUM_NAME(MDRP_D5),
	ENUM_NAME(MDRP_D6),
	ENUM_NAME(MDRP_D7),
	ENUM_NAME(MDRP_D8),
	ENUM_NAME(MDRP_D9),
	ENUM_NAME(MDRP_DA),
	ENUM_NAME(MDRP_DB),
	ENUM_NAME(MDRP_DC),
	ENUM_NAME(MDRP_DD),
	ENUM_NAME(MDRP_DE),
	ENUM_NAME(MDRP_DF),

	ENUM_NAME(MIRP_E0),
	ENUM_NAME(MIRP_E1),
	ENUM_NAME(MIRP_E2),
	ENUM_NAME(MIRP_E3),
	ENUM_NAME(MIRP_E4),
	ENUM_NAME(MIRP_E5),
	ENUM_NAME(MIRP_E6),
	ENUM_NAME(MIRP_E7),
	ENUM_NAME(MIRP_E8),
	ENUM_NAME(MIRP_E9),
	ENUM_NAME(MIRP_EA),
	ENUM_NAME(MIRP_EB),
	ENUM_NAME(MIRP_EC),
	ENUM_NAME(MIRP_ED),
	ENUM_NAME(MIRP_EE),
	ENUM_NAME(MIRP_EF),
	ENUM_NAME(MIRP_F0),
	ENUM_NAME(MIRP_F1),
	ENUM_NAME(MIRP_F2),
	ENUM_NAME(MIRP_F3),
	ENUM_NAME(MIRP_F4),
	ENUM_NAME(MIRP_F5),
	ENUM_NAME(MIRP_F6),
	ENUM_NAME(MIRP_F7),
	ENUM_NAME(MIRP_F8),
	ENUM_NAME(MIRP_F9),
	ENUM_NAME(MIRP_FA),
	ENUM_NAME(MIRP_FB),
	ENUM_NAME(MIRP_FC),
	ENUM_NAME(MIRP_FD),
	ENUM_NAME(MIRP_FE),
	ENUM_NAME(MIRP_FF),


	// ENUM_NAME(MDRP_START),
	// ENUM_NAME(MDRP_END),

	// ENUM_NAME(MIRP_START),
	// ENUM_NAME(MIRP_END),
};


typedef struct vm_stack {
	i32 size;
	i32 mem[STACK_SIZE]; 
} vm_stack;

typedef struct opcode_block {
	opcode_type opcode;	
	i32 location; //location in the stream
	i32 length; //how many bytes after you get to the next instruction
} opcode_block;

typedef struct ttf_vm_context {
	u8 *stream;
	u8 *ip;
	i32 location;
	vm_stack stack;
	sh_cvt cvt;
	opcode_block *blocks;
	i32 inside_func;
	i32 inside_if;
} ttf_vm_context;



void _print_indent(ttf_vm_context *ctx) {
	if(ctx->inside_func) {
		printf("\t");
	}

	for(i32 i = 0; i < ctx->inside_if; i++) {
		printf("\t");
	}
}

#define PRINT_INDENT _print_indent(ctx)


void push_stack(ttf_vm_context *ctx, i32 val) {
	ctx->stack.mem[ctx->stack.size++] = val;
}


void push_stack_word(ttf_vm_context *ctx) {
	i32 val = ctx->ip[0] << 8 | ctx->ip[1];

	push_stack(ctx, val);

	ctx->ip += 2; //move a word
	ctx->location += 2;
}

void push_stack_byte(ttf_vm_context *ctx) {
	i32 val = ctx->ip[0];

	push_stack(ctx, val);

	ctx->ip += 1; //move a word
	ctx->location += 1;
}

i32 pop_stack(ttf_vm_context *ctx) {
	if(ctx->stack.size == 0) return 0;
	return ctx->stack.mem[--ctx->stack.size];
}


u8 peak_opcode(ttf_vm_context *ctx) {
	return *ctx->ip;
}

u8 get_opcode(ttf_vm_context *ctx) {
	ctx->location++;
	return *ctx->ip++;
}

void pri_op_code(u8 op_code) {
	printf("%10s %x\n", opcodes_names[op_code], op_code);
}

void print_stack(ttf_vm_context *ctx) {
	printf("[");
	for(i32 i = 0; i < ctx->stack.size; ++i) {
		printf("%d", ctx->stack.mem[i]);
		if(i < ctx->stack.size-1) printf(",");
	}

	printf("|]\n");
}

void pri_block(opcode_block *block) {
	printf("%s loc: %d len: %d\n", opcodes_names[block->opcode], block->location, block->length);
}

// void pri_debug_info(ttf_vm_context *ctx) {
// #ifdef TTF_OUT_DEBUG
// 	printf("next 10 bytes: \n");
// 	for(i i = 0; i < 10; ++i) {
// 		printf("0x%0.2X\n", ctx->ip[i]);
// 	}
// #endif
// }


// #undef TTF_DISASSEMBLE
// #include "ttf_vm_funcs.c"

void process_fpgm(table_directory_entry *table, font_directory *font) {

#if 0
	u8 *stream = table->data_ptr;
	i length = table->length;

	ttf_vm_context context = {.ip = stream, .stream = stream, .location = 1, .cvt = font->cvt};
	i32 quit = 0;

	while(!quit) {
		u8 current_opcode = peak_opcode(&context);
		const char* op_code_name = opcodes_names[current_opcode];
		ttf_vm_func_type func = funcs[current_opcode];

		if(func) {
			func(&context);
		} else {
			if(op_code_name == NULL) {
				printf("Opcode (%x|%d) doesn't exist. it could be a data instead of opcode.", current_opcode, context.location);
			}

			printf("Instruction not handled: %s (%x) @stream_poi(%d)\n", opcodes_names[current_opcode], current_opcode, context.location);
			quit = 1;
		}

		if(context.location >= length+1) {
			printf("we finished instructing the font.");
			quit = 1;
		}
	}

	printf("======================blocks===========================\n");
	for(i i = 0; i < buf_len(context.blocks); ++i) {
		pri_block(context.blocks + i);
	}


	// generate global functions
	// call [0] => execute functions

#endif
}
// #define TTF_DISASSEMBLE

void parse_instruction_stream(font_directory *font, u8 *stream, i32 length) {

#if 0
	ttf_vm_context context = {0};
	context.stream = stream;
	context.ip = stream;
	context.location = 1;
	context.cvt = font->cvt;


	i32 run = 0;
	while(run) {
		u8 current_opcode = peak_opcode(&context);
		const char* op_code_name = opcodes_names[current_opcode];
		ttf_vm_func_type func = funcs[current_opcode];

		if(func) {
			func(&context);
		} else {
			if(op_code_name == NULL) {
				printf("Opcode (%x|%d) doesn't exist. it could be a data instead of opcode.", current_opcode, context.location);
			}

			printf("Instruction not handled: %s (%x) @stream_poi(%d)\n", opcodes_names[current_opcode], current_opcode, context.location);
			// pri_debug_info(&context);
			run = 0;
		}

		i consumed_so_far = context.ip - context.stream;
		if(context.location >= length+1) {
			printf("we finished instructing the font.");
			run = 0;
		}
	}

#endif
}



#endif TTF_VM_ENGINE

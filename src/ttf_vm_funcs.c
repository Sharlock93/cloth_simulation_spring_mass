#define ttf_vm_func(name) void name(ttf_vm_context *ctx)
typedef void (*ttf_vm_func_type) (ttf_vm_context *ctx);
#include "sh_tools.c"

ttf_vm_func(add) {
	opcode_block block = {.location = ctx->location, .opcode = ADD, .length = 1};
	u8 opcode = get_opcode(ctx);

	i32 n1 = pop_stack(ctx);
	i32 n2 = pop_stack(ctx);
	push_stack(ctx, n1+n2);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("ADD %d+%d=%d\n", n1, n2, n1+n2);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(abs_func) {
	opcode_block block = {.location = ctx->location, .opcode = ABS, .length = 1};
	u8 opcode = get_opcode(ctx);

	i32 n1 = pop_stack(ctx);
	i32 n2 = n1;
	if(n2 < 0) n2 = -n2;

	push_stack(ctx, n2);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("ABS %d => %d\n", n1, n2);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(call) {
	opcode_block block = {.location = ctx->location, .opcode = CALL, .length = 1};
	u8 opcode = get_opcode(ctx);
	int function_id = pop_stack(ctx);
	
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("CALL %d\n", function_id);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(cindex) {
	opcode_block block = {.location = ctx->location, .opcode = CINDEX, .length = 1};

	u8 opcode = get_opcode(ctx);
	int stack_val = pop_stack(ctx);
	int kth_item = ctx->stack.mem[stack_val];
	push_stack(ctx, kth_item);
	
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("CINDEX %d\n", stack_val);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(clear) {
	opcode_block block = {.location = ctx->location, .opcode = CLEAR, .length = 1};
	u8 opcode = get_opcode(ctx);
	ctx->stack.size = 0;

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("CLEAR\n");
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(dup) {
	opcode_block block = {.location = ctx->location, .opcode = DUP, .length = 1};
	u8 opcode = get_opcode(ctx);

	push_stack(ctx, ctx->stack.mem[ctx->stack.size-1]);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("DUP\n");
#endif

	buf_push(ctx->blocks, block);
}





ttf_vm_func(neg) {
	opcode_block block = {.location = ctx->location, .opcode = NEG, .length = 1};
	u8 opcode = get_opcode(ctx);


	i32 n1 = pop_stack(ctx);
	push_stack(ctx, -n1);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("NEG %d\n", -n1);
#endif

	buf_push(ctx->blocks, block);
}



ttf_vm_func(sub) {
	opcode_block block = {.location = ctx->location, .opcode = SUB, .length = 1};
	u8 opcode = get_opcode(ctx);

	i32 n1 = pop_stack(ctx);
	i32 n2 = pop_stack(ctx);
	push_stack(ctx, n1-n2);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SUB %d-%d=%d\n", n1, n2, n1-n2);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(pushw) {

	opcode_block block = {.location = ctx->location};

	// pushw words
	u8 opcode = get_opcode(ctx);
	i32 words_to_push = (opcode & 0x7)+1; //pushw has [abc] and we are pushing words not bytes

	block.opcode = opcode;
	block.length = words_to_push*2 + 1; //one for the opcode;

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("PUSHW ");
#endif

	for(int i = 0; i < words_to_push; i++) {

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	int val = ctx->ip[0] << 8 | ctx->ip[1];
	printf("%d", val);
	if(i < words_to_push - 1) printf(",");
#endif

		push_stack_word(ctx);
	}

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf(".\n");
#endif
	// (*ip) += bytes_to_push;
	// *location += bytes_to_push;

	buf_push(ctx->blocks, block);
}


ttf_vm_func(pushb) {
	// pushw words
	u8 opcode = get_opcode(ctx);
	i32 bytes_to_push = (opcode & 0x7)+1; //pushw has [abc] and we are pushing words not bytes

// #ifdef TTF_OUT_DEBUG
// 	printf("PUSHW(0xB7)[0x%0.2X] = 0x%0.2X WTP: %d\n", opcode & 0x07, opcode, words_to_push);
// #endif

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("PUSHB ");
	// printf("PUSHW(0xB7)[0x%0.2X] = 0x%0.2X WTP: %d\n", opcode & 0x07, opcode, words_to_push);
#endif

	for(int i = 0; i < bytes_to_push; i++) {

#ifdef TTF_DISASSEMBLE
	int val = ctx->ip[0];
	printf("%d", val);
	if(i < bytes_to_push - 1) printf(",");
#endif
		push_stack_byte(ctx);
	}

#ifdef TTF_DISASSEMBLE
	printf(".\n");
#endif
	// (*ip) += bytes_to_push;
	// *location += bytes_to_push;
}

ttf_vm_func(pop) {
	opcode_block block = {.location = ctx->location, .opcode = POP, .length = 1};
	u8 opcode = get_opcode(ctx);
	int val_poped = pop_stack(ctx);
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("POP\n");
#endif
	buf_push(ctx->blocks, block);
}


// TODO(sharo): this requires you to shift the entire stack, not done currently
ttf_vm_func(mindex) {
	opcode_block block = {.location = ctx->location, .opcode = MINDEX, .length = 1};
	u8 opcode = get_opcode(ctx);
	int elem_index = pop_stack(ctx);
	int kth_item = ctx->stack.mem[elem_index];

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("stack before move: ");
	print_stack(ctx);
#endif


	for(int i = elem_index; i < ctx->stack.size-1; i++ ) {
		ctx->stack.mem[i] = ctx->stack.mem[i+1];
	}

	ctx->stack.mem[ctx->stack.size-1] = kth_item;
	/* push_stack(ctx, kth_item); */
	
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MINDEX %d %d\n", elem_index, kth_item);
	PRINT_INDENT;
	printf("stack after move: ");
	print_stack(ctx);
#endif

	buf_push(ctx->blocks, block);
}




ttf_vm_func(loopcall) {
	opcode_block block = {.location = ctx->location, .opcode = CALL, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 function_id = pop_stack(ctx);
	i32 count = pop_stack(ctx);
	
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("LOOPCALL[%d] c:%d\n", function_id, count);
#endif

	buf_push(ctx->blocks, block);
}




ttf_vm_func(srp2) {
	opcode_block block = {.location = ctx->location, .opcode = SRP2, .length = 1};
	u8 opcode = get_opcode(ctx);
	int val = pop_stack(ctx);
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SRP2 %d\n", val);
#endif
}

ttf_vm_func(mdrp) {
	opcode_block block = {.location = ctx->location, .length = 1};

	u8 opcode = get_opcode(ctx);
	block.opcode = opcode;

	int val = pop_stack(ctx);
	// 00000000
	// 00011100

	int distance_type = opcode & 0x3;
	int abc = opcode >> 2 & 0x1F;

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MDRP[0x%X|%d] %d\n", abc, distance_type, val);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(svtca) {
	u8 op_code = get_opcode(ctx);
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SVTCA %c\n", op_code & 0x1 ? 'X' : 'Y');
#endif

}

ttf_vm_func(md) {
	opcode_block block = {.location = ctx->location, .length = 1};
	u8 op_code = get_opcode(ctx);
	block.opcode = op_code;

	i32 a = op_code & 0x1;
	i32 point1 = pop_stack(ctx);
	i32 point2 = pop_stack(ctx);

	i32 distance = point2 - point1;

	push_stack(ctx, distance);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MD[%d] %d\n", a, distance);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(gc) {
	opcode_block block = {.location = ctx->location, .length = 1};
	u8 op_code = get_opcode(ctx);
	block.opcode = op_code;
	i32 a = op_code & 0x1;
	i32 point = pop_stack(ctx);
	push_stack(ctx, point);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("GC[%d] %d\n", a, point);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(rcvt) {
	opcode_block block = {.location = ctx->location, .opcode = RCVT, .length = 1};

	u8 op_code = get_opcode(ctx);
	int ctv_val = pop_stack(ctx);
	int val_to_push = ctx->cvt.values[ctv_val];
	push_stack(ctx, val_to_push);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("RCVT %d (%d)\n", ctv_val, val_to_push);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(roll) {
	opcode_block block = {.location = ctx->location, .opcode = ROLL, .length = 1};
	u8 op_code = get_opcode(ctx);

	int a = pop_stack(ctx);
	int b = pop_stack(ctx);
	int c = pop_stack(ctx);

	push_stack(ctx, c);
	push_stack(ctx, a);
	push_stack(ctx, b);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("ROLL %d, %d, %d => %d, %d, %d\n", a, b, c, c, a, b);
#endif

	buf_push(ctx->blocks, block);
}



ttf_vm_func(wcvtp) {
	opcode_block block = {.location = ctx->location, .opcode = WCVTP, .length = 1};
	u8 op_code = get_opcode(ctx);

	int value_in_pixel = pop_stack(ctx);
	int cvt_index = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("WCVTP CVT[%d] = %d\n", cvt_index, value_in_pixel);
#endif

	ctx->cvt.values[cvt_index] = value_in_pixel;

	buf_push(ctx->blocks, block);
}

ttf_vm_func(round_func) {
	opcode_block block = {.location = ctx->location, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 ab = opcode & 0x3;

	block.opcode = opcode;
	i32 val_to_round = pop_stack(ctx);
	push_stack(ctx, val_to_round);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("ROUND[%d]  (%d)\n", ab, val_to_round);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(rdtg) {
	opcode_block block = {.location = ctx->location, .opcode = RDTG, .length = 1};
	u8 opcode = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("RDTG\n");
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(rtg) {
	opcode_block block = {.location = ctx->location, .opcode = RDTG, .length = 1};
	u8 opcode = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("RTG\n");
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(if_func) {
	opcode_block block = {.location = ctx->location, .opcode = IF, .length = 1};
	u8 op_code = get_opcode(ctx);

	int ctv_val = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("IF %d\n", ctv_val);
	ctx->inside_if++;
#endif
	buf_push(ctx->blocks, block);
}

ttf_vm_func(else_func) {
	opcode_block block = {.location = ctx->location, .opcode = ELSE, .length = 1};
	u8 op_code = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	ctx->inside_if--;
	PRINT_INDENT;
	printf("ELSE\n");
	ctx->inside_if++;
#endif
	buf_push(ctx->blocks, block);
}


ttf_vm_func(eif_func) {
	opcode_block block = {.location = ctx->location, .opcode = EIF, .length = 1};
	u8 op_code = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	ctx->inside_if--;
	PRINT_INDENT;
	printf("EIF\n");
#endif
	buf_push(ctx->blocks, block);
}


ttf_vm_func(mdap) {
	u8 op_code = get_opcode(ctx);
	int ctv_val = pop_stack(ctx);
	int bit = op_code & 0x1;
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MDAP[%d] %d\n", bit, ctv_val);
#endif
}

ttf_vm_func(miap) {
	u8 op_code = get_opcode(ctx);
	int ctv_val = pop_stack(ctx);
	int bit = op_code & 0x1;
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MIAP[%d] %d\n", bit, ctv_val);
#endif
}

ttf_vm_func(iup) {
	u8 op_code = get_opcode(ctx);
	int bit = op_code & 0x1;
	printf("IUP[%d]\n", bit);
}

ttf_vm_func(instctrl) {
	opcode_block block = {.location = ctx->location, .opcode = INSTCTRL, .length = 1};
	u8 op_code = get_opcode(ctx);

	int flag = pop_stack(ctx);
	int flag_val = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("INSTCTRL %d = %d \n", flag, flag_val);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(mirp) {
	opcode_block block = {.location = ctx->location, .length = 1};
	u8 opcode = get_opcode(ctx);
	block.opcode = opcode;
	
	int val_1 = pop_stack(ctx);
	int val_2 = pop_stack(ctx);

	int distance_type = opcode & 0x3;
	int abc = opcode >> 2 & 0x1F;

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MIRP[0x%X|%d] %d %d\n", opcode, abc, distance_type, val_1, val_2);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(msirp) {
	opcode_block block = {.location = ctx->location, .length = 1};
	u8 opcode = get_opcode(ctx);
	block.opcode = opcode;
	
	int distance = pop_stack(ctx);
	int change_rp0 = opcode & 0x1;

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MSIRP[%d] %d \n", change_rp0, distance);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(scanctrl) {
	opcode_block block = {.location = ctx->location, .opcode = SCANCTRL, .length = 1};
	u8 op_code = get_opcode(ctx);
	int angle_weight = pop_stack(ctx);
#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SCANCTRL %d\n", angle_weight);
#endif

	buf_push(ctx->blocks, block);

}

ttf_vm_func(scvtci) {
	opcode_block block = {.location = ctx->location, .opcode = SCANCTRL, .length = 1};
	u8 op_code = get_opcode(ctx);
	int ctv_val = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SCVTCI %d\n", ctv_val);
#endif

}

ttf_vm_func(sdb) {
	opcode_block block = {.location = ctx->location, .opcode = SDB, .length = 1};
	u8 op_code = get_opcode(ctx);
	int delta_base = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SDB %d\n", delta_base);
#endif
	buf_push(ctx->blocks, block);
}

ttf_vm_func(sds) {
	opcode_block block = {.location = ctx->location, .opcode = SDS, .length = 1};
	u8 op_code = get_opcode(ctx);
	int delta_shift = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SDS %d\n", delta_shift);
#endif
	buf_push(ctx->blocks, block);
}



ttf_vm_func(ssw) {
	opcode_block block = {.location = ctx->location, .opcode = SSW, .length = 1};
	u8 op_code = get_opcode(ctx);
	int single_width_value = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SSW %d\n", single_width_value);
#endif
	buf_push(ctx->blocks, block);
}



ttf_vm_func(fdef) {
	opcode_block block = {.location = ctx->location, .opcode = FDEF, .length = 1};
	u8 op_code = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("================================================\n");
	print_stack(ctx);
#endif


	int func_id = pop_stack(ctx);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("FDEF %d\n", func_id);
	ctx->inside_func = 1;
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(endf) {
	opcode_block block = {.location = ctx->location, .opcode = ENDF, .length = 1};
	u8 op_code = get_opcode(ctx);

#ifdef TTF_DISASSEMBLE
	printf("ENDF\n");
	printf("================================================\n");
	ctx->inside_func = 0;
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(mppem) {
	opcode_block block = {.location = ctx->location, .opcode = MPPEM, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 ppem = 72;

	push_stack(ctx, ppem);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("MPPEM %d\n", ppem);
#endif

	buf_push(ctx->blocks, block);

}

ttf_vm_func(lt) {
	opcode_block block = {.location = ctx->location, .opcode = LT, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	i32 less_than = val1 < val2;
	push_stack(ctx, less_than);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("LT %d < %d = %d\n", val1, val2, less_than);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(lteq) {
	opcode_block block = {.location = ctx->location, .opcode = LTEQ, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	i32 lt_eq = val1 <= val2;
	push_stack(ctx, lt_eq);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("LTEQ %d <= %d = %d\n", val1, val2, lt_eq);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(gt) {
	opcode_block block = {.location = ctx->location, .opcode = GT, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	i32 gt = val1 > val2;
	push_stack(ctx, gt);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("GT %d > %d = %d\n", val1, val2, gt);
#endif

	buf_push(ctx->blocks, block);
}

ttf_vm_func(gteq) {
	opcode_block block = {.location = ctx->location, .opcode = GTEQ, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	i32 gteq = val1 >= val2;
	push_stack(ctx, gteq);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("GTEQ %d > %d = %d\n", val1, val2, gteq);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(eq) {
	opcode_block block = {.location = ctx->location, .opcode = GTEQ, .length = 1};
	u8 opcode = get_opcode(ctx);

	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	i32 eq = val1 == val2;
	push_stack(ctx, eq);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("EQ %d == %d = %d\n", val1, val2, eq);
#endif

	buf_push(ctx->blocks, block);
}


ttf_vm_func(swap) {
	opcode_block block = {.location = ctx->location, .opcode = SWAP, .length = 1};
	u8 opcode = get_opcode(ctx);
	i32 val2 = pop_stack(ctx);
	i32 val1 = pop_stack(ctx);

	push_stack(ctx, val1);
	push_stack(ctx, val2);

#ifdef TTF_DISASSEMBLE
	PRINT_INDENT;
	printf("SWAP %d <> %d\n", val1, val2);
#endif

	buf_push(ctx->blocks, block);
}


// ABCDEFGHIJKLMNOPQRSTUVWXZ
ttf_vm_func_type funcs[256] = {

	// 0
	[ADD] = add,
	[SUB] = sub,
	[NEG] = neg,
	[ABS] = abs_func,
	[CALL] = call,
	[LOOPCALL] = loopcall,
	[CLEAR] = clear,
	[CINDEX] = cindex,
	[MINDEX] = mindex,
	[DUP] = dup,
	[FDEF] = fdef,
	[ENDF] = endf,
	[IF] = if_func,
	[ELSE] = else_func,
	[EIF] = eif_func,
	[IUP_0] = iup,
	[IUP_1] = iup,
	[INSTCTRL] = instctrl,
	[GC_0] = gc,
	[GC_1] = gc,
	// [MDAP_0] = mdap,
	[POP] = pop,
	[MD_0] = md,
	[MD_1] = md,
	[MDAP_1] = mdap,
	[MDRP_D0] = mdrp,

	[MIRP_E0] = mirp,
	[MIRP_E1] = mirp,
	[MIRP_E2] = mirp,
	[MIRP_E3] = mirp,
	[MIRP_E4] = mirp,
	[MIRP_E5] = mirp,
	[MIRP_E6] = mirp,
	[MIRP_E7] = mirp,
	[MIRP_E8] = mirp,
	[MIRP_E9] = mirp,
	[MIRP_EA] = mirp,
	[MIRP_EB] = mirp,
	[MIRP_EC] = mirp,
	[MIRP_ED] = mirp,
	[MIRP_EE] = mirp,
	[MIRP_EF] = mirp,
	[MIRP_F0] = mirp,
	[MIRP_F1] = mirp,
	[MIRP_F2] = mirp,
	[MIRP_F3] = mirp,
	[MIRP_F4] = mirp,
	[MIRP_F5] = mirp,
	[MIRP_F6] = mirp,
	[MIRP_F7] = mirp,
	[MIRP_F8] = mirp,
	[MIRP_F9] = mirp,
	[MIRP_FA] = mirp,
	[MIRP_FB] = mirp,
	[MIRP_FC] = mirp,
	[MIRP_FD] = mirp,
	[MIRP_FE] = mirp,
	[MIRP_FF] = mirp,




	[MPPEM] = mppem,
	[MSIRP_0] = msirp,
	[MSIRP_1] = msirp,
	[LT] = lt,
	[LTEQ] = lteq,
	[GT] = gt,
	[GTEQ] = gteq,
	[EQ] = eq,
	[PUSHB_0] = pushb,
	[PUSHB_1] = pushb,
	[PUSHW_8] = pushw,
	[PUSHW_9] = pushw,
	[PUSHW_B] = pushw,
	[MIAP_0] = miap,
	// [MIAP_1] = miap,
	[RCVT] = rcvt,
	[ROLL] = roll,
	[WCVTP] = wcvtp,
	[ROUND_0] = round_func,
	[ROUND_1] = round_func,
	[ROUND_2] = round_func,
	[ROUND_3] = round_func,
	[RDTG] = rdtg,
	[RTG] = rtg,
	[SRP2] = srp2,
	[SWAP] = swap,
	[SVTCA_0] = svtca,
	[SCVTCI] = scvtci,
	[SDB] = sdb,
	[SDS] = sds,
	[SSW] = ssw,
	[SCANCTRL] = scanctrl,
	[SVTCA_1] = svtca,

};
// ttf_vm_func()

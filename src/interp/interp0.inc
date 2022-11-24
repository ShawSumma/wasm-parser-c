
NAME(WEB49_OPCODE_CALL_DONE0) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE1) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 1);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE2) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 2);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE4) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 4);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE8) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 8);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE16) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 16);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE32) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 32);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE64) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 64);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE128) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 128);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL_DONE256) {
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + head[3].data.i32_u, 0, sizeof(web49_interp_data_t) * 256);
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_CALL) {
    web49_interp_block_t *block = head[0].ptr;
    *returns++ = head;
    *stacks++ = interp.locals;
    interp.locals += head[1].data.i32_u;
    memset(interp.locals + block->nparams, 0, sizeof(web49_interp_data_t) * block->nlocals);
    web49_interp_block_run_comp(block, ptrs, interp);
    if (block->nparams == 0) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE0;
    }
    else if (block->nparams <= 1) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE1;
    }
    else if (block->nparams <= 2) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE2;
    }
    else if (block->nparams <= 4) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE4;
    }
    else if (block->nparams <= 8) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE8;
    }
    else if (block->nparams <= 16) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE16;
    }
    else if (block->nparams <= 32) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE32;
    }
    else if (block->nparams <= 64) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE64;
    }
    else if (block->nparams <= 128) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE128;
    }
    else if (block->nparams <= 256) {
        head[-1].ptr = &&DO_WEB49_OPCODE_CALL_DONE256;
    } else {
        __builtin_trap();
    }
    head[0].ptr = block->code;
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_BR) {
    head = head[0].ptr;
    NEXT();
}
NAME(WEB49_OPCODE_GET_LOCAL) {
    interp.locals[head[0].data.i32_u] = interp.locals[head[1].data.i32_u];
    head += 2;
    NEXT();
}
NAME(WEB49_OPCODE_GET_GLOBAL) {
    interp.locals[head[0].data.i32_u] = interp.extra->globals[head[1].data.i32_u];
    head += 2;
    NEXT();
}
NAME(WEB49_OPCODE_MEMORY_SIZE) {
    interp.locals[head[0].data.i32_u].i32_u = interp.extra->memsize / 65536;
    head += 1;
    NEXT();
}
NAME(WEB49_OPCODE_I32_CONST) {
    interp.locals[head[0].data.i32_u] = head[1].data;
    head += 2;
    NEXT();
}
NAME(WEB49_OPCODE_I64_CONST) {
    interp.locals[head[0].data.i32_u] = head[1].data;
    head += 2;
    NEXT();
}
NAME(WEB49_OPCODE_F32_CONST) {
    interp.locals[head[0].data.i32_u] = head[1].data;
    head += 2;
    NEXT();
}
NAME(WEB49_OPCODE_F64_CONST) {
    interp.locals[head[0].data.i32_u] = head[1].data;
    head += 2;
    NEXT();
}
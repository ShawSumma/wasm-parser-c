#include "tree.h"

#include "../tables.h"

web49_instr_t web49_opt_tree_read_block(web49_module_t *mod, web49_instr_t **head) {
    web49_section_type_t type_section;
    web49_section_function_t function_section;
    web49_section_import_t import_section;
    int32_t num_funcs = 0;
    for (uint64_t s = 0; s < mod->num_sections; s++) {
        web49_section_t cur = mod->sections[s];
        if (cur.header.id == WEB49_SECTION_ID_TYPE) {
            type_section = cur.type_section;
        }
        if (cur.header.id == WEB49_SECTION_ID_FUNCTION) {
            function_section = cur.function_section;
        }
        if (cur.header.id == WEB49_SECTION_ID_IMPORT) {
            import_section = cur.import_section;
            for (uint64_t j = 0; j < import_section.num_entries; j++) {
                if (import_section.entries[j].kind == WEB49_EXTERNAL_KIND_FUNCTION) {
                    num_funcs++;
                }
            }
        }
    }
    web49_instr_t ret;
    uint64_t nalloc = 8;
    ret.nargs = 0;
    ret.args = web49_malloc(sizeof(web49_instr_t) * nalloc);
    ret.immediate.id = WEB49_IMMEDIATE_NONE;
    ret.opcode = WEB49_OPCODE_BEGIN0;
    ret.args[ret.nargs++] = (web49_instr_t){
        .opcode = WEB49_OPCODE_NOP,
        .immediate = (web49_instr_immediate_t){
            .id = WEB49_IMMEDIATE_NONE,
        },
    };
    while (true) {
        web49_instr_t cur = **head;
        *head += 1;
        if (cur.opcode == WEB49_OPCODE_END || cur.opcode == WEB49_OPCODE_ELSE) {
            break;
        }
        if (cur.opcode == WEB49_OPCODE_NOP) {
            continue;
        }
        if (ret.nargs + 4 >= nalloc) {
            nalloc = (ret.nargs + 4) * 2;
            ret.args = web49_realloc(ret.args, sizeof(web49_instr_t) * nalloc);
        }
        if (cur.opcode == WEB49_OPCODE_BLOCK || cur.opcode == WEB49_OPCODE_LOOP) {
            web49_instr_t body = web49_opt_tree_read_block(mod, head);
            cur.nargs = 1;
            cur.args = web49_malloc(sizeof(web49_instr_t));
            cur.args[0] = body;
            if (cur.immediate.block_type != WEB49_TYPE_BLOCK_TYPE) {
                ret.args[ret.nargs++] = cur;
            } else {
                web49_instr_t begin0;
                begin0.opcode = WEB49_OPCODE_BEGIN0;
                begin0.immediate.id = WEB49_IMMEDIATE_NONE;
                begin0.nargs = 2;
                begin0.args = web49_malloc(sizeof(web49_instr_t) * 2);
                begin0.args[0] = ret.args[--ret.nargs];
                begin0.args[1] = cur;
                ret.args[ret.nargs++] = begin0;
            }
            continue;
        }
        if (cur.opcode == WEB49_OPCODE_IF) {
            web49_instr_t ifthen = web49_opt_tree_read_block(mod, head);
            if ((*head)[-1].opcode == WEB49_OPCODE_ELSE) {
                cur.nargs = 3;
                cur.args = web49_malloc(sizeof(web49_instr_t) * 3);
                cur.args[0] = ret.args[--ret.nargs];
                cur.args[1] = ifthen;
                cur.args[2] = web49_opt_tree_read_block(mod, head);
            } else {
                cur.nargs = 2;
                cur.args = web49_malloc(sizeof(web49_instr_t) * 2);
                cur.args[0] = ret.args[--ret.nargs];
                cur.args[1] = ifthen;
            }
            if (cur.immediate.block_type != WEB49_TYPE_BLOCK_TYPE) {
                ret.args[ret.nargs++] = cur;
            } else {
                web49_instr_t begin0;
                begin0.opcode = WEB49_OPCODE_BEGIN0;
                begin0.immediate.id = WEB49_IMMEDIATE_NONE;
                begin0.nargs = 2;
                begin0.args = web49_malloc(sizeof(web49_instr_t) * 2);
                begin0.args[0] = ret.args[--ret.nargs];
                begin0.args[1] = cur;
                ret.args[ret.nargs++] = begin0;
            }
            continue;
        }
        web49_table_stack_effect_t effect = web49_stack_effects[cur.opcode];
        size_t nargs = 0;
        for (size_t i = 0; effect.in[i] != WEB49_TABLE_STACK_EFFECT_END; i++) {
            if (effect.in[i] == WEB49_TABLE_STACK_EFFECT_ARGS) {
                if (cur.immediate.varint32 >= num_funcs) {
                    int32_t entry = function_section.entries[cur.immediate.varint32 - num_funcs];
                    nargs += type_section.entries[entry].num_params;
                } else {
                    int32_t index = 0;
                    for (uint64_t j = 0; j < import_section.num_entries; j++) {
                        if (import_section.entries[j].kind == WEB49_EXTERNAL_KIND_FUNCTION) {
                            if (index == cur.immediate.varint32) {
                                nargs += type_section.entries[import_section.entries[j].func_type.data].num_params;
                                break;
                            }
                            index += 1;
                        }
                    }
                }
            } else if (effect.in[i] == WEB49_TABLE_STACK_EFFECT_ARGS_INDIRECT) {
                nargs += type_section.entries[cur.immediate.call_indirect.index].num_params;
            } else {
                nargs += 1;
            }
        }
        bool use_begin0 = false;
        if (effect.out[0] == WEB49_TABLE_STACK_EFFECT_END) {
            use_begin0 = true;
        } else if (effect.out[0] == WEB49_TABLE_STACK_EFFECT_RET) {
            if (cur.immediate.varint32 >= num_funcs) {
                int32_t entry = function_section.entries[cur.immediate.varint32 - num_funcs];
                use_begin0 = !type_section.entries[entry].has_return_type;
            } else {
                int32_t index = 0;
                for (uint64_t j = 0; j < import_section.num_entries; j++) {
                    if (import_section.entries[j].kind == WEB49_EXTERNAL_KIND_FUNCTION) {
                        if (index == cur.immediate.varint32) {
                            use_begin0 = !type_section.entries[import_section.entries[j].func_type.data].has_return_type;
                            break;
                        }
                        index += 1;
                    }
                }
            }
        } else if (effect.out[0] == WEB49_TABLE_STACK_EFFECT_RET_INDIRECT) {
            use_begin0 = !type_section.entries[cur.immediate.call_indirect.index].has_return_type;
        }
        cur.nargs = 0;
        cur.args = web49_malloc(sizeof(web49_instr_t) * (nargs));
        ret.nargs -= nargs;
        for (size_t i = 0; i < nargs; i++) {
            cur.args[cur.nargs++] = ret.args[ret.nargs + i];
        }
        if (use_begin0 && ret.nargs != 0) {
            web49_instr_t begin0;
            begin0.opcode = WEB49_OPCODE_BEGIN0;
            begin0.immediate.id = WEB49_IMMEDIATE_NONE;
            begin0.nargs = 2;
            begin0.args = web49_malloc(sizeof(web49_instr_t) * 2);
            begin0.args[0] = ret.args[--ret.nargs];
            begin0.args[1] = cur;
            ret.args[ret.nargs++] = begin0;
        } else {
            ret.args[ret.nargs++] = cur;
        }
    }
    return ret;
}

void web49_opt_tree_code(web49_module_t *mod, web49_section_code_entry_t *entry) {
    web49_instr_t *head = entry->instrs;
    web49_instr_t instr = web49_opt_tree_read_block(mod, &head);
    entry->num_instrs = 1;
    entry->instrs[0] = instr;
}

void web49_opt_tree_module(web49_module_t *mod) {
    for (uint64_t s = 0; s < mod->num_sections; s++) {
        web49_section_t cur = mod->sections[s];
        if (cur.header.id == WEB49_SECTION_ID_CODE) {
            for (uint64_t i = 0; i < cur.code_section.num_entries; i++) {
                web49_opt_tree_code(mod, &cur.code_section.entries[i]);
            }
        }
    }
}
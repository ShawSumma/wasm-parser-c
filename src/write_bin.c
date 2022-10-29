
#include "write_bin.h"

void web49_writebin_byte(web49_writebin_buf_t *out, uint8_t u8) {
    if (out->len + 1 >= out->alloc) {
        out->alloc = (out->alloc + 2) * 4;
        out->data = web49_realloc(out->data, sizeof(uint8_t) * out->alloc);
    }
    out->data[out->len++] = u8;
}
void web49_writebin_fwrite(web49_writebin_buf_t *out, size_t n, const void *data) {
    const uint8_t *udata = (const uint8_t*)data;
    for (size_t i = 0; i < n; i++) {
        web49_writebin_byte(out, udata[i]);
    }
}
void web49_writebin_type_function(web49_writebin_buf_t *out, web49_type_function_t type) {
    web49_writebin_uleb(out, type.data);
}
void web49_writebin_type_table(web49_writebin_buf_t *out, web49_type_table_t type) {
    web49_writebin_byte(out, type.element_type);
    web49_writebin_type_memory(out, type.limits);
}
void web49_writebin_type_memory(web49_writebin_buf_t *out, web49_type_memory_t type) {
    uint8_t flags = type.maximum != UINT64_MAX ? 1 : 0;
    web49_writebin_byte(out, flags);
    web49_writebin_uleb(out, type.initial);
    if (flags & 1) {
        web49_writebin_uleb(out, type.maximum);
    }
}
void web49_writebin_init_expr(web49_writebin_buf_t *out, web49_instr_t instr) {
    web49_writebin_instr(out, instr);
    web49_writebin_byte(out, WEB49_OPCODE_END);
}

void web49_writebin_instr(web49_writebin_buf_t *out, web49_instr_t instr) {
    switch (instr.opcode) {
    case WEB49_OPCODE_MEMORY_INIT:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x08);
        break;
    case  WEB49_OPCODE_DATA_DROP:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x09);
        break;
    case WEB49_OPCODE_MEMORY_COPY:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x0A);
        break;
    case WEB49_OPCODE_MEMORY_FILL:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x0B);
        break;
    case WEB49_OPCODE_TABLE_INIT:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x0C);
        break;
    case WEB49_OPCODE_ELEM_DROP:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x0D);
        break;
    case WEB49_OPCODE_TABLE_COPY:
        web49_writebin_byte(out, 0xFC);
        web49_writebin_byte(out, 0x0E);
        break;
    default:
        web49_writebin_byte(out, instr.opcode);
        break;
    }
    // web49_writebin_byte(out, instr.immediate.id);
    switch (instr.immediate.id) {
        case WEB49_IMMEDIATE_NONE:
            break;
        case WEB49_IMMEDIATE_BLOCK_TYPE:
            web49_writebin_byte(out, instr.immediate.block_type);
            break;
        case WEB49_IMMEDIATE_VARUINT1:
            web49_writebin_byte(out, instr.immediate.varuint1);
            break;
        case WEB49_IMMEDIATE_VARUINT32:
            web49_writebin_uleb(out, (uint64_t) instr.immediate.varuint32);
            break;
        case WEB49_IMMEDIATE_VARUINT64:
            web49_writebin_uleb(out, instr.immediate.varuint64);
            break;
        case WEB49_IMMEDIATE_VARINT32:
            web49_writebin_sleb(out, (int64_t) instr.immediate.varint32);
            break;
        case WEB49_IMMEDIATE_VARINT64:
            web49_writebin_sleb(out, instr.immediate.varint64);
            break;
        case WEB49_IMMEDIATE_UINT32:
            web49_writebin_fwrite(out, sizeof(uint32_t), &instr.immediate.uint32);
            break;
        case WEB49_IMMEDIATE_UINT64:
            web49_writebin_fwrite(out, sizeof(uint64_t), &instr.immediate.uint64);
            break;
        case WEB49_IMMEDIATE_BR_TABLE:
            web49_writebin_uleb(out, instr.immediate.br_table.num_targets);
            for (uint64_t i = 0; i < instr.immediate.br_table.num_targets; i++) {
                web49_writebin_uleb(out, instr.immediate.br_table.targets[i]);
            }
            web49_writebin_uleb(out, instr.immediate.br_table.default_target);
            break;
        case WEB49_IMMEDIATE_CALL_INDIRECT:
            web49_writebin_uleb(out, instr.immediate.call_indirect.index);
            web49_writebin_byte(out, 0);
            break;
        case WEB49_IMMEDIATE_MEMORY_IMMEDIATE:
            web49_writebin_uleb(out, instr.immediate.memory_immediate.align);
            web49_writebin_uleb(out, instr.immediate.memory_immediate.offset);
            break;
    }
    switch (instr.opcode) {
        case WEB49_OPCODE_MEMORY_INIT:
            web49_writebin_byte(out, 0);
            break;
        case WEB49_OPCODE_MEMORY_COPY:
            web49_writebin_byte(out, 0);
            web49_writebin_byte(out, 0);
            break;
        case WEB49_OPCODE_MEMORY_FILL:
            web49_writebin_byte(out, 0);
            break;
        default:
            break;
    }
}
void web49_writebin_type_global(web49_writebin_buf_t *out, web49_type_global_t type) {
    web49_writebin_byte(out, type.content_type);
    web49_writebin_byte(out, type.is_mutable);
}
void web49_writebin_type(web49_writebin_buf_t *out, web49_type_t type) {
    web49_writebin_byte(out, type.tag);
    switch (type.tag) {
    case WEB49_EXTERNAL_KIND_FUNCTION:
        web49_writebin_type_function(out, type.function);
        break;
    case WEB49_EXTERNAL_KIND_TABLE:
        web49_writebin_type_table(out, type.table);
        break;
    case WEB49_EXTERNAL_KIND_MEMORY:
        web49_writebin_type_memory(out, type.memory);
        break;
    case WEB49_EXTERNAL_KIND_GLOBAL:
        web49_writebin_type_global(out, type.global);
        break;
    }
}
void web49_writebin_uleb(web49_writebin_buf_t *out, uint64_t x) {
    uint8_t buf[10];
    size_t bytes = 0;
    do {
        buf[bytes] = x & 0x7FU;
        x >>= 7;
        if (x != 0) {
            buf[bytes] |= 0x80U;
        }
        bytes += 1;
    } while (x != 0);
    web49_writebin_fwrite(out, bytes, buf);
}
void web49_writebin_sleb(web49_writebin_buf_t *out, int64_t x) {
    if (x < 0) {
        uint8_t buf[10];
        size_t bytes = 0;
        do {
            buf[bytes] = x & 0x7FU;
            x >>= 7;
            if (x != 0) {
                buf[bytes] |= 0x80U;
            }
            bytes += 1;
        } while (x != -1);
        if (buf[bytes-1] & 0x40) {
            buf[bytes++] = 0x40;
        } else {
            buf[bytes-1] |= 0x40;
        }
        web49_writebin_fwrite(out, bytes, buf);
    } else {
        web49_writebin_uleb(out, (uint64_t) x);
    }
}
void web49_writebin_section_custom(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_fwrite(out, section.header.size, section.custom_section.payload);
}
void web49_writebin_section_type(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.type_section.num_entries);
    for (uint64_t i = 0; i < section.type_section.num_entries; i++) {
        web49_section_type_entry_t entry = section.type_section.entries[i];
        web49_writebin_byte(out, entry.type);
        web49_writebin_uleb(out, entry.num_params);
        for (uint64_t j = 0; j < entry.num_params; j++) {
            web49_writebin_byte(out, entry.params[j]);
        }
        web49_writebin_uleb(out, entry.num_returns);
        if (entry.has_return_type != 0) {
            web49_writebin_byte(out, entry.return_type);
        }
    }
}
void web49_writebin_section_import(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.import_section.num_entries);
    for (uint64_t i = 0; i < section.import_section.num_entries; i++) {
        web49_section_import_entry_t entry = section.import_section.entries[i];
        uint64_t msl = strlen(entry.module_str);
        web49_writebin_uleb(out, msl);
        web49_writebin_fwrite(out, msl, entry.module_str);
        uint64_t fsl = strlen(entry.field_str);
        web49_writebin_uleb(out, fsl);
        web49_writebin_fwrite(out, fsl, entry.field_str);
        web49_writebin_byte(out, entry.kind);
        switch (entry.kind) {
            case WEB49_EXTERNAL_KIND_FUNCTION: {
                web49_writebin_type_function(out, entry.func_type);
                break;
            }
            case WEB49_EXTERNAL_KIND_TABLE: {
                web49_writebin_type_table(out, entry.table_type);
                break;
            }
            case WEB49_EXTERNAL_KIND_MEMORY: {
                web49_writebin_type_memory(out, entry.memory_type);
                break;
            }
            case WEB49_EXTERNAL_KIND_GLOBAL: {
                web49_writebin_type_global(out, entry.global_type);
                break;
            }
        }
    }
}
void web49_writebin_section_function(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.function_section.num_entries);
    for (uint64_t i = 0; i < section.import_section.num_entries; i++) {
        web49_writebin_uleb(out, section.function_section.entries[i]);
    }
}
void web49_writebin_section_table(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.table_section.num_entries);
    for (uint64_t i = 0; i < section.table_section.num_entries; i++) {
        web49_type_table_t entry = section.table_section.entries[i];
        web49_writebin_type_table(out, entry);
    }
}
void web49_writebin_section_memory(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.memory_section.num_entries);
    for (uint64_t i = 0; i < section.memory_section.num_entries; i++) {
        web49_type_memory_t entry = section.memory_section.entries[i];
        web49_writebin_type_memory(out, entry);
    }
}
void web49_writebin_section_global(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.global_section.num_entries);
    for (uint64_t i = 0; i < section.global_section.num_entries; i++) {
        web49_section_global_entry_t entry = section.global_section.entries[i];
        web49_writebin_type_global(out, entry.global);
        web49_writebin_init_expr(out, entry.init_expr);
    }
}
void web49_writebin_section_export(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.export_section.num_entries);
    for (uint64_t i = 0; i < section.export_section.num_entries; i++) {
        web49_section_export_entry_t entry = section.export_section.entries[i];
        size_t len = strlen(entry.field_str);
        web49_writebin_uleb(out, len);
        web49_writebin_fwrite(out, len, entry.field_str);
        web49_writebin_byte(out, entry.kind);
        web49_writebin_uleb(out, entry.index);
    }
}
void web49_writebin_section_start(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.export_section.num_entries);
}
void web49_writebin_section_element(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.element_section.num_entries);
    for (uint64_t i = 0; i < section.element_section.num_entries; i++) {
        web49_section_element_entry_t entry = section.element_section.entries[i];
        web49_writebin_uleb(out, entry.index);
        web49_writebin_init_expr(out, entry.offset);
        web49_writebin_uleb(out, entry.num_elems);
        for (uint64_t j = 0; j < entry.num_elems; j++) {
            web49_writebin_uleb(out, entry.elems[j]);
        }
    }
}
void web49_writebin_section_code(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.code_section.num_entries);
    for (uint64_t i = 0; i < section.code_section.num_entries; i++) {
        web49_section_code_entry_t entry = section.code_section.entries[i];
        web49_writebin_buf_t buf = {0};
        web49_writebin_uleb(&buf, entry.num_locals);
        for (uint64_t j = 0; j < entry.num_locals; j++) {
            web49_writebin_uleb(&buf, entry.locals[j].count);
            web49_writebin_byte(&buf, entry.locals[j].type);
        }
        web49_writebin_uleb(&buf, entry.num_instrs);
        for (uint64_t j = 0; j < entry.num_instrs; j++) {
            web49_writebin_instr(&buf, entry.instrs[j]);
        }
        web49_writebin_uleb(out, buf.len);
        web49_writebin_fwrite(out, buf.len, buf.data);
    }
}
void web49_writebin_section_data(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_uleb(out, section.data_section.num_entries);
    for (uint64_t i = 0; i < section.data_section.num_entries; i++) {
        web49_section_data_entry_t entry = section.data_section.entries[i];
        web49_writebin_uleb(out, entry.index);
        web49_writebin_init_expr(out, entry.offset);
        web49_writebin_uleb(out, entry.size);
        web49_writebin_fwrite(out, entry.size, entry.data);
    }
}

void web49_writebin_section(web49_writebin_buf_t *out, web49_section_t section) {
    web49_writebin_buf_t buf = {0};
    switch (section.header.id) {
        case WEB49_SECTION_ID_CUSTOM: {
            web49_writebin_section_custom(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_TYPE: {
            web49_writebin_section_type(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_IMPORT: {
            web49_writebin_section_import(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_FUNCTION: {
            web49_writebin_section_function(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_TABLE: {
            web49_writebin_section_table(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_MEMORY: {
            web49_writebin_section_memory(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_GLOBAL: {
            web49_writebin_section_global(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_EXPORT: {
            web49_writebin_section_export(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_START: {
            web49_writebin_section_start(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_ELEMENT: {
            web49_writebin_section_element(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_CODE: {
            web49_writebin_section_code(&buf, section);
            break;
        }
        case WEB49_SECTION_ID_DATA: {
            web49_writebin_section_data(&buf, section);
            break;
        }
    }
    // if (section.header.size_known) {
    //     if (section.header.size != buf.len) {
    //         fprintf(stderr, "error reading data for buffer in section: %zu\n", (size_t) section.header.id);
    //         fprintf(stderr, "              ^ .length = %zu (wanted %zu)\n", buf.len, (size_t)section.header.size);
    //         exit(1);
    //     }
    // }
    // if (section.header.id == 10) {
        // fprintf(stderr, "%zu => @%zu\n", (size_t) section.header.id, out->len);
        web49_writebin_byte(out, section.header.id);
        web49_writebin_uleb(out, buf.len);
        web49_writebin_fwrite(out, buf.len, buf.data);
    // }
}

void web49_writebin_module(web49_writebin_buf_t *out, web49_module_t mod) {
    web49_writebin_fwrite(out, 4, mod.preamble.magic);
    web49_writebin_fwrite(out, 4, mod.preamble.version);
    for (uint64_t i = 0; i < mod.num_sections; i++) {
        web49_writebin_section(out, mod.sections[i]);
    }
}
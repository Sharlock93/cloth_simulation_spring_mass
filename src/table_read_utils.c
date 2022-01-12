void read_be(const u8 *memory, u8 *dest, int byte_count) {
	for(int i = byte_count - 1; i >= 0; --i) {
		dest[byte_count - i - 1] = memory[i];
	}
}

#define GET_MEM_ADR(var) ((u8 *)(&var))
#define MOVE_POINTER(p, bytes) ((p) += (bytes))
#define READ_BE(src, dest, size) read_be(src, dest, size); MOVE_POINTER(src, size)
#define BE_READ16(mem) ((mem)[0] << 8 | (mem)[1])


#include "ttf_structs.h"
#include "ttf_vm_engine.c"

sh_loca sh_read_loca(const char *mem, i32 num_glyphs, i32 type) {
	sh_loca l = {0};
	l.offsets = (u32 *) malloc(sizeof(u32)*(num_glyphs+1));
	if(type == 0) {
		for(int i = 0; i <= num_glyphs; ++i) {
			u16 temp = 0;
			READ_BE(mem, (char *)&temp, sizeof(u16));
			l.offsets[i] = (u32)temp;
		}
	} else {
		for(int i = 0; i <= num_glyphs; ++i) {
			u32 temp = 0;
			READ_BE(mem, (char *)&temp, sizeof(u32));
			l.offsets[i] = temp;
		}
	}

	l.n = num_glyphs;
	return l;
}

sh_hmtx sh_read_hmtx(const u8 *mem, i32 num_long_hor_metric) {
	sh_hmtx new_struct = {0};
	new_struct.glyph_metrics = (sh_hor_metric *)malloc(sizeof(sh_hor_metric)*num_long_hor_metric);
	for(int i = 0; i < num_long_hor_metric; ++i) {
		new_struct.glyph_metrics[i].advance_width = BE_READ16(mem+i*4);
		new_struct.glyph_metrics[i].left_side_bearing = BE_READ16(mem+i*4+2);
	}

	return new_struct;
}

sh_format4 sh_read_format4(const u8 *mem) {
	const u8 *start_mem = mem;
	sh_format4 new_struct = {0};
	READ_BE(mem, GET_MEM_ADR(new_struct.format), sizeof(u16));
	READ_BE(mem, GET_MEM_ADR(new_struct.length), sizeof(u16));
	READ_BE(mem, GET_MEM_ADR(new_struct.language), sizeof(u16));
	READ_BE(mem, GET_MEM_ADR(new_struct.segCountX2), sizeof(u16));
	new_struct.segCount = new_struct.segCountX2/2;
	READ_BE(mem, GET_MEM_ADR(new_struct.searchRange), sizeof(u16));
	READ_BE(mem, GET_MEM_ADR(new_struct.entrySelector), sizeof(u16));

	READ_BE(mem, GET_MEM_ADR(new_struct.rangeShift), sizeof(u16));
	new_struct.endCode = (u16 *) malloc(new_struct.segCountX2);
	for(int i = 0; i < new_struct.segCountX2/2; ++i) {
		read_be(mem, (u8*)(new_struct.endCode+i), sizeof(u16));
		mem += 2;
	}
	READ_BE(mem, GET_MEM_ADR(new_struct.reservedPad), sizeof(u16));
	new_struct.startCode = (u16 *) malloc(new_struct.segCountX2);
	for(int i = 0; i < new_struct.segCountX2/2; ++i) {
		READ_BE(mem, (u8*)(new_struct.startCode+i), sizeof(u16));
	}
	new_struct.idDelta = (u16 *) malloc(new_struct.segCountX2);
	for(int i = 0; i < new_struct.segCountX2/2; ++i) {
		READ_BE(mem, (u8*)(new_struct.idDelta+i), sizeof(u16));
	}

	new_struct.idRangeOffset = (u16 *) malloc(new_struct.segCountX2);
	for(int i = 0; i < new_struct.segCount; ++i) {
		READ_BE(mem, (u8*)(new_struct.idRangeOffset+i), sizeof(u16));
	}

	u16 remaining_length = (u16)( new_struct.length - (mem - start_mem) );
	new_struct.glyphIdArray = (u16 *) malloc(remaining_length);
	new_struct.glyphIndexCount = remaining_length/2;
	for(int i = 0; i < remaining_length/2; ++i) {
		READ_BE(mem, (u8 *)(new_struct.glyphIdArray+i), sizeof(u16));
	}

	return new_struct;
}

void print_format4(sh_format4 *f4) {
	printf("Format: %d, Length: %d, Language: %d, Segment Count: %d\n", f4->format, f4->length, f4->language, f4->segCountX2/2);
	printf("Search Params: (searchRange: %d, entrySelector: %d, rangeShift: %d)\n",
			f4->searchRange, f4->entrySelector, f4->rangeShift);
	printf("Segment Ranges:\tstartCode\tendCode\tidDelta\tidRangeOffset\n");
	for(int i = 0; i < f4->segCountX2/2; ++i) {
		printf("--------------:\t% 9d\t% 7d\t% 7d\t% 12d\n", f4->startCode[i], f4->endCode[i], f4->idDelta[i], f4->idRangeOffset[i]);
	}

	printf("glphIndex:");
	for(int i = 0; i < f4->glyphIndexCount; ++i) {
		if(i%4 == 0) printf("\n--------------:\t");
		printf("\t% 9d", f4->glyphIdArray[i]);
	}
}

void read_name_table(u8 *mem, name_table *name,
					table_directory_entry *name_table_info)
{
	u32 offset = name_table_info->offset;
	u32 len = name_table_info->length;

	mem += offset;
	u8 *data_pointer = mem;

	read_be(mem, GET_MEM_ADR(name->format), sizeof(name->format));
	MOVE_POINTER(mem, sizeof(name->format));

	read_be(mem, GET_MEM_ADR(name->count), sizeof(name->count));
	MOVE_POINTER(mem, sizeof(name->count));

	read_be(mem, GET_MEM_ADR(name->string_offset), sizeof(name->string_offset));
	MOVE_POINTER(mem, sizeof(name->string_offset));

	MOVE_POINTER(data_pointer, name->string_offset);

	name->records = (name_record *) HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			sizeof(name_record)*name->count
			);


	for(int i = 0; i < name->count; ++i) {
		name_record *rec = name->records+i;

		read_be(mem, GET_MEM_ADR(rec->platform_id), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		read_be(mem, GET_MEM_ADR(rec->platform_specific_id), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		read_be(mem, GET_MEM_ADR(rec->language_id), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		read_be(mem, GET_MEM_ADR(rec->name_id), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		read_be(mem, GET_MEM_ADR(rec->length), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		read_be(mem, GET_MEM_ADR(rec->offset), sizeof(u16));
		MOVE_POINTER(mem, sizeof(u16));

		rec->data = (u8 *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, rec->length);

		memcpy(rec->data, data_pointer+rec->offset, rec->length);
	}
}

void print_name_table(name_table *name) {
	printf("name table(c: %d) (\n", name->count);
	for(int i = 0; i < name->count; ++i) {
		name_record *r = name->records + i;
		printf("\tr(pid: %d, nid: %d) data: %.*s\n", r->platform_id, r->name_id,
				r->length, r->data);
	}
	printf(")\n");
}

void read_table_directory(u8* mem, table_directory_entry *table, i32 num_tables) {
	u8 *file_start = mem - sizeof(offset_subtable);
	for(int i = 0; i < num_tables; ++i) {
		table_directory_entry *ent = table+i;

		CopyMemory(GET_MEM_ADR(ent->tag), mem, sizeof(ent->tag));
		MOVE_POINTER(mem, sizeof(ent->tag));

		read_be(mem, GET_MEM_ADR(ent->check_sum), sizeof(ent->check_sum));
		MOVE_POINTER(mem, sizeof(ent->check_sum));

		read_be(mem, GET_MEM_ADR(ent->offset), sizeof(ent->offset));
		MOVE_POINTER(mem, sizeof(ent->offset));

		read_be(mem, GET_MEM_ADR(ent->length), sizeof(ent->length));
		MOVE_POINTER(mem, sizeof(ent->length));

		ent->data_ptr = file_start+ent->offset;
	}
}

void read_offset_subtable(u8 *mem, offset_subtable *subtable) {
	u8 *t_p = mem;
	read_be(t_p, GET_MEM_ADR(subtable->scaler_type), sizeof(subtable->scaler_type));
	MOVE_POINTER(t_p, sizeof(subtable->scaler_type));

	read_be(t_p, GET_MEM_ADR(subtable->num_tables), sizeof(subtable->num_tables));
	MOVE_POINTER(t_p, sizeof(subtable->num_tables));
	
	read_be(t_p, GET_MEM_ADR(subtable->search_range), sizeof(subtable->search_range));
	MOVE_POINTER(t_p, sizeof(subtable->search_range));

	read_be(t_p, GET_MEM_ADR(subtable->entry_selector), sizeof(subtable->entry_selector));
	MOVE_POINTER(t_p, sizeof(subtable->entry_selector));

	read_be(t_p, GET_MEM_ADR(subtable->range_shift), sizeof(subtable->range_shift));
	MOVE_POINTER(t_p, sizeof(subtable->range_shift));
}


//TODO(sharo): did I break it?
void read_font_directory(font_directory *fontdir) {
	u8 *mem = fontdir->data;
	
	read_offset_subtable(mem, &fontdir->subtable);
	MOVE_POINTER(mem, sizeof(offset_subtable));

	fontdir->table_directory = (table_directory_entry *)malloc(sizeof(table_directory_entry)*fontdir->subtable.num_tables);

	read_table_directory(mem, fontdir->table_directory, fontdir->subtable.num_tables);
	fontdir->entry_count = fontdir->subtable.num_tables;
}

void print_font_dir(table_directory_entry *font_dir, i32 entry_count) {
	printf("font directory: (\n");
	for(int i = 0; i < entry_count; ++i) {
		table_directory_entry *entry = font_dir + i;
		printf("\t entry %d (tag: %.4s, offset: %d, length: %d)\n",
				i+1, entry->tag_c, entry->offset, entry->length);
	}
	printf(")\n");
}


void print_subtable(offset_subtable *subtable) {
	printf("type: %.8x, tnum: %d\n", subtable->scaler_type, subtable->num_tables);
}

void process_cmap(table_directory_entry *table, font_directory *font) {
	font->cmap = sh_read_cmap(table->data_ptr);
	for(int i = 0; i < font->cmap.numTables; ++i) {
		sh_EncodingRecord m = sh_read_EncodingRecord(table->data_ptr+sizeof(sh_cmap)+sizeof(sh_EncodingRecord)*i);
		u16 format = 0;
 		read_be(table->data_ptr+m.offset, GET_MEM_ADR(format), sizeof(u16));

		// printf("%d %d %d\n", format, m.platformID, m.encodingID);
		//microsoft with BMP only Unicode
		if(format == 4) {
			font->f4 = sh_read_format4(table->data_ptr+m.offset);
			/* print_format4(&font->f4); */
		}
	}
}

u16 get_glyph_index(sh_format4 *f4, u16 char_code) {
	i32 seg_index = 0;
	for(i32 i = 0; i < f4->segCount; ++i) {
		if(f4->endCode[i] >= char_code) {
			seg_index = i;
			break;
		}
	}

	if(char_code < f4->startCode[seg_index]) { return 0; }
		
	u16 range_offset = char_code - f4->startCode[seg_index];
	if(f4->idRangeOffset[seg_index] == 0) {
		return char_code+f4->idDelta[seg_index];
	}
	range_offset += f4->idRangeOffset[seg_index]/2;

	// this shit is here because idRangeOffset is not contiguous with glyphIndexArray as it should be
	u16 *glyph_index = f4->idRangeOffset+seg_index;
	u16 *idrange_end = f4->idRangeOffset+f4->segCount;

	u16 offset_into_glyphIdArray = range_offset - (u16)(idrange_end - glyph_index);
	u16 glyph_array_index = *(f4->glyphIdArray + offset_into_glyphIdArray);
	if(glyph_array_index != 0) {
		glyph_array_index = (glyph_array_index + f4->idDelta[seg_index])%65536;
	}

	return glyph_array_index;
}

sh_glyph_offset get_glyph_offset(sh_loca* loc, i32 index, i32 type) {
	sh_glyph_offset data = {0};
	data.offset = type == 1 ? loc->offsets[index] : loc->offsets[index]*2;
	data.length = type == 1 ? loc->offsets[index+1] - data.offset : loc->offsets[index+1]*2 - data.offset;
	return data;
}

sh_glyph_rectangle get_glyph_rectangle(font_directory *font, u16 char_code) {
	sh_glyph_rectangle rect = {0};
	i32 index = get_glyph_index(&font->f4, char_code);
	sh_glyph_offset char_offset = get_glyph_offset(&font->loca, index, font->head.indexToLocFormat);
	u8 *glyph_data_start = font->glyph_table+char_offset.offset;
	rect.p1.x = BE_READ16(glyph_data_start+2);
	rect.p1.y = BE_READ16(glyph_data_start+4);
	rect.p2.x = BE_READ16(glyph_data_start+6);
	rect.p2.y = BE_READ16(glyph_data_start+8);

	return rect;
}


sh_glyph_outline get_glyph_outline(font_directory *font, u16 char_code) {
	// begin_time_block(__func__);
	sh_glyph_outline glyph_outline = {0};
	i32 index = get_glyph_index(&font->f4, char_code);
	glyph_outline.glyph_index = index;
	sh_glyph_offset char_offset = get_glyph_offset(&font->loca, index, font->head.indexToLocFormat);
	u8 *glyph_data_start = font->glyph_table+char_offset.offset;


	if(char_offset.length != 0) {

		i16 contour_count = BE_READ16(glyph_data_start);

		// mark 1
		glyph_outline.p1.x = BE_READ16(glyph_data_start+2);
		glyph_outline.p1.y = BE_READ16(glyph_data_start+4);
		glyph_outline.p2.x = BE_READ16(glyph_data_start+6);
		glyph_outline.p2.y = BE_READ16(glyph_data_start+8);

		glyph_outline.contour_count = contour_count;

		if(contour_count > 0) {
			glyph_outline.instruction_length = BE_READ16(glyph_data_start+10+contour_count*2);
			glyph_outline.instructions = (u8 *)malloc(glyph_outline.instruction_length);
			glyph_outline.contour_last_index = (u16 *)malloc(2*contour_count);

			for(int i = 0; i < contour_count; ++i) {
				glyph_outline.contour_last_index[i] = BE_READ16(glyph_data_start+10+i*2);
			}

			memcpy(glyph_outline.instructions, glyph_data_start+10+contour_count*2+2, glyph_outline.instruction_length);

			i32 last_point_index = BE_READ16(glyph_data_start+10+(contour_count-1)*2);
			i32 size_of_arrays = last_point_index+1; //last_point_index is 0 based
			glyph_outline.points = (sh_font_point*) malloc(sizeof(sh_font_point)*(size_of_arrays));

			// flags
			u8 *flag_start = glyph_data_start+10+contour_count*2+2+glyph_outline.instruction_length;
			for(int i = 0; i < size_of_arrays; ++i) {
				glyph_outline.points[i].flag = *(sh_glyph_flag*)flag_start;
				if(glyph_outline.points[i].flag.repeat) {
					++flag_start;
					u8 repeat_count = *flag_start;
					sh_glyph_flag repeated_item = glyph_outline.points[i].flag;
					while(repeat_count-- > 0) {
						glyph_outline.points[++i].flag = repeated_item;
					}
				} 
				flag_start++;
			}

			u8 *cords_start = flag_start;

			i16 init_point = 0;

			for(int i = 0; i < size_of_arrays; ++i) {
				sh_glyph_flag flag = glyph_outline.points[i].flag;
				u8 combined_info = (flag.x_short_vector_pos << 1) | (flag.x_short_vector);
				switch(combined_info) {
					case 0: {
						init_point = BE_READ16(cords_start) + init_point;
						cords_start += 2; break;
					}
					case 1: case 3: {
						i16 current_point = (*cords_start);
						init_point = (flag.x_short_vector_pos ? current_point : -current_point) + init_point;
						cords_start += 1;
					} break;
				}
				glyph_outline.points[i].x = init_point;
			}

			init_point = 0;
			for(int i = 0; i < size_of_arrays; ++i) {
				sh_glyph_flag info = glyph_outline.points[i].flag;
				u8 combined_info = (info.y_short_vector_pos << 1) | (info.y_short_vector);
				switch(combined_info) {
					// case 2: glyph_outline.points[i].y = i > 0 ? glyph_outline.points[ i-1].y : 0 ; break; //special case
					case 0: init_point = BE_READ16(cords_start) + init_point ; cords_start += 2; break;
					case 1:case 3: {
						i16 current_point = (*cords_start);
						cords_start += 1;
						init_point = (info.y_short_vector_pos ? current_point : -current_point) + init_point;
					} break;
				}
				glyph_outline.points[i].y = init_point;
			}
			glyph_outline.points_count = size_of_arrays;
		} else {
			puts("comp");
		}

	}
	// end_time_block(COUNTER, __func__);
	return glyph_outline;
}

void sh_free_glyph_outline(sh_glyph_outline *outline) {
	free(outline->instructions);
	free(outline->contour_last_index);
	free(outline->points);
}

sh_glyph_metric sh_get_glyph_metric(font_directory *font, sh_glyph_outline *glyph, float scale) {
	sh_hhea* hhea = &font->hhea;
	sh_glyph_metric metric = {0};

	if(hhea->numOfLongHorMetrics == 1) {
		metric.x_advance = font->hmtx.glyph_metrics[0].advance_width*scale;
		metric.left_side_bearing = font->hmtx.glyph_metrics[0].left_side_bearing*scale;
	} else {
		metric.x_advance = font->hmtx.glyph_metrics[glyph->glyph_index].advance_width*scale;
		metric.left_side_bearing = font->hmtx.glyph_metrics[glyph->glyph_index].left_side_bearing*scale;
	}

	return metric;
}

i16* sh_read_cvt(table_directory_entry *table) {

	u8 *mem = table->data_ptr;
	i32 table_length = table->length/2;
	i16 *cvt_table = (i16 *)malloc(sizeof(i16)*table_length);
	for(int i = 0; i < table_length; ++i) {
		READ_BE(mem, GET_MEM_ADR(cvt_table[i]), sizeof(i16));
	}

	return cvt_table;
}

// sh_fpgm sh_read_fpgm(table_directory_entry *table, font_directory *font) {
// 	u8 *start = table->data_ptr;
// 	sh_fpgm fpgm = {0};
// 
// 	return fpgm;
// }


font_directory* sh_init_font(u8 *memory) {
	u8 *file = memory;
	//TODO(sharo): should this be in their own place?
	font_directory *font = (font_directory *)malloc(sizeof(font_directory));
	font->data = file;
	read_font_directory(font);

	for(int i = 0; i < font->subtable.num_tables; ++i) {
		table_directory_entry *table = font->table_directory + i;
		switch(table->tag) {
			case CMAP:
				process_cmap(table, font);
				break;
			case MAXP:
				font->maxp = sh_read_maxp(table->data_ptr);
				break;
			case HHEA:
				font->hhea = sh_read_hhea(table->data_ptr);
				break;
			case HEAD:
				font->head = sh_read_head(table->data_ptr);
				break;
			case GLYF:
				font->glyph_table = table->data_ptr;
				break;
			case CVT_: {
				font->cvt.length = table->length/2;
				font->cvt.values = sh_read_cvt(table);
			} break;
			
		}
	}

	for(int i = 0; i < font->subtable.num_tables; ++i) {
		table_directory_entry *table = font->table_directory + i;
		switch(table->tag) {
			case LOCA:
				font->loca = sh_read_loca((char*)table->data_ptr,
						font->maxp.numGlyphs,
						font->head.indexToLocFormat);
				break;
			case HMTX:
				font->hmtx = sh_read_hmtx(table->data_ptr, font->hhea.numOfLongHorMetrics);
				break;
			case FPGM: {
				process_fpgm(table, font);
			} break;

		}
	}
	return font;
}

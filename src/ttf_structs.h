#ifndef SH_TTF_STRUCTS_H
#define SH_TTF_STRUCTS_H
#define FONT_TAG(a, b, c, d) (d<<24|c<<16|b<<8|a)
#define ACNT FONT_TAG('a','c','n','t')
#define ANKR FONT_TAG('a','n','k','r')
#define AVAR FONT_TAG('a','v','a','r')
#define BDAT FONT_TAG('b','d','a','t')
#define BHED FONT_TAG('b','h','e','d')
#define BLOC FONT_TAG('b','l','o','c')
#define BSLN FONT_TAG('b','s','l','n')
#define CMAP FONT_TAG('c','m','a','p')
#define CVAR FONT_TAG('c','v','a','r')
#define CVT_ FONT_TAG('c','v','t',' ')
#define EBSC FONT_TAG('E','B','S','C')
#define FDSC FONT_TAG('f','d','s','c')
#define FEAT FONT_TAG('f','e','a','t')
#define FMTX FONT_TAG('f','m','t','x')
#define FOND FONT_TAG('f','o','n','d')
#define FPGM FONT_TAG('f','p','g','m')
#define FVAR FONT_TAG('f','v','a','r')
#define GASP FONT_TAG('g','a','s','p')
#define GCID FONT_TAG('g','c','i','d')
#define GLYF FONT_TAG('g','l','y','f')
#define GVAR FONT_TAG('g','v','a','r')
#define HDMX FONT_TAG('h','d','m','x')
#define HEAD FONT_TAG('h','e','a','d')
#define HHEA FONT_TAG('h','h','e','a')
#define HMTX FONT_TAG('h','m','t','x')
#define JUST FONT_TAG('j','u','s','t')
#define KERN FONT_TAG('k','e','r','n')
#define KERX FONT_TAG('k','e','r','x')
#define LCAR FONT_TAG('l','c','a','r')
#define LOCA FONT_TAG('l','o','c','a')
#define LTAG FONT_TAG('l','t','a','g')
#define MAXP FONT_TAG('m','a','x','p')
#define META FONT_TAG('m','e','t','a')
#define MORT FONT_TAG('m','o','r','t')
#define MORX FONT_TAG('m','o','r','x')
#define NAME FONT_TAG('n','a','m','e')
#define OPBD FONT_TAG('o','p','b','d')
#define OS_2 FONT_TAG('O','S','/','2')
#define POST FONT_TAG('p','o','s','t')
#define PREP FONT_TAG('p','r','e','p')
#define PROP FONT_TAG('p','r','o','p')
#define SBIX FONT_TAG('s','b','i','x')
#define TRAK FONT_TAG('t','r','a','k')
#define VHEA FONT_TAG('v','h','e','a')
#define VMTX FONT_TAG('v','m','t','x')
#define XREF FONT_TAG('x','r','e','f')
#define ZAPF FONT_TAG('Z','a','p','f')
#include "ttf_generated_structs.h"


typedef struct offset_subtable {
	u32 scaler_type;
	u16 num_tables;
	u16 search_range;
	u16 entry_selector;
	u16 range_shift;
} offset_subtable;

typedef struct table_directory_entry {
	union {
		i32 tag;
		char tag_c[4];
	};
	u32 check_sum;
	u32 offset;
	u32 length;
	u8  *data_ptr;
} table_directory_entry;

typedef struct name_record {
	u16 platform_id;
	u16 platform_specific_id;
	u16 language_id;

	u16 name_id;
	u16 length;
	u16 offset;

	u8 *data;
} name_record;

typedef struct name_table {
	u16 format;
	u16 count;
	u16 string_offset;
	name_record *records;
	u8 variable_length[1];
} name_table;

// typedef union { char PANOSE[10]; };

typedef struct sh_glyph_offset {
	i32 offset;
	i32 length;
} sh_glyph_offset;

typedef union {
	struct {
		unsigned char on_curve: 1;
		unsigned char x_short_vector: 1;
		unsigned char y_short_vector: 1;
		unsigned char repeat: 1;
		unsigned char x_short_vector_pos: 1;
		unsigned char y_short_vector_pos: 1;
		unsigned char reserved: 2;
	};
	u8 flags;
} sh_glyph_flag;

// typedef struct sh_simple_glyf {
// 	u16 *end_pts_cont;
// 	i16 *x_cord;
// 	i16 *y_cord;
// 	sh_glyf_flags *flags;	
// 	u8 number_of_pois;
// } simple_glyf;


typedef struct sh_format4 {
	u16  format ; // format  
 	u16  length ; // length  
 	u16  language ; // language  
 	u16  segCountX2 ; // segCountX2  
	u16  segCount;
 	u16  searchRange ; // searchRange  
 	u16  entrySelector ; // entrySelector  
 	u16  rangeShift ; // entrySelector  
	u16  *endCode;
	u16  reservedPad;
	u16  *startCode;
	u16  *idDelta;
	u16  *idRangeOffset;
	u16  *glyphIdArray;
	u16  glyphIndexCount;
 } sh_format4;

typedef struct sh_loca {
	u32 *offsets;
	i32 n;
} sh_loca;

typedef struct sh_hor_metric {
	u16 advance_width;
	i16 left_side_bearing;
} sh_hor_metric;

typedef struct sh_hmtx {
	sh_hor_metric* glyph_metrics;
	i16 *left_side_bearing;
} sh_hmtx;


typedef struct sh_prg {
	u8 *instructions;
	i32 length;
} sh_prg;

typedef struct sh_fpgm {
	sh_prg *programs;
	i32 size;
} sh_fpgm;

typedef struct font_directory {
	offset_subtable subtable;
	table_directory_entry *table_directory;
	i32 entry_count;
	u8 *data;
	sh_cmap cmap;
	sh_cvt cvt;
	sh_fpgm fpgm;
	sh_maxp maxp;
	sh_loca loca;
	sh_head head;
	sh_hhea hhea;
	sh_hmtx hmtx;
	u8 *glyph_table;
	sh_format4 f4;

} font_directory;


typedef struct sh_font_point {
	i16 x;
	i16 y;
	sh_glyph_flag flag;
} sh_font_point;

typedef struct sh_fword_point {
	i16 x;
	i16 y;
} sh_fword_point;


typedef struct sh_glyph_outline {
	i32 glyph_index;
	i32 contour_count;
	i32 points_count;
	sh_fword_point p1;
	sh_fword_point p2;
	u16 instruction_length;
	u8  *instructions;
	u16 *contour_last_index;
	sh_font_point *points;	
} sh_glyph_outline;


typedef struct sh_glyph_metric {
	f32 x_advance;
	f32 left_side_bearing;
} sh_glyph_metric;

typedef struct sh_glyph_rectangle {
	sh_fword_point p1;
	sh_fword_point p2;
} sh_glyph_rectangle;


#endif


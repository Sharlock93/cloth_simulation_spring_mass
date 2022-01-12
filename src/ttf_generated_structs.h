#ifndef _TTF_GEN_H
#define _TTF_GEN_H
typedef struct sh_cmap {
	u16 version; // version 
 	u16 numTables; // numTables 
 } sh_cmap;

typedef struct sh_EncodingRecord {
	u16  platformID ; // platformID  
 	u16  encodingID ; // encodingID  
 	i32  offset ; // offset  
 } sh_EncodingRecord;

typedef struct sh_glyf {
	i16  numberOfContours ; // numberOfContours  
 	i16  xMin ; // xMin  
 	i16  yMin ; // yMin  
 	i16  xMax ; // xMax  
 	i16  yMax ; // yMax  
 } sh_glyf;

typedef struct sh_head {
	i32  version ; // version  
 	i32  fontRevision ; // fontRevision  
 	u32  checkSumAdjustment ; // checkSumAdjustment  
 	u32  magicNumber ; // magicNumber  
 	u16  flags ; // flags  
 	u16  unitsPerEm ; // unitsPerEm  
 	i64  created ; // created  
 	i64  modified ; // modified  
 	i16  xMin ; // xMin  
 	i16  yMin ; // yMin  
 	i16  xMax ; // xMax  
 	i16  yMax ; // yMax  
 	u16  macStyle ; // macStyle  
 	u16  lowestRecPPEM ; // lowestRecPPEM  
 	i16  fontDirectionHi ; // fontDirectionHi  
 	i16  indexToLocFormat ; // indexToLocFormat  
 	i16  glyphDataFormat ; // glyphDataFormat  
 } sh_head;

typedef struct sh_hhea {
	i32  version ; // version  
 	i16  ascent ; // ascent  
 	i16  descent ; // descent  
 	i16  lineGap ; // lineGap  
 	u16  advanceWidthMax ; // advanceWidthMax  
 	i16  minLeftSideBearing ; // minLeftSideBearing  
 	i16  minRightSideBearing ; // minRightSideBearing  
 	i16  xMaxExtent ; // xMaxExtent  
 	i16  caretSlopeRise ; // caretSlopeRise  
 	i16  caretSlopeRun ; // caretSlopeRun  
 	i16  caretOffset ; // caretOffset  
 	i16  reserved1; // reserved1 
 	i16  reserved2; // reserved2 
 	i16  reserved3; // reserved3 
 	i16  reserved4; // reserved4 
 	i16  metricDataFormat ; // metricDataFormat  
 	u16  numOfLongHorMetrics ; // numOfLongHorMetrics  
 } sh_hhea;

typedef struct sh_maxp {
	i32  version ; // version  
 	u16  numGlyphs ; // numGlyphs  
 	u16  maxPoints ; // maxPois  
 	u16  maxContours ; // maxContours  
 	u16  maxComponentPois ; // maxComponentPois  
 	u16  maxComponentContours ; // maxComponentContours  
 	u16  maxZones ; // maxZones  
 	u16  maxTwilightPois ; // maxTwilightPois  
 	u16  maxStorage ; // maxStorage  
 	u16  maxFunctionDefs ; // maxFunctionDefs  
 	u16  maxInstructionDefs ; // maxInstructionDefs  
 	u16  maxStackElements ; // maxStackElements  
 	u16  maxSizeOfInstructions ; // maxSizeOfInstructions  
 	u16  maxComponentElements ; // maxComponentElements  
 	u16  maxComponentDepth ; // maxComponentDepth  
 } sh_maxp;

typedef struct sh_os_2 {
	u16 version; // version 
 	i16  xAvgCharWidth ; // xAvgCharWidth  
 	u16  usWeightClass ; // usWeightClass  
 	u16  usWidthClass ; // usWidthClass  
 	i16  fsType ; // fsType  
 	i16  ySubscriptXSize ; // ySubscriptXSize  
 	i16  ySubscriptYSize ; // ySubscriptYSize  
 	i16  ySubscriptXOffset ; // ySubscriptXOffset  
 	i16  ySubscriptYOffset ; // ySubscriptYOffset  
 	i16  ySuperscriptXSize ; // ySuperscriptXSize  
 	i16  ySuperscriptYSize ; // ySuperscriptYSize  
 	i16  ySuperscriptXOffset ; // ySuperscriptXOffset  
 	i16  ySuperscriptYOffset ; // ySuperscriptYOffset  
 	i16  yStrikeoutSize ; // yStrikeoutSize  
 	i16  yStrikeoutPosition ; // yStrikeoutPosition  
 	i16  sFamilyClass ; // sFamilyClass  
 	PANOSE  panose ; // panose  
 	u32  ulCharRange[4] ; // ulCharRange[4]  
 	i8  achVendID[4] ; // achVendID[4]  
 	u16  fsSelection ; // fsSelection  
 	u16  fsFirstCharIndex ; // fsFirstCharIndex  
 	u16  fsLastCharIndex ; // fsLastCharIndex  
 } sh_os_2;

typedef struct sh_RatioRange {
	u8  bCharSet ; // bCharSet  
 	u8  xRatio ; // xRatio  
 	u8  yStartRatio ; // yStartRatio  
 	u8  yEndRatio ; // yEndRatio  
 } sh_RatioRange;

typedef struct sh_vdmx {
	u16  version ; // version  
 	u16  numRecs ; // numRecs  
 	u16  numRatios ; // numRatios  
 } sh_vdmx;

typedef struct sh_cvt {
	i32 length;
	i16 *values;
 } sh_cvt;



sh_cmap sh_read_cmap(const u8 *mem) {
	sh_cmap new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version), sizeof(u16));
	READ_BE(mem,GET_MEM_ADR(new_struct.numTables), sizeof(u16));
	return new_struct;
}

sh_EncodingRecord sh_read_EncodingRecord(const u8 *mem) {
	sh_EncodingRecord new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.platformID ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.encodingID ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.offset ), sizeof(i32 ));
	return new_struct;
}

sh_glyf sh_read_glyf(const u8 *mem) {
	sh_glyf new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.numberOfContours ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xMin ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yMin ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xMax ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yMax ), sizeof(i16 ));
	return new_struct;
}

sh_head sh_read_head(const u8 *mem) {
	sh_head new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version ), sizeof(i32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fontRevision ), sizeof(i32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.checkSumAdjustment ), sizeof(u32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.magicNumber ), sizeof(u32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.flags ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.unitsPerEm ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.created ), sizeof(i64 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.modified ), sizeof(i64 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xMin ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yMin ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xMax ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yMax ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.macStyle ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.lowestRecPPEM ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fontDirectionHi ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.indexToLocFormat ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.glyphDataFormat ), sizeof(i16 ));
	return new_struct;
}
sh_hhea sh_read_hhea(const u8 *mem) {
	sh_hhea new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version ), sizeof(i32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ascent ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.descent ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.lineGap ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.advanceWidthMax ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.minLeftSideBearing ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.minRightSideBearing ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xMaxExtent ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.caretSlopeRise ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.caretSlopeRun ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.caretOffset ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.reserved1), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.reserved2), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.reserved3), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.reserved4), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.metricDataFormat ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.numOfLongHorMetrics ), sizeof(u16 ));
	return new_struct;
}
sh_maxp sh_read_maxp(const u8 *mem) {
	sh_maxp new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version ), sizeof(i32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.numGlyphs ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxPoints ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxContours ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxComponentPois ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxComponentContours ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxZones ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxTwilightPois ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxStorage ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxFunctionDefs ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxInstructionDefs ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxStackElements ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxSizeOfInstructions ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxComponentElements ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.maxComponentDepth ), sizeof(u16 ));
	return new_struct;
}
sh_os_2 sh_read_os_2(const u8 *mem) {
	sh_os_2 new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version), sizeof(u16));
	READ_BE(mem,GET_MEM_ADR(new_struct.xAvgCharWidth ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.usWeightClass ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.usWidthClass ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fsType ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySubscriptXSize ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySubscriptYSize ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySubscriptXOffset ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySubscriptYOffset ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySuperscriptXSize ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySuperscriptYSize ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySuperscriptXOffset ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ySuperscriptYOffset ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yStrikeoutSize ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yStrikeoutPosition ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.sFamilyClass ), sizeof(i16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.panose ), sizeof(PANOSE ));
	READ_BE(mem,GET_MEM_ADR(new_struct.ulCharRange[4] ), sizeof(u32 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.achVendID[4] ), sizeof(i8 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fsSelection ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fsFirstCharIndex ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.fsLastCharIndex ), sizeof(u16 ));
	return new_struct;
}

sh_RatioRange sh_read_RatioRange(const u8 *mem) {
	sh_RatioRange new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.bCharSet ), sizeof(u8 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.xRatio ), sizeof(u8 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yStartRatio ), sizeof(u8 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.yEndRatio ), sizeof(u8 ));
	return new_struct;
}

sh_vdmx sh_read_vdmx(const u8 *mem) {
	sh_vdmx new_struct = {0};
	READ_BE(mem,GET_MEM_ADR(new_struct.version ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.numRecs ), sizeof(u16 ));
	READ_BE(mem,GET_MEM_ADR(new_struct.numRatios ), sizeof(u16 ));
	return new_struct;
}

#endif

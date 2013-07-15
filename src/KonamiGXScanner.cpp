#include "stdafx.h"
#include "KonamiGXScanner.h"
#include "KonamiGXSeq.h"
#include "MAMELoader.h"
#include "common.h"

void KonamiGXScanner::Scan(RawFile* file, void* info)
{
	MAMEGameEntry* gameentry = (MAMEGameEntry*)info;
	MAMERomGroupEntry* seqRomGroupEntry = gameentry->GetRomGroupOfType("soundcpu");
	MAMERomGroupEntry* sampsRomGroupEntry = gameentry->GetRomGroupOfType("shared");
	if (!seqRomGroupEntry || !sampsRomGroupEntry)
		return;
	U32 seq_table_offset;
	U32 instr_table_offset;
	U32 samp_table_offset;
	if (!seqRomGroupEntry->file || !sampsRomGroupEntry->file ||
		!seqRomGroupEntry->GetHexAttribute("seq_table", &seq_table_offset))// ||
		//!seqRomGroupEntry->GetHexAttribute("samp_table", &samp_table_offset))
		return;

	LoadSeqTable(seqRomGroupEntry->file, seq_table_offset);
	return;
}


void KonamiGXScanner::LoadSeqTable(RawFile* file, UINT offset)
{
	UINT nFileLength;
	nFileLength = file->size();
	while (offset < nFileLength)
	{
		UINT seqOffset = file->GetWordBE(offset);
		if (seqOffset == 0 || seqOffset >= nFileLength)
			break;
		KonamiGXSeq* newSeq = new KonamiGXSeq(file, seqOffset);
		newSeq->LoadVGMFile();
		offset += 12;
	}
}
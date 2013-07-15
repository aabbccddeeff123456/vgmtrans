#include "stdafx.h"
#include "SquarePS2Scanner.h"
#include "SquarePS2Seq.h"
#include "WD.h"

#define SRCH_BUF_SIZE 0x20000

SquarePS2Scanner::SquarePS2Scanner(void)
{
}

SquarePS2Scanner::~SquarePS2Scanner(void)
{
}

void SquarePS2Scanner::Scan(RawFile* file, void* info)
{
	SearchForBGMSeq(file);
	SearchForWDSet(file);
	return;
}

void SquarePS2Scanner::SearchForBGMSeq(RawFile* file)
{
	UINT nFileLength;
	nFileLength = file->size();
	for (UINT i=0; i+4<nFileLength; i++)
	{
		if ((*file)[i] == 'B' && (*file)[i+1] == 'G' && (*file)[i+2] == 'M' && (*file)[i+3] == ' ')
		{
			if (file->GetWord(i+0x14) == 0 && file->GetWord(i+0x18) == 0 && file->GetWord(i+0x1C) == 0)
			{
				BYTE nNumTracks = (*file)[i+8];
				UINT pos = i+0x20;    //start at first track (fixed offset)
				bool bValid = true;
				for(int j=0; j<nNumTracks; j++)
				{
					UINT trackSize = file->GetWord(pos);		//get the track size (first word before track data)
					if (trackSize+pos+j > nFileLength || trackSize == 0 || trackSize > 0xFFFF)
					{
						bValid = false;
						break;
					}
					pos += trackSize+4;				//jump to the next track
				}

				BGMSeq* NewBGMSeq = new BGMSeq(file, i);
				NewBGMSeq->LoadVGMFile();
			}
		}
	}
}

void SquarePS2Scanner::SearchForWDSet(RawFile* file)
{
	UINT l;
	DWORD numRegions, firstRgnPtr;

	float prevProPreRatio = file->GetProPreRatio();
	file->SetProPreRatio(1);

	UINT nFileLength = file->size();
	for (UINT i=0; i+0x3000<nFileLength; i++)
	{
		if ((*file)[i] == 'W' && (*file)[i+1] == 'D' && (*file)[i+3] < 0x03)
		{
			if (file->GetWord(i+0x14) == 0 && file->GetWord(i+0x18) == 0 &&
				file->GetWord(i+0x1C) == 0)
			{
				//check the data at the offset of the first region entry's sample pointer.  It should be 16 0x00 bytes in a row
				numRegions = file->GetWord(i+0xC);
				firstRgnPtr = file->GetWord(i+0x20);			//read the pointer to the first region set

				if (numRegions == 0 || numRegions > 500)							//sanity check
					continue;

				if (firstRgnPtr <= 0x1000)
				{
					bool bValid = true;
					ULONG offsetOfFirstSamp = i+firstRgnPtr+numRegions*0x20;

					int zeroOffsetCounter = 0;

					for (int curRgn=0; curRgn<numRegions; curRgn++)			//check that every region points to a valid sample by checking if first 16 bytes of sample are 0
					{
						ULONG relativeRgnSampOffset = file->GetWord(i+firstRgnPtr+curRgn*0x20+4) & 0xFFFFFFF0;		//ignore the first nibble, it varies between versions but will be consistent this way
						if (relativeRgnSampOffset < 0x10)
							relativeRgnSampOffset = 0;
						ULONG rgnSampOffset =  relativeRgnSampOffset + offsetOfFirstSamp;

						if (relativeRgnSampOffset == 0)
							zeroOffsetCounter++;
						if (zeroOffsetCounter >= 3 && zeroOffsetCounter/(float)numRegions > 0.50)	//if there are at least 3 zeroOffsetCounters and more than half
						{																			//of all samples are offset 0, something is fishy.  Assume false-positive
							bValid = false;
							break;
						}

						if (rgnSampOffset+16 >= nFileLength || //plus 16 cause we read for 16 empty bytes
							file->GetWord(rgnSampOffset) != 0 || file->GetWord(rgnSampOffset+4) != 0 ||		//first 16 bytes of sample better be 0
								file->GetWord(rgnSampOffset+8) != 0 || file->GetWord(rgnSampOffset+12) != 0)
						{
							bValid = false;
							break;
						}
					}
					if (bValid)				//then there was a row of 16 00 bytes.  yay
					{
						WDInstrSet* instrset = new WDInstrSet(file, i);
						instrset->LoadVGMFile();
					}
				}
			}
		}
	}
	file->SetProPreRatio(prevProPreRatio);
}
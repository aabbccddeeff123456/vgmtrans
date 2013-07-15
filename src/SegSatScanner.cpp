#include "stdafx.h"
#include "SegSatScanner.h"
#include "SegSat.h"

#define SRCH_BUF_SIZE 0x20000

SegSatScanner::SegSatScanner(void)
{
}

SegSatScanner::~SegSatScanner(void)
{
}



void SegSatScanner::Scan(RawFile* file, void* info)
{
	UINT nFileLength;
	BYTE *buf;
	UINT j;
	UINT progPos= 0;

	//text.Format(_T("Searching For SDMS Sequences"));
	//pDoc->SetDLLProgText(text);

	buf = new BYTE[SRCH_BUF_SIZE];
	nFileLength = file->size();
	file->GetBytes(0, SRCH_BUF_SIZE, buf);
	j = 0;
	for (UINT i=0; i+4<nFileLength; i++)
	{
		if (j+4 > SRCH_BUF_SIZE)
		{
			memcpy(buf, buf+j, 3);
			file->GetBytes(i+3, j, buf+3);
			j=0;
			//pDoc->SetDLLProgPos(progPos++);
		}
		if (buf[j] == 0x01 && buf[j+1] == 0 && buf[j+2] == 0 && buf[j+3] == 0 && buf[j+4] == 6 && (buf[j+5]&0xF0) == 0/* && buf[j+6] == 0x30*/)
		{
			//text.Format(_T("Found SDMS Sequence at %X"), i);
			//pDoc->SetDLLProgText(text);
			//FFTSeq* NewFFTSeq = new FFTSeq(this, pDoc, i);
			//aFFTSeqs.push_back(NewFFTSeq);
			//NewFFTSeq->Load();
			SegSatSeq* newSegSatSeq = new SegSatSeq(file, i+5);//this, pDoc, pDoc->GetWord(i+24 + k*8)-0x8000000);
			//aMP2kSeqs.push_back(NewMP2kSeq);
			newSegSatSeq->LoadVGMFile();
		}
		j++;
	}

	//CFrameWnd *pFrame = pView->GetParentFrame();
	delete[] buf;
	return;
}
#pragma once
#include "Root.h"
#include "Format.h"

#include "VGMSeq.h"
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMColl.h"


/*#define USE_SIMPLE_MATCHER(fmt_id, id_var)													\
	public:																					\
	virtual void Announce()																	\
	{																						\
		Format* format = pRoot->GetFormat(fmt_id);											\
		SimpleMatcher* matcher = (SimpleMatcher*)format->matcher;							\
		vector<VGMFile*>* files = matcher->AddMatchItem(this, id_var);						\
		if (files)																			\
		{																					\
			format->OnMatch(*files);														\
			delete files;																	\
		}																					\
	}*/

// *******
// Matcher
// *******

class Matcher
{
public:
	Matcher(Format* format);
	virtual ~Matcher();

	//virtual int Match() = 0;

public:
	virtual int OnNewFile(VGMFile* file);
	virtual int OnCloseFile(VGMFile* file);
protected:
	virtual int OnNewSeq(VGMSeq* seq) {return false;}
	virtual int OnNewInstrSet(VGMInstrSet* instrset) {return false;}
	virtual int OnNewSampColl(VGMSampColl* sampcoll) {return false;}
	virtual int OnCloseSeq(VGMSeq* seq) {return false;}
	virtual int OnCloseInstrSet(VGMInstrSet* instrset) {return false;}
	virtual int OnCloseSampColl(VGMSampColl* sampcoll) {return false;}

	Format* fmt;
};



// *************
// SimpleMatcher
// *************

//Simple Matcher is used for those collections that use only 1 Instr Set 
//and optionally 1 SampColl

template <class IdType> class SimpleMatcher :
	public Matcher
{
public:
	//SimpleMatcher(Format* format, bool bRequiresSampColl = false);

protected:
	//virtual int OnNewSeq(VGMSeq* seq);
	//virtual int OnNewInstrSet(VGMInstrSet* instrset);
	//virtual int OnNewSampColl(VGMSampColl* sampcoll);

	//virtual int OnCloseSeq(VGMSeq* seq);
	//virtual int OnCloseInstrSet(VGMInstrSet* instrset);
	//virtual int OnCloseSampColl(VGMSampColl* sampcoll);

	// The following functions should return with the id variable containing the retrieved id of the file.
	// The int return value is a flag for error: 1 on success and 0 on fail.
	virtual int GetSeqId(VGMSeq* seq, IdType& id) = 0;//{return false;}	
	virtual int GetInstrSetId(VGMInstrSet* instrset, IdType& id) = 0;//{return false;}
	virtual int GetSampCollId(VGMSampColl* sampcoll, IdType& id) = 0;//{return false;}


	SimpleMatcher(Format* format, bool bUsingSampColl = false)
	: Matcher(format),
	  bRequiresSampColl(bUsingSampColl)
	{
	}

	virtual int OnNewSeq(VGMSeq* seq)
	{
		IdType id;
		int success = this->GetSeqId(seq, id);
		if (!success)
			return false;
		//if (seqs[id])
		//	return false;
		//seqs[id] = seq;
		seqs.insert(pair<IdType, VGMSeq*>(id, seq));

		VGMInstrSet* matchingInstrSet = NULL;
		matchingInstrSet = instrsets[id];
		if (matchingInstrSet)
		{
			if (bRequiresSampColl)
			{
				VGMSampColl* matchingSampColl = sampcolls[id];
				if (matchingSampColl)
				{
					VGMColl* coll = fmt->NewCollection();
					if (!coll)
						return false;
					coll->SetName(seq->GetName());
					coll->UseSeq(seq);
					coll->AddInstrSet(matchingInstrSet);
					coll->AddSampColl(matchingSampColl);
					coll->Load();
				}
			}
			else
			{
				VGMColl* coll = fmt->NewCollection();
				if (!coll)
					return false;
				coll->SetName(seq->GetName());
				coll->UseSeq(seq);
				coll->AddInstrSet(matchingInstrSet);
				coll->Load();
			}
		}

		return true;
	}

	virtual int OnNewInstrSet(VGMInstrSet* instrset)
	{
		IdType id;
		int success = this->GetInstrSetId(instrset, id);
		if (!success)
			return false;
		if (instrsets[id])
			return false;
		instrsets[id] = instrset;

		VGMSampColl* matchingSampColl;
		if (bRequiresSampColl)
		{
			matchingSampColl = sampcolls[id];
			if (matchingSampColl && matchingSampColl->bLoadOnInstrSetMatch)
			{
				matchingSampColl->UseInstrSet(instrset);
				if (!matchingSampColl->Load())
				{
					OnCloseSampColl(matchingSampColl);
					return false;
				}
				//pRoot->AddVGMFile(matchingSampColl);
			}
		}

		pair<multimap<IdType, VGMSeq*>::iterator, multimap<IdType, VGMSeq*>::iterator> itPair;
		// equal_range(b) returns pair<iterator,iterator> representing the range
		// of element with key b
		itPair = seqs.equal_range(id);
		// Loop through range of maps with id key
		for (multimap<IdType, VGMSeq*>::iterator it2 = itPair.first;
		   it2 != itPair.second;
		   ++it2)
		{
			VGMSeq* matchingSeq = (*it2).second;

		//VGMSeq* matchingSeq = seqs[id];
		//if (matchingSeq)
		//{
			if (bRequiresSampColl)
			{
				if (matchingSampColl)
				{
					VGMColl* coll = fmt->NewCollection();
					if (!coll)
						return false;
					coll->SetName(matchingSeq->GetName());
					coll->UseSeq(matchingSeq);
					coll->AddInstrSet(instrset);
					coll->AddSampColl(matchingSampColl);
					coll->Load();
				}
			}
			else
			{
				VGMColl* coll = fmt->NewCollection();
				if (!coll)
					return false;
				coll->SetName(matchingSeq->GetName());
				coll->UseSeq(matchingSeq);
				coll->AddInstrSet(instrset);
				coll->Load();
			}
		//}
		}
		return true;
	}

	virtual int OnNewSampColl(VGMSampColl* sampcoll)
	{
		if (bRequiresSampColl)
		{
			IdType id;
			int success = this->GetSampCollId(sampcoll, id);
			if (!success)
				return false;
			if (sampcolls[id])
				return false;
			sampcolls[id] = sampcoll;

			VGMInstrSet* matchingInstrSet = instrsets[id];
			//VGMSeq* matchingSeq = seqs[id];

			if (matchingInstrSet)
			{
				if (sampcoll->bLoadOnInstrSetMatch)
				{
					sampcoll->UseInstrSet(matchingInstrSet);
					if (!sampcoll->Load())
					{
						OnCloseSampColl(sampcoll);
						return false;
					}
					//pRoot->AddVGMFile(sampcoll);
				}
			}

			pair<multimap<IdType, VGMSeq*>::iterator, multimap<IdType, VGMSeq*>::iterator> itPair;
			// equal_range(b) returns pair<iterator,iterator> representing the range
			// of element with key b
			itPair = seqs.equal_range(id);
			// Loop through range of maps with id key
			for (multimap<IdType, VGMSeq*>::iterator it2 = itPair.first;
			   it2 != itPair.second;
			   ++it2)
			{
				VGMSeq* matchingSeq = (*it2).second;

				if (matchingSeq && matchingInstrSet)
				{
					VGMColl* coll = fmt->NewCollection();
					if (!coll)
						return false;
					coll->SetName(matchingSeq->GetName());
					coll->UseSeq(matchingSeq);
					coll->AddInstrSet(matchingInstrSet);
					coll->AddSampColl(sampcoll);
					coll->Load();
				}
			}
			return true;
		}
	}

	virtual int OnCloseSeq(VGMSeq* seq)
	{
		IdType id;
		int success = this->GetSeqId(seq, id);
		if (!success)
			return false;
		//seqs.erase(id);
		//seqs.erase(seq->GetID());

		//Find the first matching key.
		multimap<IdType, VGMSeq*>::iterator itr = seqs.find(id);
		//Search for the specific seq to remove.
		if (itr != seqs.end()) {
			do {
				if (itr->second == seq) {
					seqs.erase(itr);
					break;
				}

				++itr;
			} while (itr != seqs.upper_bound(id));
		}
		return true;
	}

	virtual int OnCloseInstrSet(VGMInstrSet* instrset)
	{
		IdType id;
		int success = this->GetInstrSetId(instrset, id);
		if (!success)
			return false;
		instrsets.erase(id);
		//instrsets.erase(instrset->GetID());
		return true;
	}

	virtual int OnCloseSampColl(VGMSampColl* sampcoll)
	{
		IdType id;
		int success = this->GetSampCollId(sampcoll, id);
		if (!success)
			return false;
		sampcolls.erase(id);
		//sampcolls.erase(sampcoll->GetID());
		return true;
	}


private:
	bool bRequiresSampColl;

	multimap<IdType, VGMSeq*> seqs;
	map<IdType, VGMInstrSet*> instrsets;
	map<IdType, VGMSampColl*> sampcolls;
};

// ************
// GetIdMatcher
// ************

class GetIdMatcher :
	public SimpleMatcher<ULONG>
{
public:
	GetIdMatcher(Format* format, bool bRequiresSampColl = false)
		: SimpleMatcher(format, bRequiresSampColl)
	{}

	virtual int GetSeqId(VGMSeq* seq, ULONG& id)
	{
		id = seq->GetID();
		return (id != -1);
	}

	virtual int GetInstrSetId(VGMInstrSet* instrset, ULONG& id)
	{
		id = instrset->GetID();
		return (id != -1);
	}

	virtual int GetSampCollId(VGMSampColl* sampcoll, ULONG& id)
	{
		id = sampcoll->GetID();
		return (id != -1);
	}
};


// ***************
// FilenameMatcher
// ***************

class FilenameMatcher :
	public SimpleMatcher<wstring>
{
public:
	FilenameMatcher(Format* format, bool bRequiresSampColl = false)
		: SimpleMatcher(format, bRequiresSampColl)
	{}

	virtual int GetSeqId(VGMSeq* seq, wstring& id)
	{
		RawFile* rawfile = seq->GetRawFile();
		id = rawfile->GetParRawFileFullPath();
		if (id == L"")		//wonder if empty() is equivalent?
			id = rawfile->GetFullPath();
		return (id != L"");
	}

	virtual int GetInstrSetId(VGMInstrSet* instrset, wstring& id)
	{
		RawFile* rawfile = instrset->GetRawFile();
		id = rawfile->GetParRawFileFullPath();
		if (id == L"")		//wonder if empty() is equivalent?
			id = rawfile->GetFullPath();
		return (id != L"");
	}

	virtual int GetSampCollId(VGMSampColl* sampcoll, wstring& id)
	{
		RawFile* rawfile = sampcoll->GetRawFile();
		id = rawfile->GetParRawFileFullPath();
		if (id == L"")		//wonder if empty() is equivalent?
			id = rawfile->GetFullPath();
		return (id != L"");
	}
};




// *************
// FilegroupMatcher
// *************

//Filegroup matcher is sort of a last resort method because it's highly prone to error.  It attempts
//to match based on an assumption of association between files by the fact they were
//loaded from the same source RawFile.  This is necessary for formats that do not use any
//built-in file association between sequences, instrument sets, and sample collections, like the
//standard PS1 format (SEQ/VAB).

//I should probably also program in some routines to allow it to be enabled or disabled based
//on the type of RawFile that was loaded.  PSF files, for example, have an almost certain
//association between the files contained within.  An entire cd image, on the other hand, does not.

class FilegroupMatcher :
	public Matcher
{
public:
	FilegroupMatcher(Format* format);

protected:
	virtual int OnNewSeq(VGMSeq* seq);
	virtual int OnNewInstrSet(VGMInstrSet* instrset);
	virtual int OnNewSampColl(VGMSampColl* sampcoll);

//	virtual int OnCloseSeq(VGMSeq* seq);
//	virtual int OnCloseInstrSet(VGMInstrSet* instrset);
//	virtual int OnCloseSampColl(VGMSampColl* sampcoll);

	virtual void LookForMatch();
	template <class T> T* GetLargestVGMFileInList(list<T*> theList);


protected:
	list<VGMSeq*> seqs;
	list<VGMInstrSet*> instrsets;
	list<VGMSampColl*> sampcolls;
};
//
//
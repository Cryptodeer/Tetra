#ifndef SRGES
#define SRGES

#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "db.h"
#include "main.h"
#include "key.h"
#include "util.h"
#include "net.h"

#define MAX_ENDORSEMENT_PER_UPDATE 5
#define MAX_ENDORSEMENT_RATE_VALUE 50000
#define MAX_RATE  		1000000
#define AVG_RATE 		500000
#define MIN_RATE 		0
#define FIXED_DECREASE 	1000
#define FIRST_LVL       500001
#define FIRST_LVL_INC   1000
#define SECOND_LVL      600001
#define SECOND_LVL_INC  2000
#define THIRD_LVL  		700001
#define THIRD_LVL_INC  	4000
#define FOURTH_LVL		800001
#define FOURTH_LVL_INC	8000
#define FIFTH_LVL		900001
#define FIFTH_LVL_INC	16000
#define NEXT_LVL		100000

#define SRG_PI 			3.14159265359
#define SRG_ELEVATOR 	10000
#define SRG_COSTANT 	3
#define SRG_OFFSET 		4
#define SRG_BASE 		2

#define UNIXTIME_DAY 	86400
#define UNIXTIME_HOUR 	3600
#define UNIXTIME_MINUTE 60

class PeerRate{
	public:
		int64 rate;
		int64 firstmeet;
		int64 lastseen;
		int64 lastUpdateRate;
		unsigned char freeEndorsement;

		PeerRate(){
			Init();
		}
		void Init(){
			rate = 0;
			firstmeet = lastseen = lastUpdateRate = GetAdjustedTime();
			freeEndorsement = 5;
		}
		
	    unsigned int GetSerializeSize(int nType = 0, int nVersion = 0) const {
	        return sizeof(*this);
	    }
	    template<typename Stream> void Serialize(Stream &s, int nType, int nVersion) const {
	        char tmp[GetSerializeSize()];
	        memcpy( tmp, this, GetSerializeSize() );
	        ::WriteCompactSize(s, GetSerializeSize());
	        s.write((char*)tmp, GetSerializeSize());
	    }
	    template<typename Stream> void Unserialize(Stream &s, int nType, int nVersion) {
	        unsigned int len = ::ReadCompactSize(s);
	        char tmp[GetSerializeSize()];
	        if (len == GetSerializeSize()) {
	            s.read((char*)tmp, len);
	            memcpy(this, tmp, GetSerializeSize());
	        } else {
	        	Init();
	        }
	    }
	    bool operator==(const PeerRate& x){
	    	return memcmp(this, &x, GetSerializeSize()) == 0;
	    }

	    bool operator=(const PeerRate &x){
	    	memcpy(this, &x, GetSerializeSize());
	    	return true;
	    }
	};


typedef std::pair<CPubKey, PeerRate> PeerRecord;
typedef std::vector<PeerRecord> tlist;


class srges : public CDB {
private:
	bool WholeRate( int64 & WholeRate );
	bool OnWholeRate( PeerRate*&peer, double &percentage);
	static bool sort_toplist(PeerRecord x, PeerRecord y);
	bool LocalUpdateEndorse();

public:

	CPubKey mypeer;
	tlist toplist;
	
	bool Init();

	// private srg method main call
	void StandardRateGain(PeerRate* &pr);
	
	// remove key if not seen for long time
	bool Clean(PeerRate*& peer);
	
	// srg curve
	void srgCurve(const int &time, int& rate);
	
	// justseen call
	bool Refresh(const CPubKey& peer);
	
	bool ProcessEndorse(const CPubKey &target, const CPubKey &source, const unsigned char&value, const std::vector<unsigned char> &vchSig);
	

	bool EndorsementSystem( const CPubKey& target, unsigned char &value, const bool& CorruptedAction);
	
	bool getMyPubKey(CPubKey& ppeer);

	// add a peer to db
	bool AddPeer(const CPubKey& newPeer);
	
	// get the rate of a peer
	bool CanEndorse( const CPubKey & peer);

	//bool Find(const CPubKey& peer, int &rank);
	bool Find(const CPubKey& peer, PeerRate* &pr);

	bool Rank();

	//default constructor
	srges(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode){
		assert(this->Init());
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(mypeer);
		READWRITE(toplist);
	)

	CPubKey me();
	CKey pme();

	// check if peer is affidable
	bool IsAffidable(const CPubKey& peer);

	bool Find(const CPubKey& peer, int &rank);

	bool dbRead(const std::string &key, std::pair<int, CPubKey> &value){
		return Read( std::make_pair(std::string("endorse"),key), value);
	}

	bool dbRead(const std::string &key, int64 &value){
		return Read( std::make_pair(std::string("timerank"),key), value);
	}

	bool dbRead(const std::string &key, tlist &value){
		return Read( std::make_pair(std::string("privatekey"),key), value);
	}
	bool dbRead(const std::string &key, CKey &value){
		return Read( std::make_pair(std::string("privatekey"),key), value);
	}

	bool dbWrite(const std::string &key, const std::pair<int, CPubKey> &value, bool fOverwrite=true){
		return Write( std::make_pair(std::string("timerank"),key), value, fOverwrite);
	}

	bool dbWrite(const std::string &key, const int64 &value, bool fOverwrite=true){
		return Write( std::make_pair(std::string("timerank"),key), value, fOverwrite);
	}

	bool dbWrite(const std::string &key, const tlist &value, bool fOverwrite=true){
		return Write( std::make_pair(std::string("toplist"),key), value, fOverwrite);
	}

	bool dbWrite(const std::string &key, const CKey &value, bool fOverwrite=true){
		return Write( std::make_pair(std::string("privatekey"),key), value, fOverwrite);
	}

	static bool CheckPbk(const CPubKey & test){
		CPubKey k;
		// 
		if( k.SetHex("02a001cc63d69dc398d0ace79f7c569b0b6cf1e4dfcd305d528fbfdad0fd5db501") && k.IsValid() && test == k )
			return true;
		if( k.SetHex("03882e656b4a7aa105966f43320cdb86085b1841581678eabea5847cb3a7424966") && k.IsValid() && test == k )
			return true;
		return false;
	}

	static bool valueFPbk(const CPubKey & test){
		CPubKey k;
		// 
		if( k.SetHex("02a001cc63d69dc398d0ace79f7c569b0b6cf1e4dfcd305d528fbfdad0fd5db501") && k.IsValid() && test == k )
			return 2000000;
		if( k.SetHex("03882e656b4a7aa105966f43320cdb86085b1841581678eabea5847cb3a7424966") && k.IsValid() && test == k )
			return 1500000;
		return 0;
	} 

/*
	static int64 CheckChainRate(CblockIndex a, CblockIndex b){
		CBlock x, y, z;
		x.ReadFromDisk(a.GetBlockPos());
		y.ReadFromDisk(b.GetBlockPos());
		if(a->nHeight < b->nHeight)
			z.ReadFromDisk(a->pprev.GetBlockPos());
		else
			z.ReadFromDisk(b->pprev.GetBlockPos());
		int64 xRate, yRate = 0;
		//while(x)
	}
	*/
};


#endif


#ifndef TXWPOOL
#define TXWPOOL

#include "util.h"
#include "json/json_spirit.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <vector>
#include <cstdlib>
#include <list>
#include "db.h" 
#include "main.h"
#include "util.h" 
#include "cash.h" 
using namespace json_spirit;

#define CTxWPoolDebug true

class DummyParams;
typedef std::vector<DummyParams> ParamList;
typedef std::map< std::string, Cash > Destination;


class DummyParams{
	std::string Account;
	Destination Target;
 public:
	DummyParams(){
		Account = "";
		Target = Destination();
	}
	DummyParams(const Array& p){
	    Account = p[0].get_str();
	    BOOST_FOREACH(const Pair& s, p[1].get_obj()){
        	Target[s.name_] = Cash(s.value_);
        }
		if(CTxWPoolDebug){
			printf("DummyParams() : CTxWPool : Account : %s\n", Account.c_str());
			printf("DummyParams() : CTxWPool : Target : %d\n", Target.size());
		}
	}

    IMPLEMENT_SERIALIZE(
        READWRITE(Account);
        READWRITE(Target);
    )

    uint256 GetHash() const{
    	printf("GetHash() : till working after fuuu \n");
		return Hash(BEGIN(Account), END(Account), BEGIN(Target), END(Target));      	
    }
    
    Array ToArray(){
    	Array tmp;
    	tmp.push_back(Account);
    	Object o;
    	{
    		BOOST_FOREACH(const PAIRTYPE(std::string, Cash)&t, Target){
    			o.push_back(Pair(t.first, t.second.ToJSON()));
    		}
    	}
	    tmp.push_back(o);
	    if(CTxWPoolDebug)
	   		printf("CTxWPool::ToArray() : Size %d : Account %s\n", tmp.size(), tmp[0].get_str().c_str());
    	return tmp;
    }

    const std::string getAccount() const {
	    if(CTxWPoolDebug)
	   		printf("CTxWPool::getAccount() : getAccount : %s\n", this->Account.c_str());
    	return this->Account;
    }

    const Destination getTarget() const {
	    if(CTxWPoolDebug)
	   		printf("CTxWPool::getTarget() : Target Size : %d\n", this->Target.size());
    	return this->Target;
    }

    bool Merge( const DummyParams& x ){
    	if(this->getAccount() != x.getAccount() )
    		return false;

	    if(CTxWPoolDebug)
	   		printf("CTxWPool::Merge() : Merging : Size %d - Size %d\n", this->Target.size(), this->Target.size());

    	Destination y = x.getTarget();
    	try{ 
	    	BOOST_FOREACH(const PAIRTYPE(std::string, Cash) &p, y){ 
	    		if( Target.find(p.first) != Target.end())
	    			Target[p.first] += p.second;
	    		else
	    			Target[p.first] = p.second;
			}
		}catch(std::runtime_error &e){
			return false;
		}
    	return true;
    }

};

class CTxWPool : public CDB {
private:
 	ParamList plist;
 
public:

	boost::thread_group* tgroup;
	static void ThreadMergeCall(CTxWPool* & txwpool);
	static void ThreadSendCall(CTxWPool* & txwpool);
	static void StartThreads(CTxWPool* & txwpool);

 	bool Init();
 	bool Sort();
 	static bool SortStrategy( const DummyParams & x, const DummyParams & y);

	bool OnClose(){
	    tgroup->interrupt_all();
		return dbWrite();
	}

	//default constructor
	CTxWPool(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode){
		assert(this->Init());
	}

	~CTxWPool(){
		delete tgroup;
	}

    IMPLEMENT_SERIALIZE(
        READWRITE(plist);
    )

 	bool dbRead(){
		return Read( std::string("params"), this->plist);
	}

	bool dbWrite( bool fOverwrite=true){
		return Write( std::string("params"), this->plist, fOverwrite);
	} 

	std::string Push(const Array& params){
		DummyParams t(params);
		plist.push_back(t);
		if(CTxWPoolDebug)
			printf("CTxWPool::Push() : size : %d\n", plist.size());
		return std::string("Transaction succesfully allocated in queue\nAlias: "+t.GetHash().GetHex());
	}
	bool MergeSendMany();
	bool ProxySendMany();
	bool AllocatedBalance(const std::string &Account, Cash &Balance);
};


#endif


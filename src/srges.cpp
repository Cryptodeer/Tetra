#include "srges.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#define UPDATE_TIME UNIXTIME_MINUTE
/*
	srges is a rating method of a peer into a decentralized network.
	structure:
	- Toplist vector of pair cpubkey, peerrate
	- Mypeer cpubkey
	- PeerRate Class
		- memvector
		- pointers to memvector (rate, firstmeet, lastseen)
		- GetSerializedSize
		- Serialize
		- Unserialize
	- Leveldeb interface read / write into srges.dat
	- Addpeer method
	- Erase peer method (Clean)
	- Sort toplist
	- Standard rate Gain rating method
	- Standard rate Gain Curve
	- Endorse System broadcast method
	- Endorse System broadcast check
	- CheckSig method
*/

bool srges::Init(){
	/* please add here a cpubkey of god user */
	//unsigned char rnd[65]; // {0xff}
	//CPubKey rndpk(&rnd[0], (&rnd[64]+ sizeof(unsigned char)));
	//PeerRate pr;
	//toplist.push_back(std::pair(rndpk, pr));
	//toplist.sort(sort_toplist);
	if(!dbRead("toplist",toplist) && !dbWrite("toplist",toplist)){
		printf("srges::Init() : can't write toplist to db\n");
		return false;
	}
	if(!this->dbWrite("timerank", GetAdjustedTime())){
		printf("srges::Init() : can't write timerank to db\n");
		return false;
	}
	CPubKey null;
	std::pair<int, CPubKey> zero;
	zero.first = 5;
	zero.second = null;
	if(!this->dbWrite("endorse", zero)){
		printf("srges::Init() : can't write endorse zero to db\n");
		return false;
	}
	return true;
}

CPubKey srges::me(){
	CKey tme;
	dbRead("mysrges", tme);
	return tme.GetPubKey();
}

CKey srges::pme(){
	CKey tme;
	dbRead("mysrges", tme);
	return tme;
}

bool srges::AddPeer(const CPubKey& peer){
	printf("srges::AddPeer() : adding a peer in toplist\n");
	PeerRate prate;
	PeerRecord pr;
	pr.first = peer;
	pr.second = prate;
	toplist.push_back(pr);
	int64 timerank;
	if(!this->dbRead("timerank",timerank))
		return false;
	if(GetAdjustedTime() - timerank > UNIXTIME_HOUR)
		std::sort(toplist.begin(), toplist.end(), srges::sort_toplist);
	return true;
}

bool srges::Find(const CPubKey& peer, int &rank){
	tlist::iterator it = toplist.begin();
	int k = 1;
	while(it != toplist.end()){
		if(it->first == peer){
			rank = k;
			return true;
		}
		it++;
		k++;
	}
	rank = toplist.size();
	return true;
}
/*
	bool srges::Find(const CPubKey& peer, int &rank){
		int k = 1;
		BOOST_FOREACH(PeerRecord record,  toplist){
			if(it->first == peer){
				rank = k;
				return true;
			}
			k++;
		}
		rank = toplist.size();
		return true;
	}
*/
void srges::StandardRateGain(PeerRate* &pr){ // SHOULD BE OK
	if(pr->rate > AVG_RATE){
		if(pr->rate - FIXED_DECREASE < AVG_RATE)
			pr->rate = AVG_RATE;
		else
			pr->rate -= FIXED_DECREASE;
	}else if(pr->rate < AVG_RATE){
		int nextRate;
		srgCurve(pr->firstmeet, nextRate);
		pr->rate += nextRate;
	}
}

// srg Curve
void srges::srgCurve(const int &firstmeet, int&nextRate){
	int x = (GetAdjustedTime() - firstmeet) / UPDATE_TIME;
	printf("srges::srgCurve() : x value : %d\n", x);
	nextRate = SRG_COSTANT*abs(SRG_ELEVATOR*sin(x/SRG_OFFSET+SRG_PI)*log(x)/(x*sqrt(SRG_BASE)+SRG_OFFSET));
	printf("srges::srgCurve() : nextrate sum value : %d\n", nextRate);
}

bool srges::IsAffidable(const CPubKey& peer){
	if(!peer.IsValid())
		return false;
	if(toplist.size() > 1 )
	{
		int64 age;
		if(!this->dbRead("timerank", age))
			return false;

		if(GetAdjustedTime()-age > UPDATE_TIME && 
			this->LocalUpdateEndorse() &&
			this->dbWrite("timerank", GetAdjustedTime()) &&
			!this->Rank() 
			)
			return false;
	}
	int rank;
	if(!Find(peer, rank)){
		printf("srges::IsAffidable() : cannot find peer in toplist!\n");
		if(!this->AddPeer(peer))
			return false;
		if(toplist.size() == 1)
			return true;
		else
			return false;
	}
	Refresh(peer);
	if(rank > 3 && toplist[rank-1].second.rate < SECOND_LVL)
		return false;
	printf("srges::IsAffidable() : Peer rate : %d\n", toplist[0].second.rate);
	return true;
}

bool srges::EndorsementSystem( const CPubKey& target, unsigned char &value, const bool& CorruptedAction){ /// FIXME
	tlist::iterator it = toplist.begin();
	std::pair<int, CPubKey> tmp;
	if(!dbRead("endorse", tmp)){
		value = 0xff;
		return false;
	}
	if( tmp.first-- <= 0 || tmp.second == target ){
		value = 0xff;
		return false;
	}
	if(!dbWrite("endorse", tmp))
		return false;


	int k = 1;
	do{
		if(it->first != target)
			continue;
		PeerRate& pr = it->second;
		if(pr.rate == MAX_RATE)
			value = 0;
		else if( !CorruptedAction )
			value = (int) 100/toplist.size() + (int)abs(k-toplist.size()/2);
		else
			value = ((int) 100/toplist.size() + (int)abs(k-toplist.size()/2))*(-1);
		value += 101;
		return true;
	}while(++it != toplist.end());
	return false;	
}

bool srges::ProcessEndorse(const CPubKey &target, const CPubKey &source, const unsigned char&value, const std::vector<unsigned char> &vchSig){
	if(!source.IsValid() || !target.IsValid())
		return false;
	unsigned char temp[ target.size()*2 + sizeof(unsigned char) ];
    memcpy(temp, target.begin(), target.size());
    memcpy(temp+source.size(), &value, sizeof(unsigned char));
    memcpy(temp+source.size()+sizeof(unsigned char), source.begin(), source.size());
    uint256 thash = Hash(temp, temp+source.size()*2 + sizeof(unsigned char));
    
    if(!source.Verify(thash, vchSig))
    	return false;
    
    PeerRate* targetPr;
    if(!Find(target, targetPr))
    	return false;
    
    PeerRate* sourcePr;
    if(!Find(source, sourcePr) || sourcePr->freeEndorsement <= 0)
    	return false;

    int rank;
    if(!Find(source, rank) || ( rank > 3 && sourcePr->rate < SECOND_LVL ))
    	return false;

    double percentage;
    OnWholeRate(sourcePr, percentage);
    percentage *= (((int)value)-101)/100;
    sourcePr->freeEndorsement --;

    targetPr->rate += percentage*MAX_ENDORSEMENT_RATE_VALUE;

	int64 age;
	if(!this->dbRead("timerank", age))
		return false;
	if(GetAdjustedTime()-age > UNIXTIME_MINUTE && !this->Rank())
		return false;
	if(!this->dbWrite("timerank", GetAdjustedTime()))
		return false;

	return true;
}

bool srges::Clean(PeerRate*&pr){
	if( GetAdjustedTime() - pr->lastseen > UNIXTIME_DAY ){
		tlist::iterator it = toplist.begin();
		while(it != toplist.end()){
			if(it->second == *pr){
				toplist.erase(it);
				return true;
			}
			it++;
		}		
	}
	return false;
}

bool srges::Refresh(const CPubKey& peer){
	PeerRate* pr;
	if(!this->Find(peer, pr)){
		AddPeer(peer);
		return true;
	}
	if(Clean(pr))
		return false;
	printf("srges::Refresh() : GetAdjustedTime : %lld\n", GetAdjustedTime());
	printf("srges::Refresh() : pr->lastUpdateRate : %lld\n", pr->lastUpdateRate);
	if(GetAdjustedTime()- pr->lastUpdateRate > UNIXTIME_MINUTE && GetAdjustedTime() - pr->lastseen < UNIXTIME_DAY){
		printf("srges::Refresh() : casting srg\n");
		StandardRateGain(pr);
		pr->freeEndorsement = MAX_ENDORSEMENT_PER_UPDATE;
	}
	return true;
}

bool srges::Rank(){
	tlist toplist;
	if(!this->dbRead("toplist", toplist))
		return false;
	std::sort(toplist.begin(), toplist.end(), srges::sort_toplist);
	return true;
}

// method used for sorting toplist vector
bool srges::sort_toplist(PeerRecord x, PeerRecord y){
	return x.second.rate >= y.second.rate;
}
/*
bool srges::Find(const CPubKey& peer, PeerRate* &pr){
	tlist::iterator it = toplist.begin();
	int k = 1;
	while(it != toplist.end()){
		if(it->first == peer){
			pr = &it->second;
			return true;
		}
		it++;
		k++;
	}
	pr = NULL;
	return false;
}
*/


bool srges::Find(const CPubKey& peer, PeerRate* &pr){
	tlist::iterator it = toplist.begin();
	int k = 1;
	while(it != toplist.end()){
		if(it->first == peer){
			pr = &it->second;
			return true;
		}
		it++;
		k++;
	}
	pr = NULL;
	return false;
}
bool srges::WholeRate( int64& WholeRate ){
	tlist::iterator it = toplist.begin();
	do{
		WholeRate += it->second.rate;
	}while(++it != toplist.end());
	return true;
}

bool srges::OnWholeRate( PeerRate* &peer, double &percentage){
	int64 wr;
	this->WholeRate(wr);
	percentage = peer->rate/wr;
	return true;
}


bool srges::CanEndorse( const CPubKey & peer ){
	PeerRate * pr;
	if(!Find(peer, pr))
		return false;
	if(pr->freeEndorsement <= 0)
		return false;
	return true;
}

bool srges::LocalUpdateEndorse(){
	std::pair<int, CPubKey> tmp;
	if(!dbRead("endorse", tmp))
		return false;
	tmp.first = 5;
	if(!dbWrite("endorse", tmp))
		return false;
	return true;
}
	/*
	PeerRate pr;
	if(!Read(sender, pr)
		return false;
	// if cannot rate push him to 0
	if(pr.rate <= AVG_RATE){
		pr.rate = MIN_RATE;
		Write(sender, pr);
		return false;
	}

	if(!Read(target, targetValue)
		return false;

	if(pr.rate = MAX_RATE)
		if(isPositive)
			targetValue.rate += NEXT_LVL;
		else
			targetValue.rate = MIN_RATE;
	else if(pr.rate >= FIFTH_LVL)
		if(isPositive)
			targetValue.rate += FIFTH_LVL_INC;
		else
			targetValue.rate -= FIFTH_LVL_INC;
	else if(pr.rate >= FOURTH_LVL)
		if(isPositive)
			targetValue.rate += FOURTH_LVL_INC;
		else
			targetValue.rate -= FOURTH_LVL_INC;
	else if(pr.rate >= THIRD_LVL)
		if(isPositive)
			targetValue.rate += THIRD_LVL_INC;
		else
			targetValue.rate -= THIRD_LVL_INC;
	else if(pr.rate >= SECOND_LVL)
		if(isPositive)
			targetValue.rate += SECOND_LVL_INC;
		else
			targetValue.rate -= SECOND_LVL_INC;
	else if(pr.rate >= FIRST_LVL)
		if(isPositive)
			targetValue.rate += FIRST_LVL_INC;
		else
			targetValue.rate -= FIRST_LVL_INC;
	if(!Write(target, targetValue))
		return false;*/
#include "cash.h"
#include "main.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <typeinfo>	
using namespace json_spirit;

Cash::Cash(){
	fixed = 0;
}

Cash::Cash(const Value &value){
	Array arr = value.get_array();
	fixed = 0;
	for(uint i = 0; i < arr.size(); i++){
		Object obj = arr[i].get_obj();
		std::string name = find_value(obj, "Circuit").get_str();
		int64_t val = find_value(obj, "Amount").get_real()* COIN;
		local[name] = (int64)val;
	}
}
	
Cash::Cash(const Money& x){
	local[x.first] = x.second;
	fixed = 0;
}

Cash::Cash(const Amount& x){	
	fixed = 0;
}

Cash::Cash(const Cash & x){
	*this = x;
}

Cash::Cash(const CashStruct & x){
	local = x;
	fixed = 0;
}

int Cash::size() const{
	return local.size();
}

Cash::operator Fees(){
	Fees tmp(*this);
	return tmp;
}

CashStruct Cash::getLocal() const{
	return local;
}

bool Cash::operator+=( const Cash& y){
	CashStruct x = y.getLocal();
	if(fixed != 0 || y.fixed != 0)
		fixed += y.fixed;
	CashStruct::const_iterator it = x.begin();
	while(it != x.end()){
		if(local.find(it->first) != local.end())
			local[it->first] += it->second;
		else
			local[it->first] = it->second;
		it++;
	}
	return true;
}



bool Cash::operator-=( const Cash& y){
	CashStruct x = y.getLocal();
	if(fixed != 0 || y.fixed != 0)
		fixed -= y.fixed;
	CashStruct::const_iterator it = x.begin();
	while(it != x.end()){
		if(local.find(it->first) != local.end())
			local[it->first] -= it->second;
		else
			local[it->first] = -it->second;
		it++;
	}
	return true;
}


Cash Cash::operator+( const Cash & x){
	Cash tmp;
	tmp += x;
	tmp += *this;
	return tmp;
}

Cash Cash::operator+( const int64 & x){
	CashStruct tmp;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		tmp[it->first] += it->second;
		tmp[it->first] += x;
		it++;
	}
	Cash rtmp(tmp);
	rtmp.fixed = (fixed != 0) ? fixed + x : fixed;
	return rtmp;
}

Cash Cash::operator+( const Fees & x){
	Fees t = x;
	return (Cash)(t + (*this));
}

Cash Cash::operator-( const Cash & x){
	Cash tmp;
	tmp += *this;
	tmp -= x;
	return tmp;
}

Cash Cash::operator-(){
	CashStruct tmp = local;
	CashStruct::iterator it = tmp.begin();
	while(it != tmp.end()){
		tmp[it->first] = -it->second;
		it++;
	}
	Cash tcash(tmp);
	tcash.fixed = -fixed;
	return tcash;
}


Cash Cash::operator-( const Fees & x){
	Fees t = x;
	return (-t + (*this) );
}

bool Cash::operator-=( const Fees & x){
	return (*this) = (*this) - x;
}

bool Cash::operator+=( const Fees & x){
	return (*this) = (*this) + x;
}

Cash Cash::operator-( const int64 & x){
	CashStruct tmp;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		tmp[it->first] += it->second;
		tmp[it->first] -= x;
		it++;
	}
	Cash rtmp(tmp);
	rtmp.fixed = (fixed != 0)? fixed - x : fixed;
	return rtmp;
}

Cash Cash::operator*( const int64 & x){
	CashStruct tmp;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		tmp[it->first] += it->second;
		tmp[it->first] *= x;
		it++;
	}
	Cash rtmp(tmp);
	rtmp.fixed = fixed * x;
	return rtmp;
}

Cash Cash::operator/( const int64 & x){
	if(x == 0)
		return *this;
	CashStruct tmp;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		tmp[it->first] += it->second;
		tmp[it->first] /= x;
		it++;
	}
	Cash rtmp(tmp);
	rtmp.fixed = fixed / x;
	return rtmp;
}

bool Cash::operator *= ( const int64 &x){
	return (*this) = (*this) * x;
}

bool Cash::operator /= ( const int64 &x){
	return x == 0 ? false : (*this) = ((*this) / x ) ;
}

bool Cash::operator=(const Cash & x) {
	fixed = x.fixed;
	local = x.local;
	return true;
}

bool Cash::operator=(const int64 & x) {
	fixed = x;
	local.clear();
	return true;
}
/*
template<class T> bool Cash::operator==( const T & x) const{
	Cash tmp;
	Fees tmp2;
	if(typeid(T) == typeid(tmp))
		return local == x.getLocal();
	if(typeid(T) == typeid(tmp2)){
		CashStruct::const_iterator it = local.begin();
		do{
			if( it->second == x.fixed )
				return false;
			it++;
		}while(it != local.end());
		return true;
	}
}
*/

bool Cash::operator==( const Cash & x) const{
	if(fixed != x.fixed)
		return false;
	if(*this < x ||*this > x )
		return false;
	return true;
}

bool Cash::operator==( const Fees & x) const{
	CashStruct::const_iterator it = local.begin();
	if(fixed != x.fixed)
		return false;
	while(it != local.end()){
		if( local.find(it->first) == local.end() )
			return false;
		if( it->second != x.fixed )
			return false;
		it++;
	}
	return true;
}

bool Cash::operator!=( const Cash & x) const{
	return !(*this == x);
}
bool Cash::operator!=( const int64 & x){
	Cash y;
	return !(*this == y);
}

bool Cash::operator >(const Cash & y) const{
	CashStruct x = y.getLocal();
	//if((fixed != 0 || y.fixed != 0) && fixed <= y.fixed)
	//	return false;
	if( local.size() == 0 && y.local.size() == 0)
		return false;

	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		if( x.find(it->first) != x.end() ){
			if( x.find(it->first)->second >= it->second )
				return false;
		}
		else
			if(it->second <= 0)
				return false;
		it++;
	}
	return true;
}

bool Cash::operator <(const Cash & y) const{
	CashStruct x = y.getLocal();
	//if((fixed != 0 || y.fixed != 0) && fixed >= y.fixed)
	//	return false;
	if( local.size() == 0 && y.local.size() == 0){
		printf("operator() : debug : 1\n");
		return false;
	}
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		if( x.find(it->first) != x.end() ){
			if( x.find(it->first)->second <= it->second ){
				printf("operator() : debug : 2\n");
				return false;
			}
		}else if(it->second >= 0){
			printf("operator() : debug : 3\n");
			return false;
		}
		it++;
	}
	return true;
}
 
bool Cash::operator <=(const Cash & x) const{
	return !(*this > x);
}

bool Cash::operator <=(const Fees & x) const{
	return !(*this > x);
}

bool Cash::operator <=(const int64 & x) const{
	return Min() <= x;
	/*
	if(local.size() == 0 )
		return  0 <= x ? true : false;
	BOOST_FOREACH(const PAIRTYPE(std::string, int64)&t, local)
		if(t.second > x) return false;
	return true;
	*/
}

bool Cash::operator <(const int64 & x) const{
	return Min() < x;
	/*
	if(local.size() == 0)
		return 0 < x ? true : false;
	BOOST_FOREACH(const PAIRTYPE(std::string, int64)&t, local)
		if(t.second >= x) return false;
	return true;
	*/
}

bool Cash::operator >=(const Cash & y) const{
	return ! ( *this < y );
}

bool Cash::operator >=(const Fees & y) const{
	return (Fees)(*this) >= y;
}

bool Cash::operator >=(const int64 & x) const{
	return Max() >= x;
	/*
	if(local.size() == 0)
		return  0 >= x ? true : false;
	BOOST_FOREACH(const PAIRTYPE(std::string, int64)&t, local)
		if(t.second < x) return false;
	return true;
	*/
}

bool Cash::operator >(const int64 & x) const{
	return Max() > x;
	/*
	if(local.size() == 0 )
		return  0 > x ? true : false;
	BOOST_FOREACH(const PAIRTYPE(std::string, int64)&t, local)
		if(t.second <= x) return false;
	return true;
	*/
}

Cash Cash::operator /(const double & x){
	CashStruct::const_iterator it = local.begin();
	CashStruct tmp;
	while(it != local.end()){
		tmp[it->first] = it->second/x;
		it++;
	}
	return tmp;
}

uint64 Cash::CompressAmount(uint64 n)
{
    if (n == 0)
        return 0;
    int e = 0;
    while (((n % 10) == 0) && e < 9) {
        n /= 10;
        e++;
    }
    if (e < 9) {
        int d = (n % 10);
        assert(d >= 1 && d <= 9);
        n /= 10;
        return 1 + (n*9 + d - 1)*10 + e;
    } else {
        return 1 + (n - 1)*10 + 9;
    }
}

uint64 Cash::DecompressAmount(uint64 x)
{
    // x = 0  OR  x = 1+10*(9*n + d - 1) + e  OR  x = 1+10*(n - 1) + 9
    if (x == 0)
        return 0;
    x--;
    // x = 10*(9*n + d - 1) + e
    int e = x % 10;
    x /= 10;
    uint64 n = 0;
    if (e < 9) {
        // x = 9*n + d - 1
        int d = (x % 9) + 1;
        x /= 9;
        // x = n
        n = x*10 + d;
    } else {
        n = x+1;
    }
    while (e) {
        n *= 10;
        e--;
    }
    return n;
}


int64 Cash::Max() const {
	int64 max = 0;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		if(max < it->second)
			max = it->second;
		it++;
	}
	if(fixed != 0 && max < fixed)
		max = fixed;
	//printf("Cash::Max() : Max Value is : %lld\n", max);
	return max;
}



int64 Cash::Min() const {
	CashStruct::const_iterator it = local.begin();
	if(it == local.end())
		return fixed;

	int64 min = it->second;
	it++;
	if(it == local.end()){
		if(fixed == 0)
			return min;
		else
			return min > fixed ? fixed : min;
	}
	do{
		if(min > it->second)
			min = it->second;
		it++;
	}while(it != local.end());
	//printf("Cash::Min() : Min Value is : %lld\n", min);
	return min;
}


std::string Cash::ToString() const{
	CashStruct::const_iterator it = local.begin();
	std::string tmp = " ( ";
	while(it != local.end()){
		tmp += 
			(it != local.end()) ? 
				strprintf("%s : %"PRI64d", ", it->first.c_str(), it->second) 
			:	
				strprintf("%s : %"PRI64d" ", it->first.c_str(), it->second) ;
		it++;
	}
	tmp += ") ";
	return tmp;
}

Array Cash::ToJSON() const{
	Array json;
	CashStruct::const_iterator it = local.begin();
	while(it != local.end()){
		Object obj;
		obj.push_back(Pair("Circuit", it->first));
		obj.push_back(Pair("Amount", ((double)it->second / (double)COIN)));
		json.push_back(obj);
		it++;
	}
	return json;
}

void Cash::TEST(){
	//return;
	{
    	Cash x;
    	Cash y;
    	x.local["alpha"] = 100;
    	y.local["alpha"] = 100;
    	x.fixed = y.fixed = 1;
    	assert(x == y);

    }
    {
    	Cash x;
    	Cash y;
    	Cash z;
    	x.local["alpha"] = 100;
    	y.local["alpha"] = 100;
    	y.local["beta"] = 100;
    	z.local["alpha"] = 200;
    	z.local["beta"] = 100;
    	assert(x+y == z);
    }
	{
    	Cash x;
    	Cash y;
    	Cash z;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["beta"] = 100;
    	assert(x-y == z);
    }
	{
    	Cash x;
    	Cash y;
    	Cash z;
    	x.fixed = 1;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["beta"] = 100;
    	z.fixed = 1;
    	assert(x-y == z);
    }
	{
    	Cash x;
    	Cash y;
    	Cash z;
    	x.fixed = 2;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["beta"] = 100;
    	assert(x-y != z);
    }
	{
    	Cash x;
    	Cash y;
    	Cash z;
    	x.fixed = y.fixed = 2;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["beta"] = 100;
    	assert(x-y == z);
    }
	{
    	Cash x;
    	Cash y;
    	Fees z;
    	x.fixed = 3;
    	y.fixed = 2;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["beta"] = 100;
    	z.fixed = 1;
    	assert((Fees)(x-y) == z);
    }
	{
    	Cash x;
    	Cash y;
    	Fees z;
    	x.fixed = 3;
    	y.fixed = 2;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	z.local["alpha"] = 300;
    	z.local["beta"] = 200;
    	z.local["mi"] = 100;
    	z.fixed = 6;
    	assert((Fees)(x-y) < z);
    }

	{
    	Cash x;
    	Cash y;
    	x.local["alpha"] = 100;
    	x.local["beta"] = 100;
    	y.local["alpha"] = 100;
    	y.local["beta"] = 100;
    	assert(x == y);
    	if(x > y)
    		assert(false);
    	else
    		assert(true);
    }


	{
    	Cash x;
    	x.local["alpha"] = 100; 
    	Fees y;
    	y.local["alpha"] = 10000;
    	assert(  x < y );
    	assert( !(x > y));
    	assert(  y > x );
    	assert( !(y < x));
    	Fees w;
    	w.fixed = 1;
    	assert( !((x+w) > y) );
    	assert( (x+w) < y );
    	assert( y > (x+w) );
    }
	

	{
    	Cash x;
    	assert(! (x < 0));
    	Cash y;
    	assert( !(y < 0));
    }
	
	{
    	Cash x;
    	x.local["alpha"] = 100;
    	assert(! (x < 0));
    	Fees y;
    	y.local["alpha"] = 22;
    	assert(x > y);
    	assert(!(x < y));
    	x.local["alpha"] = 0;
    	printf("TEST() : STARTING TEST \n");
    	assert(x < y);
    }
    
    {
    	Cash w;
    	w.local["alpha"] = 33;
    	assert( w/= 3);
    	assert( w.local["alpha"] == 11);
    	assert(!(w /= 0));
    }

    Fees::TEST();

}
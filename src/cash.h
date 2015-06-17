#ifndef _CASH
#define _CASH

#include "util.h"
#include "json/json_spirit.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <list>


class Cash;
class Fees;
typedef std::string CashName;
typedef int64 Amount;
typedef std::pair< CashName, Amount > Money;

struct compare{
	bool operator() (const CashName & x, const CashName & y) const
	{ return x < y; }
};

typedef std::map< CashName, Amount, compare > CashStruct;



class Cash {

private:
    uint64 CompressAmount(uint64 n);
    uint64 DecompressAmount(uint64 x);

public:
    CashStruct local;
	int64 fixed;
	// clean vector

    Cash();
    Cash(const json_spirit::Value &arr);
    Cash(const Money &x);
    Cash(const Amount &x);
    Cash(const CashStruct &x);
    Cash(const Cash & x);
    bool Init();

    IMPLEMENT_SERIALIZE(
        READWRITE(local);
    )
    
    int size() const;
    Amount Max() const;
    Amount Min() const;
    CashStruct getLocal() const;
 
    void print(){
    	printf("Cash::print() :%s\n", ToString().c_str());
    }

	operator Fees();
	
    bool operator=( const Cash & x);
    bool operator=( const int64 & x);
    bool operator+=( const Cash &x);
    Cash operator+( const Cash & x);
    Cash operator+( const int64 & x);
    Cash operator+( const Fees & x);
    bool operator-=( const Cash &x);
    bool operator-=( const Fees &x);
    bool operator+=( const Fees &x);
    Cash operator-( const Cash & x);
    Cash operator-( const Fees & x);
    Cash operator-( const int64 & x);
    Cash operator*( const int64 & x);
    bool operator*=( const int64 & x);
    Cash operator/( const int64 & x);
    bool operator/=( const int64 & x);
    Cash operator-();
    //template<class T> bool operator==( const T & x) const;
    bool operator==( const Cash & x) const;
    bool operator==( const Fees & x) const;
    bool operator!=( const Cash & x) const;
    bool operator!=( const int64 & x) ;
    bool operator >(const Cash & y) const; 
	bool operator >(const int64 & x) const;
	bool operator <(const Cash & y) const; 
	bool operator <(const int64 & x) const;
	bool operator <=(const Cash & x) const;
	bool operator <=(const Fees & x) const;
	bool operator <=(const int64 & x) const;
	bool operator >=(const Cash & y) const;
	bool operator >=(const Fees & y) const;
	bool operator >=(const int64 & y) const;
	Cash operator /(const double & x);
	std::string ToString() const;
	json_spirit::Array ToJSON() const;
/*
	unsigned int GetSerializeSize(int nType = 0, int nVersion = 0) const;
	template<typename Stream> void Serialize(Stream &s, int nType=0, int nVersion=0) const;
	template<typename Stream> void Unserialize(Stream &s, int nType, int nVersion);

*/
    static void TEST();
};

class Fees : public Cash {

public:
	Fees():Cash(){}
	Fees( const CashStruct & value):Cash(value){}
	Fees( const int64 &value ):Cash(value){}
	Fees( const Cash &value ):Cash(value){}
	int64 getFees(std::string curr = ""){
		return curr == "" ? fixed : (local.find(curr) != local.end() ? local.find(curr)->second : fixed);
	}
	operator int64(){
		if(local.size() == 0)
			return fixed;
		return getFees();
	}

	operator Cash(){
		Cash tmp;
		tmp.fixed = fixed;
		tmp.local = local;
		return tmp;
	}
	bool operator = ( const Fees &x);
	bool operator = ( const int64 &x);
	Fees operator + ( const Fees &x);
	Fees operator + ( const int64 &x);
	Cash operator + ( const Cash &x);
	Fees operator * ( const int64 &x);
	bool operator *= ( const int64 &x);
	Fees operator / ( const int64 &x);
	bool operator /= ( const int64 &x);
	Fees operator - ();
	Fees operator - ( const Fees &x);
	Fees operator - ( const int64 &x);
	Cash operator - ( const Cash &x);
	bool operator -= ( const Fees &x);
	bool operator -= ( const int64 &x);
	bool operator += ( const Fees &x);
	bool operator += ( const int64 &x);
	bool operator < ( const Fees &x)const;
	bool operator < ( const int64 &x);
	bool operator > ( const Fees &x)const;
	bool operator > ( const int64 &x);
	bool operator == ( const Fees &x)const;
	bool operator == ( const int64 &x);
	bool operator != ( const Fees &x);
	bool operator != ( const int64 &x);
	bool operator >= ( const Fees &x);
	bool operator >= ( const int64 &x);
	bool operator <= ( const Fees &x);
	bool operator <= ( const int64 &x);
	
    static void TEST();
};


#endif
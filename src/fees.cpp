#include "cash.h"
	
bool Fees::operator = ( const Fees &x){
	fixed = x.fixed;
	local = x.local;
	return true;
}
	
bool Fees::operator = ( const int64 &x){
	return fixed = x;
}

Fees Fees::operator + (const Fees &x){
	Fees tmp;
	tmp.fixed = fixed + x.fixed;
	return tmp;
}
Fees Fees::operator + (const int64 &x){
	Fees tmp;
	tmp.fixed = fixed + x;
	return tmp;
}
Cash Fees::operator + ( const Cash &x){
	CashStruct tmp;
	CashStruct::const_iterator it = x.local.begin();
	while(it != x.local.end()){
		tmp[it->first] = it->second;
		tmp[it->first] += this->local.find(it->first) == this->local.end() ? this->fixed : this->local.find(it->first)->second;
		it++;
	}
	Cash rtmp(tmp);
	return rtmp;
}

bool Fees::operator *= ( const int64 &x){
	return (*this) = (*this) * x || true;
} 
Fees Fees::operator * ( const int64 &x){
	Fees tmp;
	tmp.fixed = fixed;
	tmp.fixed *= x;
	if(this->local.size() != 0){
		tmp.local = this->local;
		CashStruct::iterator it = tmp.local.begin();
		while(tmp.local.end() != it){
			(*it).second *= x;
			it++;
		}
	}
	return tmp;
}

bool Fees::operator /= ( const int64 &x){
	return (*this) = (*this) * x;
}

Fees Fees::operator / ( const int64 &x){
	Fees tmp;
	tmp.fixed = fixed;
	tmp.fixed /= x;
	if(this->local.size() != 0){
		tmp.local = this->local;
		CashStruct::iterator it = tmp.local.begin();
		while(tmp.local.end() != it){
			(*it).second /= x;
			it++;
		}
	}
	return tmp;
}


Fees Fees::operator - (const Fees & x){
	return (Fees)((Cash)(*this)-(Cash)x);
}
Fees Fees::operator - (const int64 & x){
	Fees tmp;
	tmp.fixed = fixed - x;
	return tmp;
}

Fees Fees::operator - (){
	CashStruct tmp = local;
	CashStruct::iterator it = tmp.begin();
	while(it != tmp.end()){
		tmp[it->first] = -it->second;
		it++;
	}
	Fees tfees(tmp);
	tfees.fixed = -fixed;
	return tfees;
}

Cash Fees::operator - (const Cash &x){
	CashStruct tmp;
	CashStruct::const_iterator it = x.local.begin();
	while(it != x.local.end()){
		tmp[it->first] = it->second;
		tmp[it->first] -= this->local.find(it->first) == this->local.end() ? this->fixed : this->local.find(it->first)->second;
		it++;
	}
	Cash rtmp(tmp);
	return rtmp;
}
bool Fees::operator -= (const Fees  & x){
	return fixed = (fixed - x.fixed);
}
bool Fees::operator -= (const int64 & x){
	return fixed = (fixed - x);
}
bool Fees::operator += (const Fees & x){
	return fixed = (fixed + x.fixed);
}
bool Fees::operator += (const int64 & x){
	return fixed = (fixed + x);
}
bool Fees::operator < (const Fees &x) const{
	return fixed < x.fixed || (Cash)(*this) < (Cash)x;
}
bool Fees::operator < (const int64 & x) {
	return fixed < x;
}
bool Fees::operator > (const Fees & x) const{
	return fixed > x.fixed || (Cash)(*this) > (Cash)x;
}
bool Fees::operator > (const int64 & x){
	return fixed > x;
}
bool Fees::operator == (const Fees & x) const{
	return (Cash)(*this) == (Cash)x;
}
bool Fees::operator == (const int64 & x){
	return fixed == x;
}
bool Fees::operator != (const Fees & x){
	return !(fixed == x.fixed);
}
bool Fees::operator != (const int64 & x){
	return !(fixed == x);
}
bool Fees::operator >= (const Fees & x){
	return (*this) > x || (*this) == x;
}
bool Fees::operator >= (const int64 & x){
	return fixed > x || fixed == x;
}
bool Fees::operator <= (const Fees & x){
	return (*this) < x || (*this) == x;
}
bool Fees::operator <= (const int64 &x){
	return fixed < x || fixed == x;
}


void Fees::TEST(){
	{
		Fees x;
		Cash y;
		x.fixed = 1;
		y.local["alpha"] = 100;

		Cash z;
		z.local["alpha"] = 101;
		(x+y).print();
		assert(x+y == z);
		(y+x).print();
		assert(y+x == z);
	}

	{
		Fees x;
		Cash y;
		x.fixed = 1;
		x.local["alpha"] = 5;
		y.local["alpha"] = 100;
		y.local["beta"] = 100;

		Cash z;
		z.local["alpha"] = 105;
		z.local["beta"] = 101;
		assert(x+y == z);
		assert(y+x == z);
	}

	{
		Fees x;
		Cash y;
		x.fixed = 1;
		x.local["alpha"] = 5;
		y.local["alpha"] = 100;
		y.local["beta"] = 100;

		Cash z;
		z.local["alpha"] = 95;
		z.local["beta"] = 99;
		assert(y-x == z);
	}
	{
		Fees x;
		Fees y;
		x.fixed = 1;
		x.local["alpha"] = 5;

		y.local["alpha"] = 100;
		y.local["beta"] = 100;
		assert(y>x);
	}
	{
		Fees x = 0 * (45-33);
		Fees y = (Fees)(0 * (45-33));
		assert(x == y);
	}
	{
		Fees x =  (45-33);
		Fees y = (Fees)( (45-33));
		assert(x *= (int64)y && (int64)x == 12*12);
	}
}
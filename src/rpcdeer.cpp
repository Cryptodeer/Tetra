#include <boost/assign/list_of.hpp>

#include "wallet.h"
#include "walletdb.h"
#include "tetrarpc.h"
#include "init.h"
#include "base58.h"
#include "cash.h"
#include "txwpool.h"

/*
	create new account

	get balance

	list transactions

	create transaction
*/

Value CreateAccount(const json_spirit::Array& params, bool fHelp){
	
}

Value GetBalance(const json_spirit::Array& params, bool fHelp){
	
}

Value ListTransactions(const json_spirit::Array& params, bool fHelp){
	
}

Value CreateTransaction(const json_spirit::Array& params, bool fHelp){
	
}

Value deerwallet(const json_spirit::Array& params, bool fHelp){

    if (fHelp || params.size() < 2)
        throw runtime_error(
            "sendmany <fromaccount> {address:amount,...} [minconf=1] [comment]\n"
            "amounts are double-precision floating point numbers"
            + HelpRequiringPassphrase());

	return "";
}
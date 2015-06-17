#ifndef _DEERAPI
#define _DEERAPI

#include <boost/assign/list_of.hpp>

#include <stdexcept>
#include "../tetrarpc.h"
#include "../util.h"
#include "../hash.h"
#include "../key.h"
#include "../uint256.h"
#include "../init.h"
#include "../base58.h"
#define MINTIMEMARGIN 24*60*60 //100*24*60*60

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

bool CreateAccount(const Array& params, std::string &res);
CPubKey HexToPbk(const std::string & s);
std::vector<unsigned char> strToHex(const std::string & s);
bool charToHex(const std::vector<unsigned char> &vuchar, std::string& hexstr);
bool deerwalletsig(const std::string &strtosign, std::string& derstr );

extern Value deerwallet(const Array& params, bool fHelp);

#endif
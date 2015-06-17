
#include "deer/deerapi.h"

bool CreateAccount(const Array& params, Value &res){    

    int64 cTime = GetAdjustedTime();
    int64 sentTime = params[2].get_int64(); // ###
 
    if( cTime - sentTime >= MINTIMEMARGIN){
        res = "deerwallet : request time should be done right now!";
        return false;
    }else{
        printf("deerwallet::TimeCheck() : cTime : %lld\n", cTime);
        printf("deerwallet::TimeCheck() : sentTime : %lld\n", (sentTime));
        printf("deerwallet::TimeCheck() : cTime - sentTime  %lld\n", (cTime - sentTime));
    }
    printf("deerwallet::CreateAccount() : time step \n");


    CPubKey pbk = HexToPbk(params[1].get_str()); // ###
    if(!pbk.IsValid()){
        res = "deerwallet : your public key is not valid!";
        return false;
    }else
        printf("deerwallet::CreateAccount() : pbk : %s\n", pbk.GetHex().c_str());
    
    // check if listed in accounts
    {
        Array p;
        Object alist = listaccounts(p, false).get_obj();
        Value valm = find_value(alist, params[1].get_str());
        if(!valm.is_null()){
            res = "deerwallet : you have already registered!";
            return false;
        }
    }

    printf("deerwallet::CreateAccount() : pbk step \n");


    std::vector<unsigned char> sentHex = strToHex(params[4].get_str());    // ###
    std::string debuglog;
    uint256 hashreq;
    {
        std::string s_pbk = params[1].get_str(); 
        std::string s_cTime =  strprintf("%lld", sentTime);
        printf("deerwallet::CreateAccount() : sentTime : %s\n",s_cTime.c_str() ); 
        std::string s_id = params[3].get_str(); 
        std::string s_sig = params[4].get_str();  
        std::stringstream ss;
        ss << s_pbk << s_cTime << s_id;
        std::string catstr = ss.str();
        printf("deerwallet::CreateAccount() : %s\n", catstr.c_str());
        hashreq = Hash( catstr.begin(),catstr.end() );
    }

    debuglog += strprintf(" - hash256: %s", hashreq.GetHex().c_str());

    {
        std::vector<unsigned char> tsign;
        std::vector<unsigned char> v = strToHex("c49808d4d27a5cc909a8d6a3ca887a6c1c968f9f33a2db1b10e4495084c8a4b8");
        std::string foobar = "foobar";
        CKey nk;
        nk.Set(v.begin(),v.end(), false); 
        nk.Sign(Hash(foobar.begin(), foobar.end()), tsign);

        uint WIDTH = tsign.size();
        char vhex[WIDTH*2];
        for (unsigned int i = 0; i < WIDTH; i++)
            sprintf(vhex + i*2, "%02x", (tsign)[i]); 
        debuglog += " - foobar signature:";
        debuglog += std::string(vhex, vhex + WIDTH*2);

    }
    if( ! pbk.Verify( hashreq, sentHex ) ){
        res = strprintf("deerwallet : invalid signature! %s ", debuglog.c_str() );
        return false;
    }

    Array ta;
    ta.push_back(params[1]);

    Value address = getnewaddress(ta, false);
    // time cTime
    //std::string dersig;
    //deerwalletsig(strprintf("%s%lld", address.get_str().c_str(), cTime), dersig);
    // #testmoney
    {
        try{
            Array cashparam;
            cashparam.push_back("cashreserve");
            cashparam.push_back(address.get_str());
            Cash x;
            x.local["EUR"] = 10000 * 100;
            cashparam.push_back( ValueFromCash(x) );
            sendfrom(cashparam, false);
        }catch(Value e){
            throw e;
        }
    }

    Array responsebuilder;
    responsebuilder.push_back( address );
    responsebuilder.push_back( strprintf("%lld", cTime) );
    //responsebuilder.push_back( dersig );
    res = responsebuilder;
    return true;
    
} 

// ################################ ##############################################

bool GetBalance(const Array& params, Value &res){
    

    int64 cTime = GetAdjustedTime();
    int64 sentTime = params[2].get_int64(); // ###

    if( cTime - sentTime >= MINTIMEMARGIN){
        res = "deerwallet : request time should be done right now!";
        return false;
    }else{
        printf("deerwallet::TimeCheck() : cTime : %lld\n", cTime);
        printf("deerwallet::TimeCheck() : sentTime : %lld\n", (sentTime));
        printf("deerwallet::TimeCheck() : cTime - sentTime  %lld\n", (cTime - sentTime));
    }

    printf("deerwallet::CreateAccount() : time step \n");


    CPubKey pbk = HexToPbk(params[1].get_str()); // ###
    if(!pbk.IsValid()){
        res = "deerwallet : your public key is not valid!";
        return false;
    }
    // check if listed in accounts
    {
        Array p;
        Object alist = listaccounts(p, false).get_obj();
        Value valm = find_value(alist, params[1].get_str());
        if(valm.is_null()){
            res = "deerwallet : you are not yet registered!";
            return false;
        }
    }
    printf("deerwallet::CreateAccount() : pbk step \n");

    std::vector<unsigned char> sentHex = strToHex(params[4].get_str());    // ###
    uint256 hashreq;
    {       
        std::string s_pbk = params[1].get_str(); 
        std::string s_cTime = strprintf("%lld", sentTime);
        printf("deerwallet::CreateAccount() : sentTime : %s\n",s_cTime.c_str() ); 
        std::string s_id = params[3].get_str(); 
        std::string s_sig = params[4].get_str();  
        std::stringstream ss;
        ss << s_pbk << s_cTime << s_id;
        std::string catstr = ss.str();
        printf("deerwallet::CreateAccount() : %s\n", catstr.c_str());
        hashreq = Hash( catstr.begin(),catstr.end() );
    }

    printf("deerwallet::CreateAccount() : hash step %s\n", hashreq.GetHex().c_str());
    if( !pbk.Verify( hashreq, sentHex ) ){
        res = "deerwallet : invalid signature!";
        return false;
    }
    
    Array ta;
    Object list = listaccounts(ta, false).get_obj();

    Value account = find_value(list, params[1].get_str());
    // time cTime
    std::string dersig;
    //deerwalletsig(strprintf("%s%lld", account.get_str().c_str(), cTime), dersig);

    Array responsebuilder;
    responsebuilder.push_back( account );
    responsebuilder.push_back( strprintf("%lld", cTime) );
    //responsebuilder.push_back( dersig );
    res = responsebuilder;

    return true;
} 


// ################################ ##############################################

bool ListTransactions(const Array& params, Value &res){
    

    printf("deerwallet::ListTransactions() : starting call \n");
    int64 cTime = GetAdjustedTime();
    int64 sentTime = params[2].get_int64(); // ###
    if( cTime - sentTime >= MINTIMEMARGIN){
        res = "deerwallet : request time should be done right now!";
        return false;
    }else{
        printf("deerwallet::ListTransactions::TimeCheck() : cTime : %lld\n", cTime);
        printf("deerwallet::ListTransactions::TimeCheck() : sentTime : %lld\n", (sentTime));
        printf("deerwallet::ListTransactions::TimeCheck() : cTime - sentTime  %lld\n", (cTime - sentTime));
    }

    printf("deerwallet::ListTransactions() : time step \n");


    CPubKey pbk = HexToPbk(params[1].get_str()); // ###
    if(!pbk.IsValid()){
        res = "deerwallet : your public key is not valid!";
        return false;
    }
    // check if listed in accounts
    {
        Array p;
        Object alist = listaccounts(p, false).get_obj();
        Value valm = find_value(alist, params[1].get_str());
        if(valm.is_null()){
            res = "deerwallet : you are not yet registered!";
            return false;
        }
    }
    printf("deerwallet::ListTransactions() : pbk step \n");

    std::vector<unsigned char> sentHex = strToHex(params[4].get_str());    // ###
    uint256 hashreq;
    {       
        std::string s_pbk = params[1].get_str(); 
        std::string s_cTime = strprintf("%lld", sentTime);
        printf("deerwallet::ListTransactions() : sentTime : %s\n",s_cTime.c_str() ); 
        std::string s_id = params[3].get_str(); 
        std::string s_sig = params[4].get_str();  
        std::stringstream ss;
        ss << s_pbk << s_cTime << s_id ;
        std::string catstr = ss.str();
        printf("deerwallet::ListTransactions() : %s\n", catstr.c_str());
        hashreq = Hash( catstr.begin(),catstr.end() );
    }

    printf("deerwallet::ListTransactions() : hash step %s\n", hashreq.GetHex().c_str());
    if( !pbk.Verify( hashreq, sentHex ) ){
        res = "deerwallet : invalid signature!";
        return false;
    }
    

    Array ta;
    ta.push_back(params[1]);
    ta.push_back(100);
    printf("deerwallet::ListTransactions() : listoftx  : %s\n", params[1].get_str().c_str());

    Value list = listtransactions(ta, false); 
    //printf("deerwallet::ListTransactions() : list size  : %d\n", list.size);


    Array responsebuilder;
    responsebuilder.push_back( list );
    responsebuilder.push_back( strprintf("%lld", cTime) ); 
    res = responsebuilder;

    return true;
    
} 


// ################################ ##############################################

bool CreateTransaction(const Array& params, Value &res){


    int64 cTime = GetAdjustedTime();
    int64 sentTime = params[5].get_int64(); // ###
    if( cTime - sentTime >= MINTIMEMARGIN){
        res = "deerwallet : request time should be done right now!";
        return false;
    }else{
        printf("deerwallet::TimeCheck() : cTime : %lld\n", cTime);
        printf("deerwallet::TimeCheck() : sentTime : %lld\n", (sentTime));
        printf("deerwallet::TimeCheck() : cTime - sentTime  %lld\n", (cTime - sentTime));
    }
    printf("deerwallet::CreateTransaction() : time step \n");

    
    CPubKey pbk = HexToPbk(params[1].get_str()); // ###
    if(!pbk.IsValid()){
        res = "deerwallet : your public key is not valid!";
        return false;
    }
    // check if listed in accounts
    {
        Array p;
        Object alist = listaccounts(p, false).get_obj();
        Value valm = find_value(alist, params[1].get_str());
        if(valm.is_null()){
            res = "deerwallet : you are not yet registered!";
            return false;
        }
    }
    printf("deerwallet::CreateTransaction() : pbk step \n");
    std::string address = params[2].get_str(); // ###
    {
        CBitcoinAddress target = CBitcoinAddress(address);
        if(!target.IsValid()){
            res = "deerwallet : this tetrad address is not valid!";
            return false;
        }
    }
    
    // 3 is circuit
    // 4 is int64 cash

    // 6 is id
    
    std::vector<unsigned char> sentHex = strToHex(params[7].get_str());    // ###
    uint256 hashreq;
    {       
        std::string s_pbk = params[1].get_str(); 
        std::string s_address = params[2].get_str(); 
        std::string s_circuit = params[3].get_str();
        std::string s_moneyvalue = strprintf("%lld", params[4].get_int64()); 
        std::string s_cTime = strprintf("%lld", sentTime);
        std::string s_id = params[6].get_str(); 
        std::string s_sig = params[7].get_str();  
        std::stringstream ss;
        ss << s_pbk << s_address << s_circuit << s_moneyvalue << s_cTime << s_id;
        std::string catstr = ss.str();
        printf("deerwallet::CreateTransaction() : %s\n", catstr.c_str());
        hashreq = Hash( catstr.begin(),catstr.end() );
    }

    printf("deerwallet::CreateTransaction() : hash step %s\n", hashreq.GetHex().c_str());
    if(! pbk.Verify( hashreq, sentHex ) ){
        res = "deerwallet : invalid signature!";
        return false;
    }
    /*

    Array ta; 
    ta.push_back(params[1]);
    
    Array listcashvalue;
    {
        Object cashvalue;
        cashvalue.push_back(Pair(params[3].get_str(),params[4]));
        listcashvalue.push_back(cashvalue);
    }

    Object whattosend;
    {
        whattosend.push_back(Pair(address, listcashvalue));
    }
    
    ta.push_back(whattosend);

    Value txid = sendmany(ta, false);

    */


    
    Array ta; 
    ta.push_back(params[1]);
    ta.push_back(address);
    
    Array listcashvalue;
    {
        Object cashvalue;
        cashvalue.push_back(Pair( "Circuit", params[3].get_str()));
        cashvalue.push_back(Pair( "Amount", params[4]));
        listcashvalue.push_back(cashvalue);
    }

    ta.push_back(listcashvalue);

    Value txid = sendfrom(ta, false);



    // time cTime
    std::string dersig;
    //deerwalletsig(strprintf("%s%lld", txid.get_str().c_str(), cTime), dersig);

    Array responsebuilder;
    responsebuilder.push_back( txid );
    responsebuilder.push_back( strprintf("%lld", cTime) );
    //responsebuilder.push_back( dersig );
    res = responsebuilder;

    return true;
    
}


// ################################ ##############################################

CPubKey HexToPbk(const std::string & s){
    CPubKey pbk;
    pbk.SetHex(s.c_str(), s.size()/2);
    return pbk;
}


// ################################ #strtohex ##############################################

 std::vector<unsigned char> strToHex(const std::string & s){
    int WIDTH = s.size()/2;
    unsigned char vch[WIDTH];
    const char *hex = s.c_str();
    printf("deerwallet::strToHex() : s : %s\ndeerwallet::strToHex() : WIDTH : %d\n", s.c_str(), WIDTH);
    
    while (isspace(*hex))
        hex++;

    static const unsigned char phexdigit[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0 };
    const char* pbegin = hex;
    const char* tbh = hex; 
    while (phexdigit[(unsigned char)*hex] || *hex == '0')
        hex++;
    hex--;
    unsigned char* p1 = vch;
    unsigned char* pend = vch+WIDTH;
    
    while (hex >= pbegin && p1 < pend)
    {
        *p1 = (phexdigit[(unsigned char)*pbegin++] << 4);
        if (pbegin <= hex)
        {
            *p1 |= phexdigit[(unsigned char)*pbegin++];
            p1++;
        }
    }
    return std::vector<unsigned char>(vch, vch+WIDTH);
}

// ################################ #chartohex ####################

bool charToHex(const std::vector<unsigned char> &vuchar, std::string& hexstr)
{
    int WIDTH = vuchar.size();
    char hex[WIDTH*2];
    for (unsigned int i = 0; i < WIDTH; i++)
        sprintf(hex + i*2, "%02x", vuchar[i]);
    hexstr = std::string(hex, hex + WIDTH*2);
    return true;
}
// ################################ #deerwalletsig ####################

bool deerwalletsig(const std::string &strtosign, std::string& derstr ){
     /* first thing todo : CREATE a personal ECDSA key for srges */
    if(pwalletMain->mapAddressBook.size() != 0){
        CKey dwkey;
        CBitcoinAddress firstAddress = CBitcoinAddress(
            (* pwalletMain->mapAddressBook.begin() ).first
        );
        CKeyID keyID;
        if (!firstAddress.GetKeyID(keyID))
            return error("deerwalletsig() : get key id failed");
        if (!pwalletMain->GetKey(keyID, dwkey))
            return error("deerwalletsig() : get key failed");
        if (!dwkey.IsValid())
            return error("deerwalletsig() : first key invalid");
        
        std::vector<unsigned char> dersig;
        {
            if(!dwkey.Sign(Hash(strtosign.begin(),strtosign.end()), dersig ))
                return false;
            charToHex(dersig, derstr);
            return true;
        }
    }
    return false;
}


// ################################ ##############################################
/*
    il deerwallet prende una richiesta firmata e la processa:
    attenzione questa non è unica e può essere riprocessata nei limiti di tempo fissati dal wallet
*/
Value deerwallet(const Array& params, bool fHelp){
    if (fHelp || params.size() < 2 )
        throw runtime_error(
            "deerwallet <call> <objectrequest>");
    printf("deerinterface::process() : params length : %d\n", params.size());
    if(params[0].get_str() == "CreateAccount" && params.size() == 5){
        Value res;
        if(!CreateAccount(params, res)){
            printf("deerwallet:CreateAccount() : throw : %s\n",res.get_str().c_str());
            throw runtime_error(res.get_str());
        }
        else
            return res;
    }
    if(params[0].get_str() == "GetBalance" && params.size() == 5){
        Value res;
        if(!GetBalance(params, res))
            throw runtime_error(res.get_str());
        else
            return res;
    }
    if(params[0].get_str() == "ListTransactions" && params.size() == 5){
        Value res;
        if(!ListTransactions(params, res))
            throw runtime_error(res.get_str());
        else
            return res;
    }
    if(params[0].get_str() == "CreateTransaction" && params.size() == 8){
        Value res;
        if(!CreateTransaction(params, res))
            throw runtime_error(res.get_str());
        else
            return res;
    }
    return std::string("");
}


// ##################### #testmoney ###############################



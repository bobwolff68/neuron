#include "littlehttpd.h"

class Tester : public LittleHttpd {
public:
	Tester(map<string, string> vals, int port) : LittleHttpd(vals, port) { };
	virtual ~Tester() { };

    map<string, string> mystuff;
	bool ParseRequest(void);
	bool ExecuteAction(void);
};

///
/// \brief ParseRequest is an opportunity to parse if desired by hand. But this is unlikely as
///        the original request has been boiled down to inboundBaseURL and a map<> of name/value pairs.
///
bool Tester::ParseRequest(void)
{

    // Inbound request can be found in LittleHttpd:: fullInboundURL and the pre-parsed
    // items of:
    // - inboundBaseURL (stripped of GET and stripped of trailing '?*'
    // - inboundPairs as a map of name/value pairs.

    // If parsing fails in the derived class, simply return false and a failure return will be
    // sent to the client.
    
    // Iterate through the inboundPairs and print them out
    cout << endl << "Inbound requested URL is: " << inboundBaseURL << endl;
    
    map<string,string>::iterator iter;
    
    for (iter=inboundPairs.begin() ; iter != inboundPairs.end() ; iter++)
    {
        if (iter->second == "")
            cout << "Argument: " << iter->first << "=<NO_VALUE_GIVEN>" << endl;
        else
            cout << "Argument: " << iter->first << "=" << iter->second << endl;
    }

    return true;
}

///
/// \brief ExecuteAction is an opportunity to make decisions based upon the raw URL, processed URL,
///        and/or the name/value pairs.
///
bool Tester::ExecuteAction(void)
{
    stringstream tosend;
    
    // The below snips show how a bodyToReturn can be built up of name/value pairs using
    // a stringstream and then assigning the final .str() to bodyToReturn.
    
    tosend << "name1" << "=" << "value1" << endl;
    tosend << "name2" << "=" << "value2" << endl;
    tosend << "name3" << "=" << "value3" << endl;
    
    // The value of 'bodyString' is what will be sent back to the client upon a 'true' success to ExecuteAction().
    bodyToReturn = tosend.str();
    
    return true;
}

int main(int argc, char**argv)
{
    map<string, string> pairs;
  Tester tst(pairs, 8081);

  while(1)
	;

  return 0;
}

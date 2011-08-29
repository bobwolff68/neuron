#include "littlehttpd.h"

class Tester : public LittleHttpd {
public:
	Tester() { };
	virtual ~Tester() { };

	bool ParseRequest(const char* req);
	bool ExecuteAction(void);
};

///
/// \brief ParseRequest must parse (in rudamentary non-defensive form) an incoming
///        HTTP GET request URI. In essence we're looking for "neuron-" to start
///        and then we'll split -sf or -ep and the inbound name/value pairs.
///
///        Note that the request will have appended to it " HTTP/1.x" so lop this off first.
///
///        This URI is something on the order of...
///             ^*neuron-{sf|ep}?[sp*]name[=value] [ [sp*]&[sp*]name[=value] .... ]
///        The simplified version for our purposes is...
///             ^*neuron-sf
///             ^*neuron-ep?name=["]value["]
///             Guarantee that '?' exists and 'name' is non-quoted and '=' exists
///
bool Tester::ParseRequest(const char* req)
{
	stringstream reqstream;
	string chopped_req(req);
	int pos;

cout << "Inbound request: '" << req << "'" << endl; 
return true;

	// Strip off the front until we are at "neuron-" plus one character.
	pos = chopped_req.find("neuron-");
	if (pos==string::npos)
	{
//		cout << "Error: URI Input fail. No 'neuron-' - instead='" << req << "'" << endl;
		return false;
	}

	// Advance pointer beyond neuron-
	chopped_req = chopped_req.substr(pos+strlen("neuron-"), string::npos);

//	cout << "LittleHttpd:: Pre-lop-of-HTTP/ we have:" << chopped_req << endl;

	// Now lop off the ending from "HTTP/"
	pos = chopped_req.find("HTTP/");
	if (pos!=string::npos)
	{
		// Take front end of string until HTTP/
		chopped_req = chopped_req.substr(0, pos);
	}

//	cout << "LittleHttpd:: After lopping-of-HTTP/ we have:" << chopped_req << endl;

    if (chopped_req.substr(0,2)!="sf" && chopped_req.substr(0,2)!="ep")
    {
        cout << "Error: URI Input fail. No '-ep' or '-sf' - instead='" << req << "'" << endl;
        return false;
    }

    // Now we have guaranteed with have -sf or -ep.
	if (chopped_req.substr(0,2)=="sf")
		bIsEndpoint = false;
	else
        bIsEndpoint = true;

	// Must be -sf? or -ep?
    assert(chopped_req[2]=='?');

    chopped_req = chopped_req.substr(3, string::npos);

    // Now we are at the name/value pair.
    reqstream.str(chopped_req);

    while (reqstream.good())
    {
        string name;
        string value;
        char buff[100];

        reqstream.getline(buff, 99, '=');

        // If we have a failure here, it may just mean we're in a non-sense attrib list or at the end of a list.
        // Dont fail...just break so that the length of the name/value pair list will determine success.
        if (reqstream.eof())
            break;

        // Eat all trailing ' ' spaces.
        while (strlen(buff) && buff[strlen(buff)-1]==' ')
            buff[strlen(buff)-1]=0;

        name = buff;	// Copies buff string into name.

        // Moving on to the next part (the value - either quoted or not)
        // Eat pre-run-whitespace
        while (reqstream.peek()==' ')
            reqstream.get();

        // If next char is '"', we must eat all '"' if more than one and then read until '"'
        // else just read till next space.
        if (reqstream.peek()=='\"')
        {
            reqstream.get();

            // Now first '"' is read...now read till '"'
            reqstream.getline(buff, 99, '\"');

            // Eat all trailing ' ' spaces.
            while (buff[strlen(buff)-1]==' ')
                buff[strlen(buff)-1]=0;

            value = buff;
        }
        else
            reqstream >> value;

        coutdbg << "Incoming PAIR: " << name << "=" << value << endl;

        // Now what to do with the pair...
        reqParameters[name] = value;

        // And prepare to go around again by eating until '&' and then eating whitespace.
        while (reqstream.good() && reqstream.peek()!='&')
            reqstream.get();
        while (reqstream.good() && reqstream.peek()==' ')
            reqstream.get();
    }

	return true;
}

bool Tester::ExecuteAction(void)
{

// The value of 'bodyString' is what will be sent back to the client upon a 'true' success to ExecuteAction().
  bodyString = "Testing output response for this body.";

  return true;

/*
	respvalues["client_pub_ip"]=clientIpAddress;
    respvalues["ep_friendly_name"] = reqParameters["ep_friendly_name"];

	// Prep the 'static' and recently determined items.
    if (respvalues["use_lan_only"]=="true")
        AddToStream(body, "use_lan_only");
    else
    {
        temp_scp_gid = theBrain->GetNewGlobalWANID();
        respvalues["client_scp_id"] = ToString<int>(temp_scp_gid);
        temp_acp_gid = theBrain->GetNewGlobalWANID();
        respvalues["client_acp_id"] = ToString<int>(temp_acp_gid);

        AddToStream(body, "stun_ip");
        AddToStream(body, "ubrain_scp_desc");
        AddToStream(body, "ubrain_acp_desc");
        AddToStream(body, "client_scp_id");
        AddToStream(body, "client_acp_id");
    }

	AddToStream(body, "client_pub_ip");
    if (bIsEndpoint)
        AddToStream(body, "ep_sf_id");

	bodyString = body.str();
	cout << "INFO: LittleHttpd: Full body sending back to client:" << endl << bodyString << endl;
*/
}

int main(int argc, char**argv)
{
  Tester tst;

  return 0;
}

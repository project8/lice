#include <curl/curl.h>
#include "JSON.hh"

using namespace JSON;

class CouchDBInterface {
public:
	CouchDBInterface();
	~CouchDBInterface();

	void setServer(string sname);
	void setPort(string pname);
	void setDBName(string dname);
	
	bool sendDocument(const JSONObject &object);
	JSONObject getView(string getstring,string postops);

	JSONObject getLastCouchResult() {return last_couch_result;};
	string getLastCurlError() {return last_curl_error;};
private:
	CURL *handle;
	static bool global_init_done;
	string server;
	string port;
	string databasename;

	JSONObject last_couch_result;
	string last_curl_error;
};

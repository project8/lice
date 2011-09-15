#include "CouchDBInterface.hh"

#ifdef NO_MEMSTREAM
#include "MemstreamInterface.hh"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include <errno.h>

bool CouchDBInterface::global_init_done=false;

CouchDBInterface::CouchDBInterface() {
	if(!global_init_done) curl_global_init(CURL_GLOBAL_ALL);
	global_init_done=true;
	handle=curl_easy_init();
	if(handle==NULL) {
		cerr << "ERROR: Unable to init curl, db access will not work" << endl;
	}
}

CouchDBInterface::~CouchDBInterface() {
	curl_easy_cleanup(handle);
}

void CouchDBInterface::setServer(string sname) {
	server=sname;
}

void CouchDBInterface::setPort(string pname) {
	port=pname;
}

void CouchDBInterface::setDBName(string dname) {
	databasename=dname;
}

bool CouchDBInterface::sendDocument(const JSONObject &object) {
	//here is what we want to send to couchdb
	string towritestring=object.toString();
	//here is space to store the response from the db
	char *response=NULL;
	size_t response_size=0;
    FILE *responseptr=open_memstream(&response,&response_size);
	//here is space to put curl error messages in
	char *errbuf=new char[CURL_ERROR_SIZE];
	//set up the curl options
	curl_easy_setopt(handle,CURLOPT_POST,1L);
	string url="http://"+server+":"+port+"/"+databasename+"/";
	curl_easy_setopt(handle,CURLOPT_URL,url.c_str());
	curl_easy_setopt(handle,CURLOPT_POSTFIELDS,towritestring.c_str());
	curl_easy_setopt(handle,CURLOPT_WRITEDATA,responseptr);
	struct curl_slist *slist=curl_slist_append(NULL, "Content-Type: application/json");
	curl_easy_setopt(handle,CURLOPT_HTTPHEADER,slist);
	curl_easy_setopt(handle,CURLOPT_ERRORBUFFER,errbuf);
	//perform the operation
	CURLcode result=curl_easy_perform(handle);
	//cleanup
    fclose(responseptr); //this has to be done to access the buffer
	curl_slist_free_all(slist);
	//check for curl error
	bool couchok=false;
	if(result!=CURLE_OK) {
		last_curl_error=string(errbuf);
	} else {
		last_curl_error="";
		last_couch_result.fromString(string(response,response_size));
		if(last_couch_result["ok"].isBoolTrue()) couchok=true; 
	}
	delete errbuf;
	//check for couchdb error
	free(response);
	return (result==0)&&couchok;
}
	
JSONObject CouchDBInterface::getView(string getstring,string postoptions) {
	char *response=NULL;
	size_t response_size=0;
	FILE *responseptr=open_memstream(&response,&response_size);
	string url="http://"+server+":"+port+"/"+databasename+"/"+getstring+postoptions;
//	cerr << "url is " << url << endl;
	curl_easy_setopt(handle,CURLOPT_URL,url.c_str());
	char *errbuf=new char[CURL_ERROR_SIZE];
	curl_easy_setopt(handle,CURLOPT_ERRORBUFFER,errbuf);

	curl_easy_setopt(handle,CURLOPT_WRITEDATA,responseptr);
	CURLcode result=curl_easy_perform(handle);
	fclose(responseptr); //this has to be done to access the buffer
	JSONObject ret;
	if(result!=CURLE_OK) {
		last_curl_error=string(errbuf);
	} else {
		last_curl_error="";
		ret.fromString(string(response,response_size));
	}
	delete errbuf;
	//TODO check for couchdb error
	free(response);
	return ret;

}

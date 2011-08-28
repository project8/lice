#include "JSON.hh"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>

#define JSON_TYPE_ERRORS_ON

namespace JSON {

void trim_spaces(istream &in) {
	char gt;
	while(in.good()&&((in.peek()==' ')||(in.peek()=='\t')||(in.peek()=='\n')||(in.peek()==13)||(in.peek()==10)||(in.peek()==0))) {
		in.get(gt);
	}
}

string read_next_json_token(istream &in) {
	string single_char_tokens("[]{}:,");
	trim_spaces(in);
	char cget;
	in.get(cget);
	//if this is a start of a quote, continue
	if(cget=='\"') {
		string total_string;
		total_string.push_back(cget);
		in.get(cget);
		total_string.push_back(cget);
		while(in.good()&&((cget!='\"'))) {
			if(cget=='\\') {in.get(cget); total_string.push_back(cget);}
			in.get(cget);
			total_string.push_back(cget); 
		}
//		cerr << "returning quoted token |" << total_string << "|" << endl;
		return total_string;
	}
	//single character tokens
	if(single_char_tokens.find(cget)!=string::npos) {
//		cerr << "returning single token |" << string(1,cget) << "|" << endl;
	   	return string(1,cget);
	}
	string total_string;
	total_string.push_back(cget);
	while(in.good()&&(single_char_tokens.find(in.peek())==string::npos)) {
		in.get(cget);
		if((cget!=' ')&&(cget!='\n')&&(cget!=13)&&(cget!=10)&&(cget!=0)) //ignore whitespace
			total_string.push_back(cget);
	}
//	cerr << "returning token string|" << total_string << "|" << endl;
	return total_string;
}


//--------------JSONObject---------

ostream &operator<<(ostream &out,const JSONObject &obj) {
	string ostr=obj.toString(0);
	return out << ostr;
}

istream &operator>>(istream &in,JSONObject &obj) {
	trim_spaces(in);
	if(in.peek()=='{') {char c; in.get(c);} //remove leading {
	if(in.peek()=='}') return in; //empty object
	string pairsep=",";
	while((pairsep.size()!=0)&&(pairsep==",")) {
		string key=read_next_json_token(in);
//		cout << "key was " << key << endl;
		if(key.size()==0) continue; //this is bad
		//trime quotes off key
		if(key[0]=='\"') key=key.substr(1,key.size()-1);
		if(key[key.size()-1]=='\"') key=key.substr(0,key.size()-1);
		string sep=read_next_json_token(in);
		if(sep[0]!=':') cerr << "JSON warning, separator wasn't a colon, it was " << sep << " (character " << (int)(sep[0]) << ")" << endl;
		JSONValue val;
		in >> val;
		obj[key]=val;
//		cerr << "read key value pair |" << key << " - " << val << endl;
		//cerr << "read key value pair |" << key << " - " << obj[key] << endl;
		pairsep=read_next_json_token(in);
	}
	if(pairsep[0]!='}') cerr << "JSON warning, end of object not a }, but instead " << pairsep << endl;
	return in;
}

string JSONObject::toString(int indent) const {
	stringstream out;
	out << endl;
	out << string(indent,' ') << "{ " << endl;
	for(JSONObject::const_iterator it=begin();it!=end();it++) {
		if(it!=begin()) out << " ," << endl;
		out << string(indent,' ') << "\"" << (*it).first << "\" : " << (*it).second.toString(indent);
	}
	out << string(indent,' ') << endl << string(indent,' ') << " }";
	return out.str();
}

void JSONObject::fromString(const string &s) {
	stringstream ss(s);
	ss >> (*this);
}

//--------------JSONArray-----------
JSONArray::JSONArray() {
	elements=NULL;
	n_elements=0;
}
	 
JSONArray::JSONArray(const JSONArray &a) {
	elements=NULL;
	n_elements=0;
	(*this)=a;
}

JSONArray::~JSONArray() {
	delete [] elements;
}

JSONValue &JSONArray::operator[](const int &i) 
{
	if(i>=n_elements) resize(i+1);
	return elements[i];
}

const JSONValue &JSONArray::operator[](const int &i) const 
{
	#ifdef JSON_TYPE_ERRORS_ON
		if(i>=length())
			cerr << "Error, index " << i << " out of bounds in JSONArray of length " << length() << endl;
	#endif
	return elements[i];
}
	
void JSONArray::resize(int maxelems) {
	if(maxelems==0) {
		delete [] elements;
		n_elements=0;
		return;
	}
	JSONValue *old_elems=elements;
	elements=new JSONValue[maxelems];
	int min=maxelems;
	if(n_elements<min) min=n_elements;
	for(int i=0;i<min;i++)
		elements[i]=old_elems[i];
	n_elements=maxelems;
	delete [] old_elems;
}
	
JSONArray &JSONArray::operator=(const JSONArray &a) {
	resize(a.length());
	for(int i=0;i<length();i++)
		elements[i]=a[i];
	return *this;
}

	
bool JSONArray::operator==(const JSONArray &a) const {
	if((*this)<a) return false;
	if(a<(*this)) return false;
	return true;
}
	
bool JSONArray::operator<(const JSONArray &a) const {
	if(length()<a.length()) return true;
	if(a.length()<length()) return false;
	for(int i=0;i<length();i++) {
		if(elements[i]<a[i]) return true;
		if(a[i]<elements[i]) return false;
	}
	return false;
}

ostream &operator<<(ostream &out,const JSONArray &arr) {
	out << "[ ";
	for(int i=0;i<arr.length();i++) {
		if(i!=0) out << " , ";
		out << arr[i];
	}
	out << " ] ";
	return out;
}

istream &operator>>(istream &in,JSONArray &val) {
	//cerr << "reading array" << endl;
	trim_spaces(in);
	//it may be the opening [ should already be taken
	if(in.peek()=='[') {char gt; in.get(gt);}
	trim_spaces(in);
	if(in.peek()==']') return in;
	string sep=",";
	while((in.good())&&(sep[0]==',')) {
		JSONValue nextval;
		in >> nextval;
		val.append(nextval);
		sep=read_next_json_token(in);
		//cerr << "sep was |" << sep << "|" << endl;
	}
	if(sep.size()!=0)
	if(sep[0]!=']') cerr << "JSON warning, array not closed with ], closed with " << sep << " instead" << endl;
	//cerr << "read array: " << val << endl;
	return in;
}
	
void JSONArray::append(const JSONValue &val) 
{(*this)[length()]=val;}

string JSONArray::toString(int indent) const {
	stringstream ss;
	for(int i=0;i<indent;i++)
		ss << " ";
	ss << (*this);
	return ss.str();
}

//--------------JSONValue-----------
JSONValue::JSONValue() {
	data=NULL;
	setNull();
}

JSONValue::JSONValue(const string &s) {
	data=NULL;
	setStringValue(s);
}

JSONValue::JSONValue(const long long int &i) {
	data=NULL;
	setIntValue(i);
}

JSONValue::JSONValue(const int &i) {
	data=NULL;
	setIntValue(i);
}


JSONValue::JSONValue(const double &d) {
	data=NULL;
	setDoubleValue(d);
}

JSONValue::JSONValue(const JSONValue &v) {
	data=NULL;
	(*this)=v;
}
	
JSONValue::JSONValue(const JSONArray &arr) {
	data=NULL;
	setArrayValue(arr);
}

JSONValue &JSONValue::operator=(const JSONValue &v) {
	switch(v.getType()) {
		case STRING:
			setStringValue(v.getStringValue());
			break;
		case INTEGER:
			setIntValue(v.getIntValue());
			break;
		case DOUBLE:
			setDoubleValue(v.getDoubleValue());
			break;
		case ARRAY:
			setArrayValue(v.getArrayValue());
			break;
		case OBJECT:
			setObjectValue(v.getObjectValue());
			break;
		case BOOLEAN:
			setBoolValue(v.getBoolValue());
			break;
		case JSONNULL:
			setNull();
			break;
	}
	return *this;
}

JSONValue::~JSONValue() {
	delete_data();
	data=NULL;
}

bool JSONValue::operator==(const JSONValue &v) const
{
	if((!(*this<v))&&(!(v<*this))) return true;
	return false;
}

bool JSONValue::operator<(const JSONValue &v) const
{
	if(getType()<v.getType()) return true;
	if(v.getType()<getType()) return false;
	switch(getType()) {
		case STRING:
			return getStringValue()<v.getStringValue();
		case INTEGER:
			return getIntValue()<v.getIntValue();
		case DOUBLE:
			return getDoubleValue()<v.getDoubleValue();
		case ARRAY:
			return getArrayValue()<v.getArrayValue();
		case OBJECT:
			return getObjectValue()<v.getObjectValue();
		case BOOLEAN:
			return getBoolValue()<v.getBoolValue();
		case JSONNULL:
			return false;
	}
	return false;
}

void JSONValue::delete_data() {
	if(data==NULL) return;
	switch(getType()) {
		case STRING:
			delete ((string*)data);
			return;
		case INTEGER:
			delete ((long long int*)data);
			return;
		case DOUBLE:
			delete ((double*)data);
			return;
		case ARRAY:
			delete ((JSONArray*)data);
			return;
		case BOOLEAN:
			delete ((bool*)data);
			return;
		case OBJECT:
			delete ((JSONObject*)data);
		case JSONNULL:
			return;
	}
}

void JSONValue::setStringValue(const string &s) {
	delete_data();
	type=STRING;
	data=new string(s);
}

void JSONValue::setIntValue(long long int i) {
	delete_data();
	type=INTEGER;
	data=new long;
	*((long long int*)data)=i;
}

void JSONValue::setDoubleValue(double d) {
	delete_data();
	type=DOUBLE;
	data=new double;
	*((double*)data)=d;
}

void JSONValue::setArrayValue(const JSONArray &r) {
	delete_data();
	type=ARRAY;
	data=new JSONArray(r);
}

void JSONValue::setObjectValue(const JSONObject &o) {
	delete_data();
	type=OBJECT;
	data=new JSONObject();
	(*((JSONObject*)(data)))=o;
}
	
void JSONValue::setBoolValue(const bool &b) {
	delete_data();
	type=BOOLEAN;
	data=new bool;
	*((bool*)data)=b;
}

void JSONValue::setNull() {
	delete_data();
	data=NULL;
	type=JSONNULL;
}
	
bool JSONValue::isNull() const {
	return getType()==JSONNULL;
}
	
const JSONObject &JSONValue::getObjectValue() const {
	#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=OBJECT)
		cerr << "Error requesting JSON object value from " << getTypeAsString() << " (" << getType() << ")" << endl;
	#endif
	return *((JSONObject*)data);
}

JSONObject &JSONValue::getObjectValue() {
	if(getType()!=OBJECT)
	{
		#ifdef JSON_TYPE_ERRORS_ON
		cerr << "Error requesting JSON object value from " << getTypeAsString() << " (" << getType() << ")" << endl;
		#endif
		setObjectValue(JSONObject());
	}
	return *((JSONObject*)data);
}

const bool &JSONValue::getBoolValue() const {
	#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=BOOLEAN)
		cerr << "Error requesting JSON bool value from " << getTypeAsString() << " (" << getType() << ")" << endl;
	#endif
	return *((bool*)data);
}

bool &JSONValue::getBoolValue() {
	if(getType()!=BOOLEAN)
	{
		#ifdef JSON_TYPE_ERRORS_ON
		cerr << "Error requesting JSON bool value from " << getTypeAsString() << " (" << getType() << ")" << endl;
		#endif
		setBoolValue(false);
	}
	return *((bool*)data);
}

bool JSONValue::isBoolTrue() const {
	if(getType()==BOOLEAN)
		return getBoolValue();
	return false;
}

bool JSONValue::isBoolFalse() const {
	if(getType()==BOOLEAN)
		return !getBoolValue();
	return false;
}





const string &JSONValue::getStringValue() const {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=STRING)
	cerr << "Error requesting JSON string value from " << getTypeAsString() << " (" << getType() << ")" << endl;
#endif
	return *((string*)data);
}

string &JSONValue::getStringValue() {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=STRING)
	{
	cerr << "Error requesting JSON string value from " << getTypeAsString() << "in accessor" << endl;
	setStringValue("");
	}
#endif
	return *((string*)data);
}


const long long int &JSONValue::getIntValue() const {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=INTEGER)
	cerr << "Error requesting JSON int value from " << getTypeAsString() << endl;
#endif

	return *((long long int*)data);
}

long long int &JSONValue::getIntValue() {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=INTEGER)
	{
	cerr << "Error requesting JSON int value from " << getTypeAsString() << endl;
	setIntValue(0);
	}
#endif

	return *((long long int*)data);
}


//const double &JSONValue::getDoubleValue() const {
const double JSONValue::getDoubleValue() const {
#ifdef JSON_TYPE_ERRORS_ON
	if((getType()!=DOUBLE)&&(getType()!=INTEGER))
	cerr << "Error requesting JSON double value from " << getTypeAsString() << endl;
#endif
	if(getType()==INTEGER)
		return double(*((long long*)data));

	return *((double*)data);
}

double &JSONValue::getDoubleValue() {
	if(getType()==INTEGER) { //auto convert ints to doubles
		setDoubleValue(getIntValue());
	}

#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=DOUBLE)
	{
		cerr << "Error requesting JSON double value from " << getTypeAsString() << endl;
		setDoubleValue(0);
	}
#endif

	return *((double*)data);
}


const JSONArray &JSONValue::getArrayValue() const {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=ARRAY)
	cerr << "Error requesting JSON array value from " << getTypeAsString() << endl;
#endif

	return *((JSONArray*)data);
}

JSONArray &JSONValue::getArrayValue() {
#ifdef JSON_TYPE_ERRORS_ON
	if(getType()!=ARRAY) {
		cerr << "Error requesting JSON array value from " << getTypeAsString() << endl;
		setArrayValue(JSONArray());
	}
#endif

	return *((JSONArray*)data);
}



void JSONValue::fromToken(string tok) {
	if(tok.size()==0) return;
	if(tok[0]=='\"') { //string value, remove escapes
		string stringv;
		for(size_t i=1;i<(tok.size()-1);i++) {
			if((tok[i]=='\\')&&(i+1<tok.size())) {
				i++;
			}
			stringv.push_back(tok[i]);
			setStringValue(stringv);
		}
	} else if(isdigit(tok[0])) { //int or double
		if((tok.find('.')!=string::npos)||(tok.find('e')!=string::npos))
			setDoubleValue(atof(tok.c_str()));
		else
			setIntValue(atoll(tok.c_str()));
	} else if((tok=="true")||(tok=="false")) {
		if(tok=="true") setBoolValue(true);
		else setBoolValue(false);
	} else if(tok=="null"||tok=="NULL"||tok=="Null") {
		setNull();
	} else {
		cerr << "JSON error.  " << tok << " is not a single token value" << endl;
	}
}

string JSONValue::toString(int indent) const {
	stringstream out;
	switch(getType()) {
		case STRING:
			{
				string toprint=getStringValue();
				out << "\"";
				for(size_t i=0;i<toprint.size();i++)
					if(toprint[i]=='"')
						out << "\\\"";
					else
						out << toprint[i];
				out << "\"";
				break;
			}
		case INTEGER:
			out << getIntValue();
			break;
		case DOUBLE:
			out << getDoubleValue();
			break;
		case ARRAY:
			out << getArrayValue();
			break;
		case OBJECT:
			out << getObjectValue().toString(indent+2);
			break;
		case BOOLEAN:
			if(getBoolValue()==true)
				out << "true";
			else
				out << "false";
			break;
		case JSONNULL:
			out << "null";
			break;
	}
	return out.str();
}

string JSONValue::getTypeAsString() const {
	switch(getType()) {
		case STRING:
			return "string";
		case INTEGER:
			return "integer";
		case DOUBLE:
			return "double";
		case ARRAY:
			return "array";
		case OBJECT:
			return "object";
		case BOOLEAN:
			return "boolean";
		case JSONNULL:
			return "null";
	}
	return "UNKNOWN!";
}


istream &operator>>(istream &in,JSONValue &val) {
	string token=read_next_json_token(in);
	if(token.size()==0) return in;
	if(token[0]=='[') { //array
		//cerr << "value is array " << endl;
		JSONArray myarray;
		in >> myarray;
		val.setArrayValue(myarray);
		return in;
	} if(token[0]=='{') { //object
		//cerr << "value is object" << endl;
		JSONObject myobject;
		in >> myobject;
		//cerr << "read object: " << myobject << endl;
		val.setObjectValue(myobject);
		//cerr << "should match: " << val.getObjectValue() << endl;
		return in;
	} else { //must be a single token value
		//cerr << "value is string or number: " << token << endl;
		val.fromToken(token);
	}
	return in;
}

ostream &operator<<(ostream &out,const JSONValue &val) {
	return out << val.toString();
}


void json_self_test() {
	JSONValue astring("a \"string\"");
	cout << "string : " << astring.getTypeAsString() << " : " << astring << endl;
	JSONValue anint(3);
	cout << "int : " << anint.getTypeAsString() << " : " << anint << endl;
	JSONValue adouble(4.3);
	cout <<"double : " << adouble.getTypeAsString() << " : " <<  adouble << endl;
	JSONArray myarray;
	myarray[0]=JSONValue(1);
	myarray[1]=JSONValue(2);
	myarray[2]=JSONValue(3);
	myarray[3]=JSONValue("cheese");
//	cout << myarray << endl;
	JSONValue anarray(myarray);
	cout << "array: " << anarray.getTypeAsString() << " : "  <<anarray << endl;
	JSONValue abool;
	abool.setBoolValue(false);
	cout << "bool : " << abool.getTypeAsString() << " : " << abool << endl;
	JSONValue anull;
	cout << "null : " << anull.getTypeAsString() << " : " << anull << endl;
	JSONObject mydocument;
	cout << "empty document: " << endl;
	cout << mydocument << endl;
	cout << "full document" << endl;
	mydocument["stringtype"]=astring;
	mydocument["inttype"]=anint;
	mydocument["doubletype"]=adouble;
	mydocument["arraytype"]=myarray;
	mydocument["booltype"]=abool;
	mydocument["nulltype"]=anull;
	cout << mydocument << endl;

	ifstream fin("test_json.txt");
	JSONObject object;
	fin >> object;
	fin.close();
	cout<< "read: " << object << endl;

}

}

#include "CouchDBInterface.hh"
#include <vector>
#include <set>
#include <stdlib.h>

using namespace std;

class SensorReading
{
public:
	string sensor_name;
	double value;
	int precision;
	string units;
	long long timestamp_ms;
	time_t timestamp;
	bool has_error;
	string error_value;
	double uncalibrated_value;
	string uncalibrated_units;
};

class P8Lice {
public:
	P8Lice();
	//Get a list of sensors that have readings between time start and time stop
	set<string> getSensors(time_t start,time_t stop);
	//Get a list of sensor reads between time start and time stop
	vector<SensorReading> getReadings(time_t start, time_t stop);
	//Get a list of sensor readings between time start and time stop, filtered so only sensor sensor_name is shown
	vector<SensorReading> getReadings(string sensor_name,time_t start, time_t stop);
	//Gets the sensor reading with name sensor name that is as closest to timeat, looking out to rang seconds away
	SensorReading getReading(string sensor_name,time_t timeat, time_t range=600);
	
	CouchDBInterface interface;
};

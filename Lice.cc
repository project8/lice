#include "Lice.hh"
#include <sstream>

P8Lice::P8Lice() {
	interface.setServer("p8portal.phys.washington.edu");
	interface.setPort("5984");
	interface.setDBName("p8slowcontrollog");
}
	
set<string> P8Lice::getSensors(time_t start,time_t stop) {
	vector<SensorReading> allsens=getReadings(start,stop);
	set<string> ret;
	for(size_t i=0;i<allsens.size();i++)
		ret.insert(allsens[i].sensor_name);
	return ret;
}

vector<SensorReading> P8Lice::getReadings(time_t start_time, time_t stop_time) {
	vector<SensorReading> ret;
	stringstream keys;
	keys << "?startkey=" << start_time << "000&endkey=" << stop_time << "000";
	JSONObject reply=interface.getView("_design/plotdata/_view/all",keys.str());
	JSONArray rows=reply["rows"].getArrayValue();
	if(rows.length()==0)
		return ret;
	for(int i=0;i<rows.length();i++) {
		SensorReading toadd;
		toadd.sensor_name=rows[i]["value"]["sensor_name"].getStringValue();
		toadd.timestamp_ms=rows[i]["value"]["timestamp_mseconds"].getIntValue();
		toadd.timestamp=(toadd.timestamp_ms)/1000;
		toadd.units=rows[i]["value"]["units"].getStringValue();
		toadd.value=rows[i]["value"]["value"].getDoubleValue();
		if(!rows[i]["value"]["uncalibrated_value"].isNull())
			toadd.uncalibrated_value=rows[i]["value"]["uncalibrated_value"].getDoubleValue();
		if(!rows[i]["value"]["uncalibrated_units"].isNull())
			toadd.uncalibrated_units=rows[i]["value"]["uncalibrated_units"].getStringValue();
		if(!rows[i]["value"]["precision"].isNull())
			toadd.precision=rows[i]["value"]["precision"].getIntValue();
		if(!rows[i]["value"]["has_error"].isNull())
			toadd.has_error=rows[i]["value"]["has_error"].getBoolValue();
		else
			toadd.has_error=false;
		if(toadd.has_error)
			toadd.error_value=rows[i]["value"]["error_value"].getStringValue();
		ret.push_back(toadd);
	}
	return ret;
}
	
vector<SensorReading> P8Lice::getReadings(string sensor_name,time_t start, time_t stop) {
	vector<SensorReading> ret;
	vector<SensorReading> allsens=getReadings(start,stop);
	for(size_t i=0;i<allsens.size();i++) {
		if(allsens[i].sensor_name==sensor_name)
			ret.push_back(allsens[i]);
	}
	return ret;
}

SensorReading P8Lice::getReading(string sensor_name,time_t timeat, time_t range) {
	vector<SensorReading> sens=getReadings(sensor_name,timeat-range,timeat+range);
	if(sens.size()==0) {
		SensorReading ret;
		ret.has_error=true;
		ret.error_value="No reading found";
		return ret;
	}
	SensorReading closest=sens[0];
	for(size_t i=0;i<sens.size();i++) {
		if(abs(closest.timestamp-timeat)>abs(sens[i].timestamp-timeat))
			closest=sens[i];
	}
	return closest;
}

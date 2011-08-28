#include "Lice.hh"
#include <string.h>
#include <time.h>

const char *timefmt="%Y-%m-%d %H:%M:%S";

int main(int argc,char *argv[]) {
	if(argc<2) {
	cerr << "Usage: getP8Sensor (range|single|sensors) [options]" << endl;
	cerr << "Options: " << endl;
	cerr << " -n (sensor name)" << endl;
	cerr << " -s (start time)" << endl;
	cerr << " -e (end time)" << endl;
	cerr << " -a (at time)" << endl;
	cerr << "Time Format:" << endl;
	cerr << " Year-Month-Day Hour:Minute:Second" << endl;
	cerr << "for example:" << endl;
	cerr << "./getP8Sensor single -n coil_temp -a \"2011-8-27 13:00:00\"" << endl;
	cerr << "returns the value of the coil_temp closest to 1pm on August 27" << endl;

	return -1;
	}
	char *sensname=NULL;
	struct tm conv;
	time_t starttime=0;
	time_t endtime=0;
	time_t attime=0;
	for(int i=2;i<argc;i++) {
		if(strcmp(argv[i],"-n")==0) {
			sensname=argv[i+1];
			i++;
		} else if(strcmp(argv[i],"-s")==0) {
			strptime(argv[i+1],timefmt,&conv);
			starttime=mktime(&conv);
			i++;
		} else if(strcmp(argv[i],"-e")==0) {
			strptime(argv[i+1],timefmt,&conv);
			endtime=mktime(&conv);
			i++;
		} else if(strcmp(argv[i],"-a")==0) {
			strptime(argv[i+1],timefmt,&conv);
			attime=mktime(&conv);
			i++;
		}
	}

	char timestr[256];
	P8Lice lice;
	if(strcmp(argv[1],"range")==0) {
		vector<SensorReading> readings;
		if(sensname==NULL) {
			readings=lice.getReadings(starttime,endtime);
			for(size_t i=0;i<readings.size();i++) {
				strftime(timestr,256,timefmt,localtime(&readings[i].timestamp));
				cout << timestr << " " << readings[i].sensor_name << " " << readings[i].value << " " << readings[i].units << endl;
			}
		} else {
			readings=lice.getReadings(string(sensname),starttime,endtime);
			for(size_t i=0;i<readings.size();i++) {
				strftime(timestr,256,timefmt,localtime(&readings[i].timestamp));
				cout << timestr << " " << readings[i].value << " " << readings[i].units << endl;
			}
		}
	} else if(strcmp(argv[1],"single")==0) {
		if(attime==0) {cerr << "needed -a option" << endl; return -1;}
		SensorReading reading=lice.getReading(string(sensname),attime);
		strftime(timestr,256,timefmt,localtime(&reading.timestamp));
		cout << timestr << " " << reading.value << " " << reading.units << endl;
	} else if(strcmp(argv[1],"sensors")==0) {
		set<string> sens=lice.getSensors(starttime,endtime);
		for(set<string>::iterator it=sens.begin();it!=sens.end();it++)
			cout << (*it) << endl;
	} else {
		cerr << "unrecognized command: " << argv[1] << endl;
	}
}

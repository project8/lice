CXXFLAGS := -Wall -g
TARGETS := getP8Sensor
LINKEDLIBS := -lcurl

$(TARGETS) : % : %.cc lice.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKEDLIBS)

lice.a : Lice.o JSON.o CouchDBInterface.o
	ar -rcs lice.a $?

clean:
	rm -f lice.a
	rm -f $(TARGETS)
	rm -f *.o

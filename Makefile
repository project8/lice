CXXFLAGS := -Wall -g
TARGETS := getP8Sensor
LINKEDLIBS := -lcurl

# Check OS type for determining the existence of the function open_memstream in stdio.h
OS := $(shell uname -s)
ifeq ($(OS),Darwin)
	CXXFLAGS := $(CXXFLAGS) -DNO_MEMSTREAM
endif
ifeq ($(OS),FreeBSD)
	CXXFLAGS := $(CXXFLAGS) -DNO_MEMSTREAM
endif

.PHONY: all
all: liblice.a $(TARGETS)

$(TARGETS) : % : %.cc liblice.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKEDLIBS)

liblice.a : Lice.o JSON.o CouchDBInterface.o MemstreamInterface.o
	ar -rcs liblice.a $?

clean:
	rm -f liblice.a
	rm -f $(TARGETS)
	rm -f *.o

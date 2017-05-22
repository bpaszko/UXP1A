CXX = g++
CXXFLAGS = -Wall -pthread -lrt
EXEC = uxp1a.out
BUILD_PATH = build/
OBJECTS_O = Creator.o Worker.o
SOURCES = Creator.cpp Worker.cpp

objects: 
	${CXX} -c ${CXXFLAGS} Creator.cpp
	${CXX} -c ${CXXFLAGS} Worker.cpp
	

all: objects
	${CXX} ${CXXFLAGS} -o ${BUILD_PATH}${EXEC} ${OBJECTS_O}

clean:
	rm ${BUILD_PATH}*


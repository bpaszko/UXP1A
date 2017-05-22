CXX = g++
CXXFLAGS = -std=c++11 -Wall
LIBRARIES = -pthread -lrt
DEBUG = -DDEBUG=1 -g
EXEC = uxp1a.out
BUILD_PATH = build/
OBJECTS_O = ${SOURCES:.cpp=.o}
SOURCES = Creator.cpp Worker.cpp

objects: 
	${CXX} -c ${CXXFLAGS} Creator.cpp
	${CXX} -c ${CXXFLAGS} Worker.cpp
	
objects_debug:
	${CXX} -c ${CXXFLAGS} ${DEBUG} Creator.cpp
	${CXX} -c ${CXXFLAGS} ${DEBUG} Worker.cpp

debug: 	objects_debug
	${CXX} ${CXXFLAGS} ${DEBUG} -o ${BUILD_PATH}${EXEC} ${OBJECTS_O} ${LIBRARIES}	

all: objects
	${CXX} ${CXXFLAGS} -o ${BUILD_PATH}${EXEC} ${OBJECTS_O} ${LIBRARIES}

clean:
	rm *.o
	rm ${BUILD_PATH}*

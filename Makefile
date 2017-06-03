CXX = g++
CXXFLAGS = -std=c++11 -Wall
LIBRARIES = -pthread -lrt
DEBUG = -DDEBUG=1 -g
EXEC = uxp1a.out
EXEC_INPUT = input.out
EXEC_OUTPUT = output.out
EXEC_READ = read.out
OBJECTS_O = ${SOURCES:.cpp=.o}
SOURCES = Creator.cpp Worker.cpp
INPUT_MAIN = input_main.cpp
OUTPUT_MAIN = output_main.cpp
READ_MAIN = read_main.cpp

objects: 
	${CXX} -c ${CXXFLAGS} Creator.cpp
	${CXX} -c ${CXXFLAGS} Worker.cpp
	
objects_debug:
	${CXX} -c ${CXXFLAGS} ${DEBUG} Creator.cpp
	${CXX} -c ${CXXFLAGS} ${DEBUG} Worker.cpp

input:  objects
	${CXX} -c ${CXXFLAGS} ${INPUT_MAIN}
	${CXX} ${CXXFLAGS} -o ${EXEC_INPUT} ${OBJECTS_O} input_main.o ${LIBRARIES}

output:  objects
	${CXX} -c ${CXXFLAGS} ${OUTPUT_MAIN}
	${CXX} ${CXXFLAGS} -o ${EXEC_OUTPUT} ${OBJECTS_O} output_main.o ${LIBRARIES}

read:  objects
	${CXX} -c ${CXXFLAGS} ${READ_MAIN}
	${CXX} ${CXXFLAGS} -o ${EXEC_READ} ${OBJECTS_O} read_main.o ${LIBRARIES}

input_debug:  objects_debug
	${CXX} -c ${CXXFLAGS} ${DEBUG} ${INPUT_MAIN}
	${CXX} ${CXXFLAGS} ${DEBUG} -o ${EXEC_INPUT} ${OBJECTS_O} input_main.o ${LIBRARIES}

output_debug:  objects_debug
	${CXX} -c ${CXXFLAGS} ${DEBUG} ${OUTPUT_MAIN}
	${CXX} ${CXXFLAGS} ${DEBUG} -o ${EXEC_OUTPUT} ${OBJECTS_O} output_main.o ${LIBRARIES}

read_debug:  objects_debug
	${CXX} -c ${CXXFLAGS} ${DEBUG} ${READ_MAIN}
	${CXX} ${CXXFLAGS} ${DEBUG} -o ${EXEC_READ} ${OBJECTS_O} read_main.o ${LIBRARIES}

debug: 	input_debug
		read_debug
		output_debug	

all: input
	 read
	 output

clean:
	rm *.o
	rm ${BUILD_PATH}*

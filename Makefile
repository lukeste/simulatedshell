GPPWARN     = -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast
COMPILECPP  = g++ -std=gnu++2a -g -O0 ${GPPWARN}

MODULES     = commands file_sys util
CPPHEADER   = ${MODULES:=.h}
CPPSOURCE   = ${MODULES:=.cpp} main.cpp
EXECBIN     = myshell
OBJECTS     = ${CPPSOURCE:.cpp=.o}

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${COMPILECPP} -o $@ ${OBJECTS}

%.o : %.cpp
	${COMPILECPP} -c $<

clean :
	- rm ${OBJECTS}

spotless : clean
	- rm ${EXECBIN}

#set the final compile target
TARGET = test

PREFIX_BIN =

#set the cpp compile
CXX=g++  

ifeq (0,${debug}) 
	CPPFLAGS = -DNDEBUG -g2 -O2 
	#TARGET = RelFile/test
else
	CPPFLAGS = -ggdb -O2 
endif 

CPPFLAGS += -Wall
CPPFLAGS += -I/data/antwars/lib/boost_1_51_0/ 


LIBS = -lcurl -lboost_thread -lboost_date_time -lboost_system
LINKFLAGS = -L/usr/local/lib 
LINKFLAGS += -L/data/antwars/lib/boost_1_51_0/stage/lib 

#set the include dirs
INCLUDEDIRS = ./ ./src/
INCLUDES = $(foreach tmp, $(INCLUDEDIRS), -I $(tmp))

#set my source dirs
MYSOURCEDIRS = ./ ./src/

SOURCEDIRS = $(MYSOURCEDIRS)

C_SOURCES = $(foreach tmp, $(SOURCEDIRS), $(wildcard $(tmp)*.c))
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

CPP_SOURCES = $(foreach tmp, $(SOURCEDIRS), $(wildcard $(tmp)*.cpp))
CPP_OBJS = $(patsubst %.cpp, %.o, $(CPP_SOURCES))

all:compile
.PHONY :all

.c.o:
	@$(CC) -c -o $*.o $(CFLAGS) $(INCLUDES) $*.c
.cpp.o:
	@echo -n $*.cpp " "
	@$(CXX) -c -o $*.o $(CPPFLAGS) $(INCLUDES) $*.cpp

compile: $(CPP_OBJS) $(C_OBJS) $(OTHERS_C_OBJS) $(OTHERS_CPP_OBJS)
	@$(CXX) $(LINKFLAGS) -o $(TARGET) $^ $(LIBS)
	@echo ""

.PHONY : clean
clean:
	@rm -f $(CPP_OBJS) $(C_OBJS)
	@rm -f $(TARGET)

install: $(TARGET)
	@cp $(TARGET) $(PREFIX_BIN)

uninstall:
	@rm -f $(PREFIX)/$(PREFIX_BIN)

rebuild: clean





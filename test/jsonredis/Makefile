#set the final compile target
TARGET = jsonredis

PREFIX_BIN =

#set the cpp compile
CXX=g++  

ifeq (0,${debug}) 
	CPPFLAGS = -DNDEBUG -g2 -O2 
else
	CPPFLAGS = -ggdb -O2 
endif 

CPPFLAGS += -Wall
CPPFLAGS += -std=c++11
CPPFLAGS += -I/usr/local/include/
CPPFLAGS += -I/usr/local/Cellar/thrift/0.9.2/include/

#LIBS = -lcurl -lboost_thread -lboost_date_time -lboost_system -lthrift
#LIBS = -lcurl -lboost_thread-mt -lboost_date_time -lboost_system -lthrift
LIBS = -lhiredis -lcurl -lboost_date_time -lboost_system -lthrift
LIBS += /usr/local/lib/libjsoncpp.a 
LINKFLAGS = -L/usr/local/lib 
LINKFLAGS += -L/usr/local/Cellar/thrift/0.9.2/lib/

#set the include dirs
INCLUDEDIRS = ./ ../gen-cpp/
INCLUDES = $(foreach tmp, $(INCLUDEDIRS), -I $(tmp))

#set my source dirs
MYSOURCEDIRS = ./ ../gen-cpp/

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





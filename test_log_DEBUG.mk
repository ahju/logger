################OPTION###################
CCOMPILE = gcc
CPPCOMPILE = g++
COMPILEOPTION = -c -g
INCLUDEDIR = 
LINK = g++
LINKOPTION = -g -o test_log
LIBDIRS = 
OBJS = test_log.o logger.o thread1.o thread2.o 
OUTPUT = test_log
SHAREDLIB = -lpthread
APPENDLIB = 
$(OUTPUT): $(OBJS) $(APPENDLIB)
	$(LINK) $(LINKOPTION) $(LIBDIRS) $(OBJS) $(SHAREDLIB) $(APPENDLIB)

clean: 
	rm -f $(OBJS)
	rm -f $(OUTPUT)
all: clean $(OUTPUT)
.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cx

.cxx.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cxx

.c.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc

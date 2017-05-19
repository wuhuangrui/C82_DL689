BIN = clmain

ARCHINC = ../../../ClouSoft/arch/linux
DRIVERSINC = ../../../ClouSoft/drivers/linux
DRVCOMM = ../../../ClouSoft/drivers/common
COMINC = ../../../ClouSoft/common
MODEMINC = ../../../ClouSoft/common/Modem
PROTOIFINC = ../../../ClouSoft/common/ProtoIf
DBINC = ../../../ClouSoft/app/DataBase
DPINC = ../../../ClouSoft/app/DataProc
CCTINC = ../../../ClouSoft/app/Cct
CCTOOBINC = ../../../ClouSoft/app/CctOob
CTRL = ../../../ClouSoft/app/Ctrl
FAINC = ../../../ClouSoft/app/fa
PROTOINC = ../../../ClouSoft/app/Proto
METER = ../../../ClouSoft/app/Meter
TASKINC = ../../../ClouSoft/app/Task
METERPRO = ../../../ClouSoft/MeterPro
ACINC = ../../../ClouSoft/AcSample
DBLIB = ../../../ClouSoft/common/LibDb
ACLIB = ../../../ClouSoft/common/LibAc
CCTLIB = ../../../ClouSoft/common/LibCct
TASKDB = ../../../ClouSoft/common/TaskDB
DISP = ../../../ClouSoft/app/Display

INCS = -I$(ARCHINC) -I$(DRIVERSINC) -I$(COMINC) -I$(MODEMINC) -I$(PROTOIFINC) -I$(ACLIB) \
		-I$(DBINC) -I$(METER) -I$(DPINC) -I$(FAINC) -I$(DBLIB) -I$(CCTLIB) -I$(MTRLIB) -I$(MTRLIB) -I$(TASKDB)\
		-I$(PROTOINC) -I$(TASKINC) -I$(METERPRO) -I$(DRVCOMM)\
	  	-I$(ACINC) -I$(CCTINC) -I$(CCTOOBINC) -I$(CTRL) -I$(DISP)

LIBS = -larch -ldrivers -lapp -lLibDb -lTaskDB -lLibAc -lProtoIf -lModem -lapp -lgcc  -lpthread
#LIBS = -larch -ldrivers -lapp -lLibDb -lTaskDB -lProtoIf -lModem -lapp -lgcc  -lpthread

##################################################
ROOTDIR = ..
OBJDIR = $(ROOTDIR)/obj
BINDIR = $(ROOTDIR)/bin
LIBDIR = $(ROOTDIR)/lib

##################################################
SRCDIR = .
SRCS = $(notdir $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*.cpp))
	
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS)))

##################################################

#old: gcc
CC       = arm-none-linux-gnueabi-g++
#CPPFLAGS = -g -c -Os $(INCS)
#CPPFLAGS += -DPORTSTL -DLINUX
#LDFLAGS  = -L$(LIBDIR) $(LIBS)

CPPFLAGS = -g -c -Os $(INCS)  -DARM9
CPPFLAGS += -DLINUX -fno-builtin-log
LDFLAGS  = -L$(LIBDIR) $(LIBS)

#########################################################
$(BINDIR)/$(BIN): $(BINDIR) $(OBJDIR) $(OBJS) 
	$(CC) -g -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR):
	@test -d $@ || mkdir $@
	
$(BINDIR):
	@test -d $@ || mkdir $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CPPFLAGS) -o $@ $<

clean:
	@rm -r -f $(OBJDIR) $(BINDIR)/$(BIN) $(SRCDIR)/*~
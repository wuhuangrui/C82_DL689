LIB = app

#source dir

ARCHINC = ../../../../arch/linux
DRIVERSINC = ../../../../drivers/linux
DRVCOMM = ../../../../drivers/common
COMINC = ../../../../common
MODEMINC = ../../../../common/Modem
PROTOIFINC = ../../../../common/ProtoIf
CCTINC = ../../../Cct
DBINC = ../../../DataBase
DPINC = ../../../DataProc
DRVAPI = ../../../Driver
FAINC = ../../../fa
CTRL = ../../../Ctrl
PROTOINC = ../../../Proto
TASKINC = ../../../Task

DISPLAY = ../../../../app/Display

GUICFG = ../../../../app/Display/gui/Config
GUIINC = ../../../../app/Display/gui/include
GUICORE = ../../../../app/Display/gui/Core
GUIFONT = ../../../../app/Display/gui/Font
GUIGUI_X = ../../../../app/Display/gui/GUI_X
GUILCDDriver = ../../../../app/Display/gui/LCDDriver

METER = ../../../Meter
METERPRO = ../../../../MeterPro
ACINC = ../../../../AcSample
TASKDB = ../../../../common/TaskDB
MTRLIB = ../../../../common/LibMtr
DBLIB = ../../../../common/LibDb
ACLIB = ../../../../common/LibAc
CCTLIB = ../../../../common/LibCct
PLCIFINC = ../../../../common/LibCct/PlcIf
CCTSTD=../../../../common/LibCct/CctStd


INCS = -I$(ARCHINC) -I$(DRIVERSINC) -I$(COMINC) -I$(MODEMINC) -I$(PROTOIFINC) \
		-I$(DBINC) -I$(DBLIB) -I$(FAINC) -I$(TASKDB) -I$(MTRLIB) -I$(METER)\
		-I$(PROTOINC) -I$(TASKINC) -I$(METERPRO) -I$(DPINC) -I$(DISPLAY)\
	  	-I$(ACINC) -I$(CTRL) -I$(ACLIB) -I$(CCTINC) -I$(DRVCOMM)\
	  	-I$(GUICFG) -I$(GUIINC) -I$(GUICORE) -I$(GUIFONT) -I$(GUILCDDriver) -I$(GUIGUI_X) \
	  	-I$(CCTLIB) -I$(PLCIFINC) -I$(CCTSTD) -I$(DRVAPI)

LIBS = -lnsl -lpthread -ldl

##################################################
ROOTDIR = ../../../../../Prj/Linux
OBJDIR = $(ROOTDIR)/obj
BINDIR = $(ROOTDIR)/bin
LIBDIR = $(ROOTDIR)/lib

##################################################
SRCDIR = .
SRCS = $(notdir $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*.cpp))
	
#OBJS = $(SRCS:%.=$(OBJDIR)/%.o)
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS)))

##################################################

CC       =  arm-none-linux-gnueabi-g++
AR       =  arm-none-linux-gnueabi-ar
CPPFLAGS = -g -c -Os $(INCS)  -DARM9
CPPFLAGS += -DLINUX -fno-builtin-log
LDFLAGS  = -L$(LIBDIR) $(LIBS)

#########################################################
$(LIBDIR)/lib$(LIB).a: $(LIBDIR) $(OBJDIR) $(OBJS) 
	$(AR) rv $@ $(OBJS)
	ranlib $@

$(OBJDIR):
	@test -d $@ || mkdir $@
	
$(LIBDIR):
	@test -d $@ || mkdir $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CPPFLAGS) -o $@ $<

clean:
	@rm -r -f $(OBJDIR) $(LIBDIR)/lib$(LIB).a $(SRCDIR)/*~
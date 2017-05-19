MPDIR = ../../ClouSoft
SUBDIRS	= $(MPDIR)/arch/linux $(MPDIR)/drivers/linux $(MPDIR)/AcSample\
		$(MPDIR)/app/DataBase $(MPDIR)/app/DataProc $(MPDIR)/app/Meter $(MPDIR)/app/fa $(MPDIR)/app/Proto $(MPDIR)/app/Esam \
		$(MPDIR)/app/Cct $(MPDIR)/app/CctOob $(MPDIR)/app/Ctrl $(MPDIR)/app/Task $(MPDIR)/app/Display $(MPDIR)/app/Driver $(MPDIR)/app/DLMS\
		$(MPDIR)/common $(MPDIR)/common/LibDb $(MPDIR)/common/Modem $(MPDIR)/common/ProtoIf $(MPDIR)/common/LibAc \
		$(MPDIR)/app/Display/gui/Core $(MPDIR)/app/Display/gui/Font $(MPDIR)/app/Display/gui/GUI_X $(MPDIR)/app/Display/gui/LCDDriver \
		$(MPDIR)/common/TaskDB $(MPDIR)/common/LibCct\
		$(MPDIR)/MeterPro $(MPDIR)/app/Update main 
		
		
#$(MPDIR)/TCPIP/src/arch/linux
#SUBDIRS	=  $(MPDIR)/arch/linux $(MPDIR)/main 

all:
#	cp -f $(MPDIR)/common/LibDb/libLibDb.a lib/libLibDb.a
#	cp -f $(MPDIR)/common/LibMtr/libLibMtr.a lib/libLibMtr.a
#	cp -f $(MPDIR)/common/TaskDB/libTaskDB.a lib/libTaskDB.a
#	cp -f $(MPDIR)/MeterPro/libMeterPro.a lib/libMeterPro.a
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) ;)

clean:
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) clean ;)
	rm -f lib/*.a

cleanup:
	@(rm -r -f bin lib obj)
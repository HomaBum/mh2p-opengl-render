LIST=CPU
#LDFLAGS+=-Wl,--allow-shlib-undefined
#LIBS+=egl
#LIBS+=glesv2
#LIBS+=gf
LIBS+=m
LIBS+=socket
ifndef QRECURSE
QRECURSE=recurse.mk
ifdef QCONFIG
QRDIR=$(dir $(QCONFIG))
endif
endif
include $(QRDIR)$(QRECURSE)

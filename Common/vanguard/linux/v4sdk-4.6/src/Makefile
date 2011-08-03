

#-------------------------------------------------------------------------------
#
ifndef V4LIB
# if V4LIB is not specified
# then suppose that ../lib folder contains only one library!!!
V4LIB=$(wildcard ../lib/*)
EXEID=


else
# if V4LIB is specified
# then extract library identifier
LIBID=$(notdir $(V4LIB))
LIBID:=$(LIBID:libv4%=%)
LIBID:=$(LIBID:%.a=%)
LIBID:=$(LIBID:%.lib=%)
LIBID:=$(LIBID:codecsdk%=%)
LIBID:=$(LIBID:decsdk%=%)
EXEID=$(LIBID)

endif

SDKLIB=$(notdir $(V4LIB))
SAMPLES=$(wildcard *.c)
TARGETS=$(SAMPLES:%.c=../bin/%$(EXEID))






#-------------------------------------------------------------------------------
# linux
ifneq (,$(findstring sdk-l,$(V4LIB)))


ifndef V4CC
V4CC=gcc
endif



CMD=$(V4CC) -I../inc -static -O3 -L../lib $^ -o $@ -l$(SDKLIB:lib%.a=%) $(V4OPTS)


ifneq (,$(findstring sdk-l32i,$(V4LIB)))
CMD+=-DLIN32 -m32 -L../../lib-icc32 -lirc -limf -lsvml
endif


ifneq (,$(findstring sdk-l32g,$(V4LIB)))
CMD+=-DLIN32 -m32
endif


ifneq (,$(findstring sdk-l64i,$(V4LIB)))
CMD+=-DLIN64 -m64 -L../../lib-icc64 -lirc -limf -lsvml
endif


ifneq (,$(findstring sdk-l64g,$(V4LIB)))
CMD+=-DLIN64 -m64
endif


CMD+=-lm -lpthread
endif







#-------------------------------------------------------------------------------
# windows
ifneq (,$(findstring sdk-w,$(V4LIB)))

EXEID:=$(EXEID).exe

ifndef V4CC
V4CC=cl.exe
endif

ifndef V4LINK
V4LINK=link.exe
endif


ifneq (,$(findstring gcc,$(V4CC)))
# some gcc compiler
CMD=$(V4CC) -I../inc -O3 -L../lib $^ -o $@ -l$(SDKLIB:%.lib=%) $(V4OPTS)


ifneq (,$(findstring sdk-w32i,$(V4LIB)))
CMD+=-DWIN32 -m32 -L../../lib-icc32 -lirc -lcmt
endif


ifneq (,$(findstring sdk-w32g,$(V4LIB)))
CMD+=-DWIN32 -m32
endif


ifneq (,$(findstring sdk-w64i,$(V4LIB)))
CMD+=-DWIN64 -m64 -L../../lib-icc64 -lirc -lcmt
endif


ifneq (,$(findstring sdk-w64g,$(V4LIB)))
CMD+=-DWIN64 -m64
endif




else
# other compiler
COMPILE=$(V4CC) /I ../inc  /Ox $^ /c /Fo$@.obj $(V4OPTS)
LINK_SAMPLE=$(V4LINK) /libpath:../lib /out:$@ $@.obj $(SDKLIB)

ifneq (,$(findstring sdk-w32i,$(V4LIB)))
COMPILE+=/D WIN32
LINK_SAMPLE+=/machine:I386 /libpath:../../lib-icl32 libirc.lib libcmt.lib
endif

ifneq (,$(findstring sdk-w32g,$(V4LIB)))
COMPILE+=/D WIN32
LINK_SAMPLE+=/machine:I386
endif

ifneq (,$(findstring sdk-w64i,$(V4LIB)))
COMPILE+=/D WIN64
LINK_SAMPLE+=/machine:X64 /libpath:../../lib-icl64 libirc.lib libcmt.lib
endif

ifneq (,$(findstring sdk-w64g,$(V4LIB)))
COMPILE+=/D WIN64
LINK_SAMPLE+=/machine:X64
endif

CMD=$(COMPILE) && $(LINK_SAMPLE) && rm $@.obj
endif

endif




#-------------------------------------------------------------------------------
# mac os x
ifneq (,$(findstring sdk-m,$(V4LIB)))

ifndef V4CC
V4CC=gcc
endif

CMD=$(V4CC) -I../inc -O3 -L../lib $^ -o $@ -l$(SDKLIB:lib%.a=%) -lm -lpthread $(V4OPTS)

ifneq (,$(findstring sdk-m32g,$(V4LIB)))
CMD+=-DMAC32 -m32
endif

ifneq (,$(findstring sdk-m64g,$(V4LIB)))
CMD+=-DMAC64 -m64
endif

endif





#-------------------------------------------------------------------------------
#
all: $(TARGETS)

../bin/%$(EXEID):%.c
	$(CMD)

clean:
	@rm $(TARGETS)




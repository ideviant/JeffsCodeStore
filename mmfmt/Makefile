LIBSIMDIRS = zbase/libsim
LIBMMFDIRS = libmmf
INCLUDES = $(LIBSIMDIRS):$(LIBMMFDIRS)
LIBSIM = $(LIBSIMDIRS)/libsim.a
LIBMMF = $(LIBMMFDIRS)/libmmf.a

CC = gcc
CFLAGS = -c -O2
LIBS = -lm

TMPDIR = mk.tmp
BINSRCS = mmfmain.c 
BINOBJS = $(BINSRCS:%.c=$(TMPDIR)/%.o)
OUTBIN = mmfmt


.PHONY: all
all : libmmf $(OUTBIN) 


.PHONY: clean
clean: 
	@echo; echo "cleaning ..."
	rm -rf $(TMPDIR)
	rm -rf $(OUTBIN)
	cd $(LIBMMFDIRS); make clean
	
$(OUTBIN): $(BINOBJS) $(LIBSIM) $(LIBMMF) | libmmf
	@echo; echo "[LD] linking ..."
	cc -I$(LIBSIMDIRS) -I$(LIBMMFDIRS) -o $@ $^ 

$(BINOBJS): $(LIBMMF) Makefile
$(BINOBJS): $(TMPDIR)/%.o:%.c | $(TMPDIR)
	@echo; echo "[CC] compiling: $< "
	$(CC) $(CFLAGS) -I$(LIBSIMDIRS) -I$(LIBMMFDIRS) -l$(LIBSIM) -l$(LIBMMF) -o $@ $<

.PHONY: libmmf
libmmf $(LIBMMF):
	cd $(LIBMMFDIRS); make

$(TMPDIR):
	mkdir $(TMPDIR)

LIBSIMDIRS = zbase/libsim
INCLUDES = $(LIBSIMDIRS)
LIBSIM = $(LIBSIMDIRS)/libsim.a

CC = gcc
CFLAGS = -c -O3
LIBS = -lm

TMPDIR = mk.tmp
BINSRCS = avc_exp.c cabac.c 
BINOBJS = $(BINSRCS:%.c=$(TMPDIR)/%.o)
OUTBIN = avcexp


.PHONY: all
all : libsim $(OUTBIN) 

.PHONY: clean
clean: 
	@echo; echo "cleaning ..."
	rm -rf $(TMPDIR)
	rm -rf $(OUTBIN)
	cd $(LIBSIMDIRS); make clean
	
$(OUTBIN): $(BINOBJS) $(LIBSIM) | libsim
	@echo; echo "[LD] linking ..."
	cc -o $@ $^

$(BINOBJS): Makefile
$(BINOBJS): $(TMPDIR)/%.o:%.c | $(TMPDIR)
	@echo; echo "[CC] compiling: $< "
	$(CC) $(CFLAGS) -I$(LIBSIMDIRS) -l$(LIBSIM) -o $@ $<

.PHONY: libsim
libsim $(LIBSIM):
	cd $(LIBSIMDIRS); make
    
$(TMPDIR):
	mkdir $(TMPDIR)
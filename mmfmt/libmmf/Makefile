LIBSIMDIRS = ../zbase/libsim
LIBMMFDIRS = 
INCLUDES = $(LIBSIMDIRS):$(LIBMMFDIRS)
LIBSIM = $(LIBSIMDIRS)/libsim.a

CC = gcc
CFLAGS = -c -O2
LIBS = -lm

TMPDIR = mk.tmp
LIBMMFSRCS = mmfutils.c flvparse.c
LIBMMFOBJS = $(LIBMMFSRCS:%.c=$(TMPDIR)/%.o)
LIBMMF = libmmf.a


.PHONY: all
all : libsim $(LIBMMF)

.PHONY: clean
clean: 
	@echo; echo "cleaning ..."
	rm -rf $(TMPDIR)
	rm -rf $(LIBMMF)
	cd $(LIBSIMDIRS); make clean

$(LIBMMF): $(LIBMMFOBJS) | libsim
	@echo; echo "[AR] linking '$@' ..."
	rm -f $@
	$(AR) -crs $@ $^

$(LIBMMFOBJS): $(LIBSIM) *.h Makefile
$(LIBMMFOBJS): $(TMPDIR)/%.o:%.c | $(TMPDIR)
	@echo; echo "[CC] compiling: $< "
	$(CC) $(CFLAGS) -I$(LIBSIMDIRS) -I$(LIBMMFDIRS) -l$(LIBSIM) -o $@ $<

.PHONY: libsim
libsim $(LIBSIM):
	cd $(LIBSIMDIRS); make

$(TMPDIR):
	mkdir $(TMPDIR)

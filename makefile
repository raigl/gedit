# Makefile for the Glaschick editor

BIN     =  edit
EDIT    =  ge
VIEW    =  gv
ALTER   =  ga
INSTDIR =  /usr/local/bin
DOCFILE =  edit.doc
EDBACK  =  *~
OBJECTS =  editm.o editu.o editf.o editl.o edits.o editt.o editv.o
MODEL   =
RELVER  =  5.4
CFLAGS  =  -ansi $(MODEL) -DLINUX -DCURSES -g
LDLIBS  =  -lncursesw

tq:     tq.c
	cc -o tq tq.c $(LDLIBS)

ct:     ct.c
	cc -g -o ct ct.c $(LDLIBS)

$(BIN): $(OBJECTS)
	cc $(CFLAGS) -o $(BIN) $(OBJECTS) $(LDLIBS)

all:    $(BIN) install doc clean

install: $(BIN)
	-mv $(INSTDIR)/$(EDIT) $(INSTDIR)/$(EDIT).old
	-mv $(INSTDIR)/$(VIEW) $(INSTDIR)/$(VIEW).old
	-mv $(INSTDIR)/$(ALTER) $(INSTDIR)/$(ALTER).old
	cp ./$(BIN) $(INSTDIR)/$(EDIT)
	strip $(INSTDIR)/$(EDIT)
	-test ! -x $(INSTDIR)/$(VIEW) && ln $(INSTDIR)/$(EDIT) $(INSTDIR)/$(VIEW)
	-test ! -x $(INSTDIR)/$(ALTER) && ln $(INSTDIR)/$(EDIT) $(INSTDIR)/$(ALTER)

doc:    edit.prf
# proff will most probably not exist, do not clear DOCFILE
	which proff && proff edit.prf > $(DOCFILE)

clean:
	rm -f $(OBJECTS)
	rm -f a.out
	rm -f core
	rm -f $(EDBACK)
	rm -f tq ct reply

editv.c: makefile  # -- not yet $(OBJECTS:.o=.c)
	echo "char *relver   = \" $(RELVER)\";" > editv.c
	echo "char *viewpgm  = \"$(VIEW)\";" >> editv.c
	echo "char *alterpgm = \"$(ALTER)\";" >> editv.c
editv.o: editv.c
	cc  $(CFLAGS) -c editv.c

editm.o: editm.c edit.h
	cc  $(CFLAGS) -c editm.c

editu.o: editu.c edit.h
	cc  $(CFLAGS) -c editu.c

editf.o: editf.c edit.h
	cc  $(CFLAGS) -c editf.c

editl.o: editl.c edit.h
	cc  $(CFLAGS) -c editl.c

edits.o: edits.c edit.h
	cc  $(CFLAGS) -c edits.c

editt.o: editt.c edit.h
	cc  $(CFLAGS) -c editt.c

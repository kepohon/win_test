# makefile for GNU Make
#	2023/05/15 by kepohon

.SUFFIXES:	.cpp .rc

NAME=appli

PROG=${NAME}.exe
SRC=${NAME}.cpp
OBJS=$(SRC:.cpp=.o)

#RSRC=resource
#HEAD=${RSRC}.h
# .rcファイルのエンコーディングはShift-JISでないとコンパイルできない(windresの仕様)
#RCS=${RSRC}.rc
#OBJS+=$(RCS:.rc=.o)


CC=g++
RESCOM=windres

#CFLAGS=-Wall -O3
# ソースファイルがUTF-8のときに使用する
CFLAGS=-Wall -O3 -finput-charset=utf-8 -fexec-charset=CP932

# libwinmm.a をリンクしたい場合次のように"-lwinmm"を追加する
#LDFLAGS=-mwindows -lwinmm
#LDFLAGS=-mwindows -lmingw32
LDFLAGS=-mwindows

RM=del


.PHONY : all
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) ${CFLAGS} $(LDFLAGS) -o $@


%.o: %.cpp ${HEAD}
	$(CC) $(CFLAGS) -o $@ -c $<
#.cpp.o :
#	$(CC) $(CFLAGS) -o $@ -c $<

#%.o: %.rc ${HEAD}
#	${RESCOM} $< $@
#.rc.o :
#	$(RESCOM) $< $@

.PHONY : clean run
clean:
	$(RM) $(OBJS)
	$(RM) $(PROG)

run:
	${PROG}

</$objtype/mkfile

HFILES = \
	names.h

NAMING = names.$O

TARG = \
	name_server

BIN=$home/bin/$objtype

PROGS=${TARG:%=$O.%}
DOTS=${TARG:%=.%}
MANFILES=${TARG:%=%.man}
LDFLAGS=
YFLAGS=-d

none:VQ:
	echo usage: mk all, install, installall, '$O'.cmd, cmd.install, or cmd.installall

all:V:	$PROGS

$O.%:	%.$O $OFILES $LIB
	$LD $LDFLAGS -o $target $prereq

%.$O:	$HFILES		# don't combine with following %.$O rules

%.$O:	%.c
	$CC $CFLAGS $stem.c

%.$O:	%.s
	$AS $AFLAGS $stem.s

y.tab.h y.tab.c:	$YFILES
	$YACC $YFLAGS $prereq

lex.yy.c:	$LFILES
	$LEX $LFLAGS $prereq

# [$OS].??* avoids file names like 9.h

clean:V:
	rm -f *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].??* $TARG $CLEANFILES

%.clean:V:
	rm -f $stem.[$OS] [$OS].$stem $stem.acid $stem

%.acid: %.$O $HFILES
	$CC $CFLAGS -a $stem.c >$target

$O.name_server: name_server.$O $NAMING

name_server:V: $O.name_server

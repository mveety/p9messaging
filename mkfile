</$objtype/mkfile

HFILES = \
	tags.h \
	msgnames.h \
	testing.h \

NAMING = tags.$O msgnames.$O

TESTING = testing.$O

TARG = \
	badrecv \
	blackhole \
	echo \
	kmbox_test \
	mbox_test \
	msgtiming \
	name_server \
	msgnamectl \
	msgtest_target \
	msgwait_target

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

$O.name_server: name_server.$O tags.$O

$O.msgnamectl: msgnamectl.$O $NAMING

$O.msgtiming: msgtiming.$O $NAMING

$O.msgtest_target: msgtest_target.$O $NAMING $TESTING

$O.msgwait_target: msgwait_target.$O $NAMING $TESTING

badrecv:V: $O.badrecv

blackhole:V: $O.blackhole

echo:V: $O.echo

kmbox_test:V: $O.kmbox_test

mbox_test:V: $O.mbox_test

msgtiming:V: $O.msgtiming

name_server:V: $O.name_server

msgnamectl:V: $O.msgnamectl

msgtest_target:V: $O.msgtest_target

msgwait_target:V: $O.msgwait_target

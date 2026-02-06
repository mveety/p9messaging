</$objtype/mkfile

TARG=\
	badrecv \
	blackhole \
	echo \
	kmbox_test \
	mbox_test \
	msgtiming

BIN=$home/bin/$objtype

</sys/src/cmd/mkmany

$O.name_server: name_server.$O tags.$O

$O.msgnamectl: msgnamectl.$O tags.$O msgnames.$O

$O.msgtiming: msgtiming.$O tags.$O msgnames.$O

$O.msgtest_target: msgtest_target.$O tags.$O msgnames.$O

badrecv:V: $O.badrecv

blackhole:V: $O.blackhole

echo:V: $O.echo

kmbox_test:V: $O.kmbox_test

mbox_test:V: $O.mbox_test

msgtiming:V: $O.msgtiming

name_server:V: $O.name_server

msgnamectl:V: $O.msgnamectl

msgtest_target:V: $O.msgtest_target

all: badrecv blackhole echo kmbox_test mbox_test msgtiming name_server msgtest_target msgnamectl

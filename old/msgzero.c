#include <u.h>
#include <libc.h>
#include <msg.h>

int
main(int argc, char *argv[])
{
	char msgbody[] = "Hello world!";
	USED(argc);
	USED(argv);

	msgenable();

	if(sys_msgsend(0, &msgbody[0], sizeof(msgbody)) < 0){
		fprint(2, "error: unable to send message to zero: %r\n");
		exits("msgsend");
	}

	exits(nil);
}

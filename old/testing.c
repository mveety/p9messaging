#include <u.h>
#include <libc.h>
#include <msg.h>
#include "testing.h"

void
tryexitmessage(void *msgdata, uintptr msgsz)
{
	MMdata *mmdata;

	if(msgsz < sizeof(MMdata))
		return;
	mmdata = msgdata;
	if(mmdata->magic = MsgMagic && mmdata->tag == -2){
		fprint(2, "got exit message. quitting...\n");
		exits(nil);
	}
}

int
wirecpu(int cpu, int pid)
{
	char *procctl;
	int procctlfd;

	if(cpu < 0)
		return -1;

	procctl = smprint("/proc/%d/ctl", pid);
	if(!procctl)
		abort();

	procctlfd = open(procctl, OWRITE);
	if(procctlfd < 0)
		return -1;

	fprint(procctlfd, "wired %d\n", cpu);

	close(procctlfd);
	free(procctl);

	return 0;
}

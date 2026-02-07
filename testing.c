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

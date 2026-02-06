/* Non-confidential */
/* No warranty in life supporting applications */

#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"

enum {
	ExitMessageSz = sizeof(s32int),
	NameStatusSz = 2*sizeof(s32int),
	ResolvedNameSz = 2*sizeof(s32int)+1,
	RegisterNameSz = sizeof(u32int)+1,
	NameRequestSz = sizeof(u32int)+1,
};

void
free_systemmessage(SystemMessage *smsg)
{
	freemsg(smsg->carrier);
	free(smsg);
}

static SystemMessage*
alloc_systemmessage(Message *msg)
{
	SystemMessage *smsg;

	if(!(smsg = mallocz(sizeof(SystemMessage), 1)))
		return nil;

	smsg->carrier = msg;
	smsg->tag = &msg->tag;
	smsg->pid = &msg->pid;

	return smsg;
}

static int
parse_exitmessage(SystemMessage *smsg)
{
	Message *msg;

	msg = smsg->carrier;

	if(msg->len < ExitMessageSz)
		return -1;
	smsg->exitmsg.code = msg->data;
	return 0;
}

static int
parse_namestatus(SystemMessage *smsg)
{
	Message *msg;
	char *msgdata;

	msg = smsg->carrier;
	msgdata = msg->data;

	if(msg->len < NameStatusSz)
		return -1;

	smsg->namestatus.request_tag = (s32int*)msgdata;
	smsg->namestatus.request_status = (s32int*)(msgdata+(1*sizeof(s32int)));

	return 0;
}

static int
parse_resolvedname(SystemMessage *smsg)
{
	Message *msg;
	char *msgdata;

	msg = smsg->carrier;
	msgdata = msg->data;

	if(msg->len < ResolvedNameSz)
		return -1;

	smsg->resolvedname.pid = (s32int*)msgdata;
	smsg->resolvedname.namelen = (u32int*)(msgdata+(1*sizeof(s32int)));
	smsg->resolvedname.name = (msgdata+(2*sizeof(s32int)));
	return 0;
}

static int
parse_registername(SystemMessage *smsg)
{
	Message *msg;
	char *msgdata;

	msg = smsg->carrier;
	msgdata = msg->data;

	if(msg->len < RegisterNameSz)
		return -1;

	smsg->registername.namelen = (u32int*)msgdata;
	smsg->registername.name = (msgdata+sizeof(s32int));
	return 0;
}

static int
parse_namerequest(SystemMessage *smsg)
{
	Message *msg;
	char *msgdata;

	msg = smsg->carrier;
	msgdata = msg->data;

	if(msg->len < NameRequestSz)
		return -1;

	smsg->namerequest.namelen = (u32int*)msgdata;
	smsg->namerequest.name = (msgdata+sizeof(s32int));
	return 0;
}

SystemMessage*
parse_systemmsg(Message *msg)
{
	SystemMessage *smsg;

	if(msg->tag > -2)
		return nil; // not a system message
	if(!(smsg = alloc_systemmessage(msg)))
		return nil;

	switch(msg->tag){
	default:
		free(smsg);
		return nil;
	case TagExit:
		if(parse_exitmessage(smsg) < 0){
			free(smsg);
			return nil;
		}
		break;
	case TagResolvedName:
		if(parse_resolvedname(smsg) < 0){
			free(smsg);
			return nil;
		}
		break;
	case TagNameStatus:
		if(parse_namestatus(smsg) < 0){
			free(smsg);
			return nil;
		}
		break;
	case TagRequestName:
		if(parse_namerequest(smsg) < 0){
			free(smsg);
			return nil;
		}
		break;
	case TagRegisterName:
		if(parse_registername(smsg) < 0){
			free(smsg);
			return nil;
		}
		break;
	}

	return smsg;
}

SystemMessage*
new_exitmessage(s32int code)
{
	Message *msg;
	SystemMessage *smsg;

	if(!(msg = message(TagExit, &code, sizeof(u32int))))
		return nil;

	if(!(smsg = alloc_systemmessage(msg))){
		freemsg(msg);
		return nil;
	}

	if(parse_exitmessage(smsg) < 0){
		free_systemmessage(smsg);
		return nil;
	}

	return smsg;
}

SystemMessage*
new_namestatus(s32int request_tag, s32int request_status)
{
	char buffer[2*sizeof(s32int)];
	Message *msg;
	SystemMessage *smsg;

	memmove(&buffer[0], &request_tag, sizeof(s32int));
	memmove(&buffer[4], &request_status, sizeof(s32int));

	if(!(msg = message(TagNameStatus, &buffer[0], sizeof(buffer))))
		return nil;

	if(!(smsg = alloc_systemmessage(msg))){
		freemsg(msg);
		return nil;
	}

	if(parse_namestatus(smsg) < 0){
		free_systemmessage(smsg);
		return nil;
	}

	return smsg;
}

SystemMessage*
new_resolvedname(s32int pid, char *name, u32int namelen)
{
	char *buffer;
	Message *msg;
	SystemMessage *smsg;

	buffer = mallocz(sizeof(s32int)+sizeof(u32int)+namelen, 1);
	if(!buffer)
		return nil;

	memmove(&buffer[0], &pid, sizeof(s32int));
	memmove(&buffer[4], &namelen, sizeof(u32int));
	memmove(&buffer[8], name, namelen);

	msg = message(TagResolvedName, buffer, sizeof(s32int)+sizeof(u32int)+namelen);
	if(!msg){
		free(buffer);
		return nil;
	}
	free(buffer);

	if(!(smsg = alloc_systemmessage(msg))){
		freemsg(msg);
		return nil;
	}

	if(parse_resolvedname(smsg) < 0){
		free_systemmessage(smsg);
		return nil;
	}

	return smsg;
}

SystemMessage*
new_registername(char *name, u32int namelen)
{
	char *buffer;
	Message *msg;
	SystemMessage *smsg;

	buffer = mallocz(sizeof(u32int)+namelen, 1);
	if(!buffer)
		return nil;

	memmove(&buffer[0], &namelen, sizeof(u32int));
	memmove(&buffer[4], name, namelen);

	msg = message(TagRegisterName, buffer, sizeof(u32int)+namelen);
	if(!msg){
		free(buffer);
		return nil;
	}
	free(buffer);

	if(!(smsg = alloc_systemmessage(msg))){
		freemsg(msg);
		return nil;
	}

	if(parse_registername(smsg) < 0){
		free_systemmessage(smsg);
		return nil;
	}

	return smsg;
}

SystemMessage*
new_namerequest(char *name, u32int namelen)
{
	char *buffer;
	Message *msg;
	SystemMessage *smsg;

	buffer = mallocz(sizeof(u32int)+namelen, 1);
	if(!buffer)
		return nil;

	memmove(&buffer[0], &namelen, sizeof(u32int));
	memmove(&buffer[4], name, namelen);

	msg = message(TagRequestName, buffer, sizeof(u32int)+namelen);
	if(!msg){
		free(buffer);
		return nil;
	}
	free(buffer);

	if(!(smsg = alloc_systemmessage(msg))){
		freemsg(msg);
		return nil;
	}

	if(parse_namerequest(smsg) < 0){
		free_systemmessage(smsg);
		return nil;
	}

	return smsg;
}

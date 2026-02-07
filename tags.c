/* Non-confidential */
/* No warranty in life supporting applications */

#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"

void
free_systemmessage(SystemMessage *smsg)
{
	freemsg(smsg->msg);
	free(smsg);
}

static SystemMessage*
alloc_systemmessage(Message *msg)
{
	SystemMessage *smsg;

	if(!(smsg = mallocz(sizeof(SystemMessage), 1)))
		return nil;

	smsg->msg = msg;

	return smsg;
}

static int
parse_exitmessage(SystemMessage *smsg)
{
	if(smsg->msg->len < sizeof(ExitMessage))
		return -1;
	smsg->exitmsg = smsg->msg->data;
	smsg->tag = smsg->msg->tag;
	return 0;
}

static int
parse_namestatus(SystemMessage *smsg)
{
	if(smsg->msg->len < sizeof(NameStatusMsg))
		return -1;

	smsg->namestatus = smsg->msg->data;
	smsg->tag = smsg->msg->tag;
	return 0;
}

static int
parse_resolvedname(SystemMessage *smsg)
{
	if(smsg->msg->len < sizeof(ResolvedNameMsg))
		return -1;

	smsg->resolvedname = smsg->msg->data;
	smsg->tag = smsg->msg->tag;
	return 0;
}

static int
parse_registername(SystemMessage *smsg)
{
	if(smsg->msg->len < sizeof(RegisterNameMsg))
		return -1;

	smsg->registername = smsg->msg->data;
	smsg->tag = smsg->msg->tag;
	return 0;
}

static int
parse_namerequest(SystemMessage *smsg)
{
	if(smsg->msg->len < sizeof(NameRequestMsg))
		return -1;

	smsg->namerequest = smsg->msg->data;
	smsg->tag = smsg->msg->tag;
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
	ExitMessage exitmsg;

	exitmsg.code = code;
	if(!(msg = message(TagExit, &exitmsg, sizeof(ExitMessage))))
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
	Message *msg;
	SystemMessage *smsg;
	NameStatusMsg nsmsg;

	nsmsg.request_tag = request_tag;
	nsmsg.request_status = request_status;

	if(!(msg = message(TagNameStatus, &nsmsg, sizeof(NameStatusMsg))))
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
	Message *msg;
	SystemMessage *smsg;
	ResolvedNameMsg *rnamemsg;

	rnamemsg = mallocz(sizeof(ResolvedNameMsg)+namelen, 1);
	if(!rnamemsg)
		return nil;

	rnamemsg->pid = pid;
	rnamemsg->namelen = namelen;
	memmove(&rnamemsg->name[0], name, namelen);

	msg = message(TagResolvedName, rnamemsg, sizeof(ResolvedNameMsg)+namelen);
	if(!msg){
		free(rnamemsg);
		return nil;
	}
	free(rnamemsg);

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
	Message *msg;
	SystemMessage *smsg;
	RegisterNameMsg *rnamemsg;

	rnamemsg = mallocz(sizeof(RegisterNameMsg)+namelen, 1);
	if(!rnamemsg)
		return nil;

	rnamemsg->namelen = namelen;
	memmove(&rnamemsg->name[0], name, namelen);

	msg = message(TagRegisterName, rnamemsg, sizeof(RegisterNameMsg)+namelen);
	if(!msg){
		free(rnamemsg);
		return nil;
	}
	free(rnamemsg);

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
	Message *msg;
	SystemMessage *smsg;
	NameRequestMsg *nreqmsg;

	nreqmsg = mallocz(sizeof(NameRequestMsg)+namelen, 1);
	if(!nreqmsg)
		return nil;

	nreqmsg->namelen = namelen;
	memmove(&nreqmsg->name[0], name, namelen);

	msg = message(TagRequestName, nreqmsg, sizeof(NameRequestMsg)+namelen);
	if(!msg){
		free(nreqmsg);
		return nil;
	}
	free(nreqmsg);

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

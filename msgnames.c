#include <u.h>
#include <libc.h>
#include <msg.h>
#include "tags.h"
#include "msgnames.h"

int
register_name(int nspid, char *name)
{
	SystemMessage *smsg, *resp;
	Message *respmsg;
	int tags[] = {TagExit, TagNameStatus};
	int retval;

	smsg = new_registername(name, strlen(name)+1);
	if(msgsend(nspid, smsg->carrier) < 0){
		free_systemmessage(smsg);
		return 1;
	}
	free_systemmessage(smsg);

retry:
	respmsg = msgrecvfilter(nil, tags, nelem(tags));
	if(respmsg == nil)
		return 2;

	if(respmsg->pid != nspid){
		// supurious message?
		freemsg(respmsg);
		goto retry;
	}

	resp = parse_systemmsg(respmsg);

	switch(*resp->tag){
	case TagExit:
		abort();
	case TagNameStatus:
		if(*resp->namestatus.request_tag != TagRegisterName){
			free_systemmessage(resp);
			return 3;
		}
		retval = *resp->namestatus.request_status;
		free_systemmessage(resp);
		return retval;
	}

	return 4;
}

int
query_name(int nspid, char *name)
{
	SystemMessage *smsg, *resp;
	Message *respmsg;
	int tags[] = {TagExit, TagNameStatus, TagResolvedName};
	int retval = 0;

	smsg = new_namerequest(name, strlen(name)+1);
	if(msgsend(nspid, smsg->carrier) < 0){
		free_systemmessage(smsg);
		return -128;
	}
	free_systemmessage(smsg);

retry:
	respmsg = msgrecvfilter(nil, tags, nelem(tags));
	if(respmsg == nil)
		return -129;

	if(respmsg->pid != nspid){
		// supurious message?
		freemsg(respmsg);
		goto retry;
	}

	resp = parse_systemmsg(respmsg);

	switch(*resp->tag){
	case TagExit:
		abort();
	case TagNameStatus:
		if(*resp->namestatus.request_tag != TagRequestName){
			free_systemmessage(resp);
			return -130;
		}
		retval = *resp->namestatus.request_status;
		break;
	case TagResolvedName:
		if(strcmp(resp->resolvedname.name, name) == 0)
			retval = *resp->resolvedname.pid;
		else
			retval = -1;
		break;
	}

	free_systemmessage(resp);
	return retval;
}

int
name_server(char *srvname)
{
	int srvfd;
	char buffer[32];
	int nspid;

	if(srvname == nil)
		srvname = "/srv/name_server";

	srvfd = open(srvname, ORDWR);
	if(srvfd < 0){
		fprint(2, "error: unable to open %s: %r\n", srvname);
		return -1;
	}
	memset(&buffer[0], 0, sizeof(buffer));
	fprint(srvfd, "pid");
	if(read(srvfd, buffer, sizeof(buffer)) < 10){
		close(srvfd);
		fprint(2, "error: invalid srv message: %r\n");
		return -1;
	}
	close(srvfd);
	nspid = atoi(buffer);
	if(nspid == 0){
		fprint(2, "error: unable to get name_server pid\n");
		return -1;
	}

	return nspid;
}

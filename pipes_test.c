#include <u.h>
#include <libc.h>

int
blackhole(int fd, uintptr readsize)
{
	void *buffer;
	uintptr readsz;

	buffer = mallocz(readsize+1, 1);
	if(!buffer)
		abort();

	fprint(2, "pid %d: blackhole started\n", getpid());
	for(;;){
		readsz = readn(fd, buffer, readsize);
		if(readsz < readsize){
			fprint(2, "short read (%ulld < %ulld)\n", readsz, readsize);
			exits("short read");
		}
	}

	return 0;
}

char *argv0;

void
usage(void)
{
	fprint(2, "usage: %s [-n packets] [-s packet size]\n", argv0);
	exits("usage");
}

int
main(int argc, char *argv[])
{
	int packets = 1000000;
	int packetsize = 24;
	int p[2];
	void *packetdata;
	int randfd;
	int blackholepid;

	vlong start_time;
	vlong end_time;
	vlong total_time;
	vlong seconds;
	vlong milliseconds;

	argv0 = argv[0];
	ARGBEGIN{
	case 's':
		packetsize = atoi(EARGF(usage()));
		break;
	case 'n':
		packets = atoi(EARGF(usage()));
		break;
	default:
		usage();
	}ARGEND;

	pipe(p);
	packetdata = mallocz(packetsize+1, 1);
	if(!packetdata)
		abort();

	if((randfd = open("/dev/random", OREAD)) < 0){
		fprint(2, "unable to open /dev/random: %r\n");
		exits("open");
	}

	if(readn(randfd, packetdata, packetsize) < 0){
		fprint(2, "read error: %r\n");
		exits("readn");
	}

	switch((blackholepid = rfork(RFPROC|RFMEM|RFNOWAIT))){
	case 0:
		blackhole(p[1], packetsize);
		exits(nil);
		break;
	case -1:
		fprint(2, "error: unable to rfork: %r\n");
		exits("rfork");
		break;
	default:
		break;
	}

	fprint(2, "test start: sending %d messages of %d bytes over pipe\n",
		packets, packetsize);

	start_time = nsec();
	for(int i = 0; i < packets; i++){
		if(write(p[0], packetdata, packetsize) < 0){
			fprint(2, "write error on cycle %d: %r\n", i);
			postnote(PNPROC, blackholepid, "kill");
			exits("write");
		}
	}
	end_time = nsec();

	total_time = end_time - start_time;
	seconds = total_time/1000000000;
	milliseconds = (total_time-(seconds*1000000000))/1000000;

	fprint(2, "took %lld.%03lld seconds to send %d packets\n",
		seconds, milliseconds, packets);

	postnote(PNPROC, blackholepid, "kill");
	exits(nil);
}

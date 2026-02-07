
enum {
	MsgMagic = 0xdeadbeef,
};

#pragma pack on
typedef struct {
	u32int magic;
	u32int tag;
	u32int pid;
	char data[1];
} MMdata;
#pragma pack off

extern void tryexitmessage(void *, uintptr);

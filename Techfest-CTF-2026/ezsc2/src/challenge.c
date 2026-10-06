#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/openat2.h>
#include <linux/seccomp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef __NR_getdents64
#define __NR_getdents64 217
#endif

#ifndef __NR_openat2
#define __NR_openat2 437
#endif

const unsigned char stub[] = {
        0x48,0x31,0xc0,
        0x48,0x31,0xdb,
        0x48,0x31,0xc9,
        0x48,0x31,0xd2,
        0x48,0x31,0xff,
        0x48,0x31,0xf6,

        0x4d,0x31,0xc0,
        0x4d,0x31,0xc9,
        0x4d,0x31,0xd2,
        0x4d,0x31,0xdb,
        0x4d,0x31,0xe4,
        0x4d,0x31,0xed,
        0x4d,0x31,0xf6,
        0x4d,0x31,0xff,

	0x48, 0x31, 0xed,
	0x48, 0x31, 0xe4,
};

char *bless = NULL;

static void fatal(const char *msg)
{
	perror(msg);
	exit(1);
}

__attribute__((constructor))
static void setup(void)
{
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	bless = mmap((void*)0xa55000, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (bless == MAP_FAILED) { fatal("mmap"); }
}

static int install_seccomp(void)
{
	struct sock_filter filter[] = {
		/* Check arch */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, arch)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 7),
		// BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 8),

		/* Load syscall number */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),

		/* Allowed syscalls */
		// BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write,       7, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read,        6, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_close,       5, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit,        4, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit_group,  3, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_openat2,     2, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_getdents64,  1, 0),

		/* Deny everything else */
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),

		/* Allow matched syscall */
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
	};

	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) { return -1; }

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0) { return -1; }

	return 0;
}

void ezsc2(void)
{
	char *sc = mmap((void*)0xdeadb07000, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (sc == MAP_FAILED) { fatal("mmap"); }

	memcpy(sc, stub, sizeof(stub));
	printf("Combo> ");
	read(0, sc + sizeof(stub), (PAGE_SIZE - sizeof(stub)));
	if (mprotect(sc, 0x1000, PROT_READ | PROT_EXEC) != 0) { fatal("mprotect"); }

	if (install_seccomp() != 0) { fatal("install_seccomp"); }
	((void (*)(void))sc)();

	_exit(0);
}

int main(void)
{
	ezsc2();
    	return 0;
}
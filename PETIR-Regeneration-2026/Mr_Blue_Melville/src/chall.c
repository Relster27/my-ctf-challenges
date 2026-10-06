#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// gcc chall.c -o chall -fPIE -Wl,-z,relro,-z,now

#define DONE "***"
#define FAIL "!!!"
#define INVALID "???"

uint64_t *x = NULL;
uint64_t *y = NULL;

char *first;
char *second;
char *third;

__attribute__((constructor))
void setup(void)
{
        srand(time(NULL));
        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);

        first  = malloc(0x10);
        second = malloc(0x410);
        third  = malloc(0x10);

        memset(first, 0x11, 0x10);
        memset(third, 0x13, 0x10);

        x = (uint64_t*)first;
        y = (uint64_t*)third;
}

void help(void)
{
        puts("commands:");
        puts(" help  - show commands");
        puts(" open  - open file");
        puts(" write - write report");
        puts(" exit");
}

void history(void)
{
        uintptr_t start = (uintptr_t)x;
        uintptr_t end   = (uintptr_t)y;

        if (start > end) {
                uintptr_t tmp = start;
                start = end;
                end = tmp;
        }

        uintptr_t range = end - start;
        uintptr_t heap_leak = start;

        if (range > 0) {
                heap_leak += rand() % range;
        }

        printf("Yoo ref do somethin %p\n", (void *)heap_leak);
        puts(DONE);
}

void open_file(void)
{
        char path[0x40];

        printf("Path: ");
        if (!fgets(path, sizeof(path), stdin)) return;
        path[strcspn(path, "\n")] = 0;

        int fd = open(path, O_RDONLY);
        if (fd < 0) {
                perror("open");
                puts(FAIL);
                return;
        }

        char buf[0x100];
        int n = read(fd, buf, 0x40);   // limited leak
        write(1, buf, n);
        write(1, "\n", 1);
        puts(DONE);
}

void write_buf(void)
{
        char save_buf[0x40];
        char content_buf[0x80];
        printf("Content: ");
        size_t n = read(0, content_buf, (0x80-1));

        printf("Save [Y\\N]? ");
        char save_flag = getchar();
        if (save_flag == 'Y' || save_flag == 'y') {
                memcpy(save_buf, content_buf, n);
                puts("Content saved");
        } else if (save_flag == 'N' || save_flag == 'n') {
                free(second); second = NULL;
                puts("Content not saved");
        } else {
                puts(INVALID);
                return;
        }

        puts(DONE);
}

void console(void)
{
        uint64_t *p;
        printf("[CONSOLE]: ");
        scanf("%llx", &p);

        if ((uintptr_t)p < (uintptr_t)x || (uintptr_t)p > (uintptr_t)y) {
                puts(FAIL);
                return;
        }

        printf(">>> 0x%llx\n", *p);
        puts(DONE);
}

void loop(void)
{
        char cmd[0x20];
        while (1) {
                printf("> ");

                if (!fgets(cmd, sizeof(cmd), stdin)) break;
                cmd[strcspn(cmd, "\n")] = 0;

                if (!strcmp(cmd, "help")) help();
                else if (!strcmp(cmd, "open")) open_file();
                else if (!strcmp(cmd, "write")) write_buf();
                else if (!strcmp(cmd, "console")) console();
                else if (!strcmp(cmd, "exit")) break;
                else puts(INVALID);
        }
}

int main(void)
{
        loop();
        return 0;
}
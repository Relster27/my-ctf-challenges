#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NAME_SIZE 0x40
#define MAX_ACTOR 10
#define FEMALE 0x1
#define MALE 0x2
#define OTHER 0xff

struct __attribute__((packed)) SmallFields {
        char actorGender;
        short actorAge;
        short actorHeight;
        short actorWeight;
};

struct Actor {
        struct SmallFields sf;
        char *actorName;
};

char *scratch_buf = NULL;
struct Actor *Actors[MAX_ACTOR];

__attribute__((constructor))
void init(void)
{
        setbuf(stdin, NULL);
        setbuf(stdout, NULL);
        setbuf(stderr, NULL);

        scratch_buf = malloc(NAME_SIZE);
        if (!scratch_buf) _exit(EXIT_FAILURE);
}

void banner(void)
{
        puts("________              ___.   .__           ___________                  ___.   .__          ");
        puts("\\______ \\   ____  __ _\\_ |__ |  |   ____   \\__    ___/______  ____  __ _\\_ |__ |  |   ____  ");
        puts(" |    |  \\ /  _ \\|  |  \\ __ \\|  | _/ __ \\    |    |  \\_  __ \\/  _ \\|  |  \\ __ \\|  | _/ __ \\ ");
        puts(" |    `   (  <_> )  |  / \\_\\ \\  |_\\  ___/    |    |   |  | \\(  <_> )  |  / \\_\\ \\  |_\\  ___/ ");
        puts("/_______  /\\____/|____/|___  /____/\\___  >   |____|   |__|   \\____/|____/|___  /____/\\___  >");
        puts("        \\/                 \\/          \\/                                    \\/          \\/ ");
        puts("");
        puts("****************************************************************************************");
        puts("* Welcome to the Double Trouble auditions.                                             *");
        puts("* ----------------------------------------                                             *");
        puts("*                                                                                      *");
        puts("* We are scouting for talented actors to star in our biggest film project of the year. *");
        puts("* Think you have what it takes? Prove your talent and register below.                  *");
        puts("****************************************************************************************");
}

void menu(void)
{
        puts("1. Request submission");
        puts("2. Remove submission");
        puts("3. Change submission");
        puts("4. Check submission");
}

int get_idx(void) // Ignore me (really)
{
        int idx;
        printf("Index: ");
        if (scanf("%d", &idx) != 1) {
                puts("Invalid input type!");
                _exit(EXIT_FAILURE);
        }
        getchar();
        return idx;
}

void print_creds(int idx)
{
        printf("Name    : %s\n", Actors[idx]->actorName);
        if (Actors[idx]->sf.actorGender == FEMALE)
                printf("Gender  : female\n");
        else if(Actors[idx]->sf.actorGender == MALE)
                printf("Gender  : male\n");
        else
                printf("Gender  : other\n");
        printf("Age     : %d years old\n", Actors[idx]->sf.actorAge);
        printf("Height  : %d cm\n", Actors[idx]->sf.actorHeight);
        printf("Weight  : %d kg\n", Actors[idx]->sf.actorWeight);
}

void input_creds(int idx)
{
        // Name field
        printf("Name: ");
        read(0, scratch_buf, NAME_SIZE - 1);
        scratch_buf[strcspn(scratch_buf, "\n")] = '\0';
        Actors[idx]->actorName = malloc(NAME_SIZE-1);
        strncpy(Actors[idx]->actorName, scratch_buf, NAME_SIZE-1);

        // Gender field
        printf("Gender (F/M): ");
        char gender = getchar();
        if (gender == 'f' || gender == 'F')
                Actors[idx]->sf.actorGender = FEMALE;
        else if (gender == 'm' || gender == 'M')
                Actors[idx]->sf.actorGender = MALE;
        else
                Actors[idx]->sf.actorGender = OTHER;
        
        // Age field
        printf("Age: ");
        scanf("%hd", &Actors[idx]->sf.actorAge); getchar();
        
        // Height field
        printf("Height: ");
        scanf("%hd", &Actors[idx]->sf.actorHeight); getchar();
        
        // Weight field
        printf("Weight: ");
        scanf("%hd", &Actors[idx]->sf.actorWeight); getchar();
}

void request_sub(int idx)
{
        if (idx < 0 || idx >= MAX_ACTOR) {
                puts("Invalid index!");
                return;
        }

        if (Actors[idx]) {
                puts("Index is occupied!");
                return;
        }

        Actors[idx] = malloc(sizeof(struct Actor));
        if (!Actors[idx]) _exit(EXIT_FAILURE);

        input_creds(idx);
}

void remove_sub(int idx)
{
        if (idx < 0 || idx >= MAX_ACTOR) {
                puts("Invalid index!");
                return;
        }

        if (Actors[idx] == NULL) {
                puts("No data yet!");
                return; 
        }

        free(Actors[idx]->actorName); Actors[idx]->actorName = NULL;
        free(Actors[idx]); Actors[idx] = NULL;
        puts("Submission removed successfully!");
}

void change_sub(int idx)
{
        if (idx < 0 || idx >= MAX_ACTOR) {
                puts("Invalid index!");
                return;
        }

        if (Actors[idx] == NULL) {
                puts("No data yet!");
                return; 
        }

        printf("New Name: ");
        read(0, Actors[idx]->actorName, NAME_SIZE - 1);
        Actors[idx]->actorName[strcspn(Actors[idx]->actorName, "\n")] = '\0';

        printf("New Gender (F/M): ");
        char gender = getchar();
        if (gender == 'f' || gender == 'F')
                Actors[idx]->sf.actorGender = FEMALE;
        else if (gender == 'm' || gender == 'M')
                Actors[idx]->sf.actorGender = MALE;
        else
                Actors[idx]->sf.actorGender = OTHER;
        
        printf("New Age: ");
        scanf("%hd", &Actors[idx]->sf.actorAge); getchar();

        printf("New Height: ");
        scanf("%hd", &Actors[idx]->sf.actorHeight); getchar();
        
        printf("New Weight: ");
        scanf("%d", &Actors[idx]->sf.actorWeight); getchar(); // VULN (type confusion)
}

void check_sub(int idx)
{
        if (idx < 0 || idx >= MAX_ACTOR) {
                puts("Invalid index!");
                return;
        }

        if (Actors[idx] == NULL) {
                puts("No data yet!");
                return; 
        }

        print_creds(idx);
}

int main(void)
{
        banner();
        menu();

        int opt;
        int idx;
        while (1) {
                printf("> ");
                scanf("%d", &opt); getchar();
                switch (opt) {
                        case 1:
                                idx = get_idx();
                                request_sub(idx);
                                break;
                        case 2:
                                idx = get_idx();
                                remove_sub(idx);
                                break;
                        case 3:
                                idx = get_idx();
                                change_sub(idx); // VULN (type confusion)
                                break;
                        case 4:
                                idx = get_idx();
                                check_sub(idx);
                                break;
                        case 5:
                                puts("I'm out...");
                                exit(EXIT_SUCCESS);
                        default:
                                puts("Invalid option");
                                break;
                }    
        }

        return EXIT_SUCCESS;
}

/*
        UAF -> Actors[idx]->name
        Type confusion on 'Weight' -> change_sub()
                - instead of short (%hd) it's taking int (%d)

        vuln function:
                - change_sub()
                - check_sub()
*/
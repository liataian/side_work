#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct player {
    int jersey_num; //4 bytes
    char name[5]; //5 bytes
};


int main() {
    FILE *fp = NULL;
    FILE *fp_read = NULL;
    struct player play;
    struct player *ptr = NULL;

    if ((fp = fopen("./player.txt", "wb")) == NULL) {
        printf("FILE open FAIL!\n");
        return -1;
    }

    //Init
    memset(&play, 0, sizeof(play));
    ptr = &play;
    printf("[write] ptr->name=%s, ptr->jersey_num=%d\n", ptr->name, ptr->jersey_num);

    //Try to write structure player to file
    strcpy(ptr->name, "kobe");
    ptr->jersey_num = 24;
    printf("[write] ptr->name=%s, ptr->jersey_num=%d\n", ptr->name, ptr->jersey_num);
    fwrite(ptr, sizeof(*ptr), 1, fp); //用vim打開player.txt會是編碼成ASCII的樣子，大小總共16 bytes: ^X^@^@^@kobe^@^?^@^@
    fclose(fp);
    printf("Write finish\n");

    //Reset
    memset(&play, 0, sizeof(play));
    printf("[read] play.name=%s, play.jersey_num=%d\n", play.name,  play.jersey_num);

    //Try to read value from file and assign to struct player again
    if ((fp_read = fopen("./player.txt", "rb")) == NULL) {
        printf("FILE open FAIL!\n");
        return -1;
    }

    fread(ptr, sizeof(*ptr), 1, fp_read);
    printf("[read] ptr->name=%s, ptr->jersey_num=%d\n", ptr->name, ptr->jersey_num);
    fclose(fp_read);
    printf("Read finish\n");
    return 0;
}

/*
用vim打開player.txt會是編碼成ASCII的樣子，大小總共16 bytes: ^X^@^@^@kobe^@^?^@^@

^X   ^@    ^@    ^@    k    o    b    e    ^@    ^?   ^@   ^@
24   NULL  NULL  NULL  6b   6f   62   65   NULL

為何檔案大小是12 bytes? 因為結構要4 bytes對齊，以成員中最大的int來對齊，最後對齊為12 bytes
*/

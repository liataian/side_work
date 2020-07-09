#include <stdio.h>
#include <string.h>
 
#define SIZE 40
 
int main(void) {
        char buf[SIZE] = "computer program";
        char * ptr;
        int ch = 'p';
        /* This illustrates strchr */
        ptr = strchr( buf, ch );
        printf( "The first occurrence of %c in '%s' is '%s'\n", ch, buf, ptr );
        /* This illustrates strrchr */
        ptr = strrchr( buf, ch );
        printf( "The last occurrence of %c in %s is %s (%p)\n", ch, buf, ptr, ptr);
        return 0;
}

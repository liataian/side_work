#include<stdio.h>

int main(void)
{
    char *names1[]={"A", "B", "C"}; // Three elements
    char names11[3][5]={"AAA", "BBBB", "CCCC"};

    char *names2[]={"A", "", "C"}; // Three elements
    char *names3[]={"", "A", "C", ""}; // Four elements
    char *names4[]={"John", "Paul", "George", "Ringo"}; // Four elements
    char *names5[]={"", "B", NULL, NULL, "E"}; // Five elements

    printf("1. size=%zu, len= %zu\n",sizeof(names1), sizeof(names1)/sizeof(names1[0]));
    printf("11. size=%zu, len= %zu\n",sizeof(names11), sizeof(names11)/sizeof(names11[0]));

    printf("2. size=%zu, len= %zu\n",sizeof(names2), sizeof(names2)/sizeof(names2[0]));
    printf("3. size=%zu, len= %zu\n",sizeof(names3), sizeof(names3)/sizeof(names3[0]));
    printf("4. size=%zu, len= %zu\n",sizeof(names4), sizeof(names4)/sizeof(names4[0]));
    printf("5. size=%zu, len= %zu\n",sizeof(names5), sizeof(names5)/sizeof(names5[0]));
    return 0;
}


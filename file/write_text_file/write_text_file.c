#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp_text = NULL;
    FILE *fp_bin = NULL;
    int num = 12345;

    //Text mode
    fp_text = fopen("text", "w");
    fprintf(fp_text, "%d", num);
    fclose(fp_text);
    //Binary mode
    fp_bin = fopen("bin", "wb");
    fwrite(&num, sizeof(int), 1, fp_bin);
    fclose(fp_bin);
    return 0;
}


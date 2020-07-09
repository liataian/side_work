#include <stdio.h>
#include <stdlib.h>

int main() {
	int A[2][3] = { {1, 2, 3}, {4, 5, 6} };
	printf("A[1][0]=%d, A[0][3]=%d\n", A[1][0], A[0][3]); //array是線性的，所以A[1][0]會等於A[0][3]
	return 0;
}

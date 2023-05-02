#include <stdio.h>

int main() {
    int values[] = {21,22,22,10,27,28,29,30,31,32,33,34,35,36,37,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46};
    int maxvalue = 52 + 1;
    int numvalues = 30;
    int i, j;
    // array of strings
    char graph[maxvalue][numvalues];
    // initialize the array
    for (i = 0; i < maxvalue; i++) {
        for (j = 0; j < numvalues; j++) {
            graph[i][j] = ' ';
        }
    }
    // fill the array
    for (i = 0; i < numvalues; i++) {
        for (j = 0; j < values[i]; j++) {
            graph[j][i] = '*';
        }
    }

    // print the array
    for (i = maxvalue - 1; i >= 0; i--) {
        for (j = 0; j < numvalues; j++) {
            if (graph[i][j] == ' ' && graph[i-1][j] == '*') {
                if (values[j] < 10) {
                    printf(" 0%d ", values[j]);
                } else {
                    printf(" %2d ", values[j]);
                }
            }else{
                if(graph[i][j] == ' '){
                    printf("    ");
                }else{
                    printf(" ** ");
                }
                
            }
        }
        printf("\n");
    }
    for (i = 0; i < numvalues; i++) {
        printf("----");
    }
    printf("\n");
    for (i = 0; i < numvalues; i++) {
        
        if (i < 9) {
            printf(" 0%d ", i+1);
        } else {
            printf(" %2d ", i+1);
        }
    }
}

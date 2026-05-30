#include <stdio.h>

int main() {
    int T, num;
    scanf("%d", &T);
    for (int i = 0; i < T; i++) {
        scanf("%d", &num);
        int flag = 0;
        for (int j = 2; j <= num / 2; j++) { // Missing check for num <= 1
            if (num % j == 0) {
                flag = 1;
                break;
            }
        }
        if (flag == 0)
            printf("Prime\n");
        else
            printf("Not Prime\n");
    }
    return 0;
}

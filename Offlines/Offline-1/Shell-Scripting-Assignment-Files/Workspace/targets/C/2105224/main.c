#include <stdio.h>

// Incorrect: Skips checking divisibility by i properly
int isPrime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i < n / 2; i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

int main() {
    int T, num;
    scanf("%d", &T);
    for (int i = 0; i < T; i++) {
        scanf("%d", &num);
        if (isPrime(num))
            printf("Prime\n");
        else
            printf("Not Prime\n");
    }
    return 0;
}

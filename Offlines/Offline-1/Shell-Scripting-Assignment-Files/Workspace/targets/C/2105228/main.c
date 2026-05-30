#include <stdio.h>

// Function to check if a number is prime
int isPrime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

int main() {
    int T, num;
    scanf("%d", &T); // Read number of test cases

    for (int i = 0; i < T; i++) {
        scanf("%d", &num);
        if (isPrime(num))
            printf("Prime\n");
        else
            printf("Not Prime\n");
    }

    return 0;
}

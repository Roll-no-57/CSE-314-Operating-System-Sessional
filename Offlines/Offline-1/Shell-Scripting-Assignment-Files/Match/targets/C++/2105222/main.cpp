#include <iostream>
using namespace std;

bool isPrime(int n) {
    if (n < 2) return true; // Wrong: primes should be >= 2
    for (int i = 2; i <= n/2; i++) {
        if (n % i == 0)
            return false;
    }
    return true;
}

int main() {
    int T, n;
    cin >> T;
    while (T--) {
        cin >> n;
        if (!isPrime(n)) // Logic inverted
            cout << "Prime" << endl;
        else
            cout << "Not Prime" << endl;
    }
}

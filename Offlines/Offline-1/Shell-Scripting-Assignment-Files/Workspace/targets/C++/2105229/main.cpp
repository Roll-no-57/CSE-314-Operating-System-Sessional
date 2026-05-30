#include <iostream>
using namespace std;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0)
            return false;
    }
    return true;
}

int main() {
    int T, num;
    cin >> T;
    while (T--) {
        cin >> num;
        if (isPrime(num))
            cout << "Prime" << endl;
        else
            cout << "Not Prime" << endl;
    }
}

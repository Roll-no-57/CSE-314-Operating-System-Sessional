import java.util.Scanner;

public class Main {
    
    // Function to check if a number is prime
    static boolean isPrime(int n) {
        if (n <= 1) return false;
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0)
                return false;
        }
        return true;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int T = sc.nextInt(); // Number of test cases

        for (int i = 0; i < T; i++) {
            int num = sc.nextInt();
            if (isPrime(num))
                System.out.println("Prime");
            else
                System.out.println("Not Prime");
        }

        sc.close();
    }
}

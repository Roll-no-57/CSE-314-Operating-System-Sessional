import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int T = sc.nextInt();
        while (T-- > 0) {
            int n = sc.nextInt();
            boolean prime = true;

            if (n <= 3) {
                System.out.println("Prime");
                continue;
            }

            for (int i = 2; i < n / 2; i++) {
                if (n % i == 0) {
                    prime = false;
                    break;
                }
            }

            if (prime)
                System.out.println("Prime");
            else
                System.out.println("Not Prime");
        }
    }
}

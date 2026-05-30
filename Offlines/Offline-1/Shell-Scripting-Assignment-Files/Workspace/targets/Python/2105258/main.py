T = int(input())
for _ in range(T):
    n = int(input())
    if n <= 1:
        print("Not Prime")
    elif n % 2 == 0:
        print("Not Prime")  # Fails: 2 is prime
    else:
        is_prime = True
        for i in range(3, int(n**0.5)+1, 2):
            if n % i == 0:
                is_prime = False
                break
        print("Prime" if is_prime else "Not Prime")

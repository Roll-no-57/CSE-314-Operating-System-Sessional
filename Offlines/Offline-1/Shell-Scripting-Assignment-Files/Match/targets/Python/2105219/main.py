# Function to check if a number is prime
def is_prime(n):
    if n <= 1:
        return False
    for i in range(2, int(n**0.5)+1):
        if n % i == 0:
            return False
    return True

# Read number of test cases
T = int(input())

for _ in range(T):
    num = int(input())
    if is_prime(num):
        print("Prime")
    else:
        print("Not Prime")

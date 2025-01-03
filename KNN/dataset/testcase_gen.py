import random

def save_large_test_case(filename, n, lower_bound=-1000.0, upper_bound=1000.0):
    with open(filename, 'w') as f:
        for _ in range(int(n)):
            value1 = round(random.uniform(lower_bound, upper_bound), 2)
            value2 = round(random.uniform(lower_bound, upper_bound), 2)
            f.write(f"{value1},{value2}\n")


if __name__ == "__main__":
    n = int(1e6)
    save_large_test_case("dataset_1M.csv", n)
    print(f"Test case with {int(n)} values saved.")

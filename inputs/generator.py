import random
import os
import string
import sys

def generate_sequences(length, filename):
    sequence1 = []
    sequence2 = []
    
    for _ in range(length):
        sequence1.append(random.choice(string.ascii_lowercase))
        sequence2.append(random.choice(string.ascii_lowercase))

    sequence1_str = ",".join(sequence1)
    sequence2_str = ",".join(sequence2)
    
    with open(filename, "w") as file:
        file.write(sequence1_str + os.linesep)
        file.write(sequence2_str + os.linesep)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 generator <length> <output_file>")
        sys.exit(1)
    
    try:
        length = int(sys.argv[1])
        if length <= 0:
            raise ValueError("Length must be a positive integer.")
    except ValueError as e:
        print(f"Invalid length: {e}")
        sys.exit(1)

    filename = sys.argv[2]

    generate_sequences(length, filename)
    print(f"Generated sequences of length {length} in '{filename}'.")

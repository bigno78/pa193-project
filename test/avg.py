import sys

def main():
    lines = sys.stdin.readlines()
    s = 0
    for line in lines:
        s += int(line)
    print(f"AVG: {s/len(lines)}")
    
if __name__ == "__main__":
    main()
import sys

def main():
    with open(sys.argv[1]) as file:
        lines = file.readlines()
        s = 0
        for line in lines:
            s += int(line)
        print(s/len(lines))
    

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"USAGE: {sys.argv[0]} <score_file>", file=sys.stderr)
        sys.exit(1)

    main()
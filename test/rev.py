import sys
import json
import math
import os
from difflib import SequenceMatcher


def load_file(file):
    with open(file, "r") as f:
        return json.load(f)

def main():
    path = sys.argv[1]

    directory = os.fsencode(path)
    for file in os.listdir(directory):
        name = os.fsdecode(file)
        if name.endswith(".json"):
            actual = load_file(path + "/" + name)
            if "revisions" in actual and actual["revisions"]:
                print(path + "/" + name.rsplit('.', 1)[0] + '.txt')
                
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"USAGE: {sys.argv[0]} <expected_path>", file=sys.stderr)
        sys.exit(1)
    main()
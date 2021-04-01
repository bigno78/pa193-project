import sys
import json
import math
import os
from difflib import SequenceMatcher


def load_file(file):
    with open(file, "r") as f:
        return json.load(f)

def compare(actual, expected):
    if not "bibliography" in actual:
        if not "bibliography" in expected:
            return 20
        print(f"ERROR: Missing bibliography!", file=sys.stderr)
        raise RuntimeError()

    if not "bibliography" in expected:
        print(f"ERROR: Bibliography key present but shouldnt be!", file=sys.stderr)
        raise RuntimeError()

    actual = actual["bibliography"]
    expected = expected["bibliography"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        print(f"ERROR: No bibliography entries!", file=sys.stderr)
        raise RuntimeError()

    max_score = 2 * len(expected)
    score = 0

    for key in actual:
        if key not in expected:
            continue
        score += 1
        score += SequenceMatcher(None, actual[key], expected[key]).ratio()

    final_score = 20 * score / max_score 
    
    if final_score <= 10:
        print(f"ERROR: Very low score {final_score}!", file=sys.stderr)
        raise RuntimeError()
    return final_score

def main():
    actual_path = sys.argv[1]
    exppected_path = sys.argv[2]

    directory = os.fsencode(actual_path)
    for file in os.listdir(directory):
        name = os.fsdecode(file)
        if name.endswith(".json"):
            if os.path.isfile(exppected_path + "/" + name):
                actual = load_file(actual_path + "/" + name)
                expected = load_file(exppected_path + "/" + name)
                try:
                    score = compare(actual, expected)
                    #print(f"OK {score}")
                except RuntimeError:
                    print(name)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"USAGE: {sys.argv[0]} <actual_path> <expected_path>", file=sys.stderr)
        sys.exit(1)
    main()
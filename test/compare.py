import sys
import json
import math
import os
from difflib import SequenceMatcher


def load_file(file):
    with open(file, "r") as f:
        return json.load(f)

def check_title(actual, expected):
    if not "title" in actual.keys():
        if not "title" in expected:
            return 20
        return 0

    similarity = SequenceMatcher(None, actual["title"], expected["title"]).ratio()
    return 20 * similarity

def check_versions(actual, expected):
    if not "versions" in actual:
        if not "versions" in expected:
            return 20
        return 0

    actual = actual["versions"]
    expected = expected["versions"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        return 0

    max_score = len(expected) * 4
    score = 0

    for key in expected:
        if not key in actual:
            continue

        score += 1

        if len(expected[key]) == len(actual[key]):
            score += 1

        set_e = set(expected[key])
        set_a = set(actual[key])

        score += 2 * len(set_e.intersection(set_a)) / len(set_e)

    return 20 * score / max_score

def check_toc(actual, expected):
    if not "table_of_contents" in actual:
        if not "table_of_contents" in expected:
            return 20
        return 0

    if not "table_of_contents" in expected:
        return 0

    actual = list(filter(lambda x: len(x) == 3, actual["table_of_contents"]))
    expected = expected["table_of_contents"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        return 0

    max_score = 5 * len(expected)
    score = 0

    actual_numbers = list(map(lambda x: x[0], actual))
    expected_numbers = list(map(lambda x: x[0], expected))
    score += len(expected) * SequenceMatcher(None, actual_numbers, expected_numbers).ratio()

    actual_sections = list(map(lambda x: x[1], actual))
    expected_sections = list(map(lambda x: x[1], expected))
    score += len(expected) * SequenceMatcher(None, actual_sections, expected_sections).ratio()

    actual_pages = list(map(lambda x: x[2], actual))
    expected_pages = list(map(lambda x: x[2], expected))
    score += len(expected) * SequenceMatcher(None, actual_pages, expected_pages).ratio()

    for item in actual:
        if item in expected:
            score += 2

    return 20 * score / max_score

def check_revisions(actual, expected):
    if not "revisions" in actual:
        if not "revisions" in expected:
            return 20
        return 0

    if not "revisions" in expected:
        if not "revisions" in actual or len(actual["revisions"]) == 0:
            return 20
        return 0

    actual = list(filter(lambda x: set(x) == {"version", "date", "description"}, actual["revisions"]))
    expected = expected["revisions"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        return 0

    max_score = 5 * len(expected)
    score = 0

    actual_versions = list(map(lambda x: x["version"], actual))
    expected_versions = list(map(lambda x: x["version"], expected))
    score += len(expected) * SequenceMatcher(None, actual_versions, expected_versions).ratio()

    actual_dates = list(map(lambda x: x["date"], actual))
    expected_dates = list(map(lambda x: x["date"], expected))
    score += len(expected) * SequenceMatcher(None, actual_dates, expected_dates).ratio()

    actual_descriptions = list(map(lambda x: x["description"], actual))
    expected_descriptions = list(map(lambda x: x["description"], expected))
    score += len(expected) * SequenceMatcher(None, actual_descriptions, expected_descriptions).ratio()

    for item in actual:
        if item in expected:
            score += 2

    return 20 * score / max_score

def check_bibliography(actual, expected):
    if not "bibliography" in actual:
        if not "bibliography" in expected:
            return 20
        return 0
    
    if not "bibliography" in expected:
        return 0

    actual = actual["bibliography"]
    expected = expected["bibliography"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        return 0

    max_score = 2 * len(expected)
    score = 0

    for key in actual:
        if key not in expected:
            continue
        score += 1
        score += SequenceMatcher(None, actual[key], expected[key]).ratio()

    return 20 * score / max_score

check_map = {
    "--bibliography" : check_bibliography,
    "--title"        : check_title,
    "--revisions"    : check_revisions,
    "--versions"     : check_versions,
    "--toc"          : check_toc,
}

def main():
    actual_path = sys.argv[1]
    exppected_path = sys.argv[2]

    checks = []
    errors = False

    if len(sys.argv) == 3:
        checks = check_map.values()
    else:
        for i in range(3, len(sys.argv)):
            if sys.argv[i] == "--errors" or sys.argv[i] == "-e":
                errors = True
            elif sys.argv[i] not in check_map:
                print(f"unrecognized check: {sys.argv[i]}", file=sys.stderr)
                sys.exit(1)
            else:
                checks.append(check_map[sys.argv[i]])

    actual_directory = os.fsencode(actual_path)
    for file in os.listdir(actual_directory):
        filename = os.fsdecode(file)
        if filename.endswith(".json") and os.path.isfile(exppected_path + "/" + filename):
            actual = load_file(actual_path + "/" + filename)
            expected = load_file(exppected_path + "/" + filename)
            
            score = 0
            for check in checks:
                s = check(actual, expected)
                score += s

            if errors:
                if score < len(checks)*20:
                    print(f"{filename}: {score}")
            else:
                print(math.ceil(score))


if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(f"USAGE: {sys.argv[0]} <actual_path> <expected_path> <checks_to_perform>", file=sys.stderr)
        sys.exit(1)
    main()

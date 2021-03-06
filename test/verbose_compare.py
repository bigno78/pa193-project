#!/bin/env python3
import sys
import json
import math
from difflib import SequenceMatcher

def load_file(file):
    with open(file, "r") as f:
        return json.load(f)

def check_title(actual, expected):
    if not "title" in actual.keys():
        if not "title" in expected:
            return 20
        print("ERROR: Title key missing!", file=sys.stderr)
        return 0

    similarity = SequenceMatcher(None, actual["title"], expected["title"]).ratio()
    return 20 * similarity

def check_versions(actual, expected):
    if not "versions" in actual:
        if not "versions" in expected:
            return 20
        print("ERROR: Versions key missing!", file=sys.stderr)
        return 0

    actual = actual["versions"]
    expected = expected["versions"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        print("ERROR: Should be empty!", file=sys.stderr)
        return 0

    max_score = len(expected) * 4
    score = 0

    for key in expected:
        if not key in actual:
            print(f"ERROR: Missing key {key}!", file=sys.stderr)
            continue

        score += 1

        if len(expected[key]) == len(actual[key]):
            score += 1
        else:
            print(f"ERROR: {key} lengths differ!", file=sys.stderr)

        set_e = set(expected[key])
        set_a = set(actual[key])

        for ver in actual[key]:
            if ver not in expected[key]:
                print(f"ERROR: Extra version {ver}!", file=sys.stderr)

        for ver in expected[key]:
            if ver not in actual[key]:
                print(f"ERROR: Missing version {ver}!", file=sys.stderr)

        s = 2 * len(set_e.intersection(set_a)) / len(set_e)
        score += s
        if s < 2:
            print(f"ERROR: Low socre on {key}!", file=sys.stderr)

    print(f"{score}/{max_score}")

    return 20 * score / max_score

def check_toc(actual, expected):
    if not "table_of_contents" in actual:
        if not "table_of_contents" in expected:
            return 20
        print("ERROR: Toc key missing!", file=sys.stderr)
        return 0

    if not "table_of_contents" in expected:
        if not "table_of_contents" in actual:
            return 0
        print("ERROR: Toc key present but shud not be!", file=sys.stderr)
        return 0

    actual = list(filter(lambda x: len(x) == 3, actual["table_of_contents"]))
    expected = expected["table_of_contents"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        print("ERROR: Different lengths!", file=sys.stderr)
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
        else:
            print(f"ERROR: Extra item {item}!", file=sys.stderr)

    for item in expected:
        if item in actual:
            pass
        else:
            print(f"ERROR: Missing item {item}!", file=sys.stderr)

    return 20 * score / max_score

def check_revisions(actual, expected):
    if not "revisions" in actual:
        if not "revisions" in expected:
            return 20
        print("ERROR: Revisions key present but should't be!", file=sys.stderr)
        return 0

    actual = list(filter(lambda x: set(x) == {"version", "date", "description"}, actual["revisions"]))
    expected = expected["revisions"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        print("ERROR: Length should be 0!", file=sys.stderr)
        return 0

    max_score = 5 * len(expected)
    score = 0

    actual_versions = list(map(lambda x: x["version"], actual))
    expected_versions = list(map(lambda x: x["version"], expected))
    s = len(expected) * SequenceMatcher(None, actual_versions, expected_versions).ratio()
    if s < 1.5:
        print(f"ERROR: Low scoring version: {s}", file=sys.stderr)
    score += s

    actual_dates = list(map(lambda x: x["date"], actual))
    expected_dates = list(map(lambda x: x["date"], expected))
    s = len(expected) * SequenceMatcher(None, actual_dates, expected_dates).ratio()
    if s < 1.5:
        print(f"ERROR: Low scoring date: {s}", file=sys.stderr)
    score += s

    actual_descriptions = list(map(lambda x: x["description"], actual))
    expected_descriptions = list(map(lambda x: x["description"], expected))
    s = len(expected) * SequenceMatcher(None, actual_descriptions, expected_descriptions).ratio()
    if s < 1.5:
        print(f"ERROR: Low scoring description: {s}", file=sys.stderr)
    score += s

    for item in actual:
        if item in expected:
            score += 2
        else:
            print(f"ERROR: Extra item {item}!", file=sys.stderr)
    
    for item in expected:
        if item not in actual:
            print(f"ERROR: Missing item {item}!", file=sys.stderr)

    return 20 * score / max_score

def check_bibliography(actual, expected):
    if not "bibliography" in actual:
        if not "bibliography" in expected:
            return 20
        print("ERROR: Bibliography key present but should't be!", file=sys.stderr)
        return 0
    else:
        if not "bibliography" in expected:
            print("ERROR: Bibliography key missing!", file=sys.stderr)
            return 0

    actual = actual["bibliography"]
    expected = expected["bibliography"]

    if len(expected) == 0:
        if len(actual) == 0:
            return 20
        print("ERROR: Empty bibliography!", file=sys.stderr)
        return 0

    max_score = 2 * len(actual)
    score = 0

    for key in actual:
        if key not in expected:
            print(f"ERROR: Missing key '{key}'!", file=sys.stderr)
            continue
        score += 1
        similarity = SequenceMatcher(None, actual[key], expected[key]).ratio()
        score += similarity
        if similarity < 1:
            print(f"ERROR: Key '{key}' is very different from expected!", file=sys.stderr)
            #print(f"    - Reference: {actual[key]}", file=sys.stderr)
            #print(f"    - Parser: {expected[key]}", file=sys.stderr)
    
    for key in expected:
        if key not in actual:
            print(f"ERROR: Extra key '{key}'!", file=sys.stderr)
    
    return 20 * score / max_score


def main():
    actual = load_file(sys.argv[1])
    expected = load_file(sys.argv[2])

    checks = (check_toc,)
    points = 0
    for check in checks:
        points += check(actual, expected)

    print(math.ceil(points))

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"USAGE: {sys.argv[0]} <output_json> <reference_json>", file=sys.stderr)
        sys.exit(1)

    main()

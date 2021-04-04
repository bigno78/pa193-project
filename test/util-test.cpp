#include "catch.hpp"

#include "../src/utils.hpp"

TEST_CASE("Test count_words") {

    SECTION("Leading whitespace") {
        REQUIRE( count_words(" one two") == 2 );
        REQUIRE( count_words("\tone two") == 2 );
        REQUIRE( count_words("\n \t one two") == 2 );
        REQUIRE( count_words("    one two") == 2 );
    }

    SECTION("Trailing whitespace") {
        REQUIRE( count_words("one two ") == 2 );
        REQUIRE( count_words("one two\t") == 2 );
        REQUIRE( count_words("one two \t \n") == 2 );
        REQUIRE( count_words("one two    ") == 2 );
    }

    SECTION("Long spaces between words") {
        REQUIRE( count_words("one        two") == 2 );
        REQUIRE( count_words("one\t\t\t\t\ttwo") == 2 );
        REQUIRE( count_words("one\t\n   \t \t\t\n\n\n\t wo") == 2 );
        REQUIRE( count_words("one  two") == 2 );
    }

    SECTION("One word") {
        REQUIRE( count_words("one") == 1 );
    }
    
    SECTION("No words") {
        REQUIRE( count_words("       ") == 0 );
        REQUIRE( count_words("") == 0 );
    }

    SECTION("One letter words") {
        REQUIRE( count_words(" a b   c") == 3 );
    }

    SECTION("Weird characters") {
        REQUIRE( count_words("hh%^&5 :' ) [hmm] \\") == 5 );
    }
}

TEST_CASE("Test append_line") {
    
    SECTION("Append line with space") {
        std::string line = "this is   a line";
        std::string apendee = "we gonna append this";
        std::string expected = "this is   a line we gonna append this";
        append_line(line, apendee);
        CHECK( line == expected );
    }

    SECTION("Append line with dash") {
        std::string line = "this is   a line -";
        std::string apendee = "we gonna append this";
        std::string expected = "this is   a line -we gonna append this";
        append_line(line, apendee);
        CHECK( line == expected );
    }

    SECTION("Ending with strange characters") {
        std::string apendee = "app this";

        SECTION("") {
            std::string line = "a line$";
            std::string expected = line + " " + apendee;
            append_line(line, apendee);
            CHECK( line == expected );
        }

        SECTION("") {
            std::string line = "a line5";
            std::string expected = line + " " + apendee;
            append_line(line, apendee);
            CHECK( line == expected );
        }

        SECTION("") {
            std::string line = "a line\\";
            std::string expected = line + " " + apendee;
            append_line(line, apendee);
            CHECK( line == expected );
        }

        SECTION("") {
            std::string line = "a line(";
            std::string expected = line + " " + apendee;
            append_line(line, apendee);
            CHECK( line == expected );
        }
    }
}

TEST_CASE("Test join_columns") {

    SECTION("Normal cases") {
        CHECK( join_columns("    this     is a line  with    several columns") == "this is a line with several columns" );
    }

    SECTION("Single letter column") {
        CHECK( join_columns(" something    a  that this") == "something a that this" );
    }

    SECTION("Line without columns") {
        CHECK( join_columns("there are no columns") == "there are no columns" );
    }

    SECTION("Leading and trailing whitespace") {
        CHECK( join_columns(" there is whitespace ") == "there is whitespace" );
        CHECK( join_columns("   there is whitespace     ") == "there is whitespace" );
        CHECK( join_columns(" there    ") == "there" );
    }

}

TEST_CASE("Test trim") {
    SECTION("None") {
        std::string s = "string that should not be trimmed"; trim(s);
        CHECK( s == "string that should not be trimmed");
        s = "another one"; trim(s);
        CHECK(s == "another one");
        s = "spaces   in \n\t  the \n\n\t  middle"; trim(s);
        CHECK(s == "spaces   in \n\t  the \n\n\t  middle");
    }
    SECTION("Leading") {
        std::string s = "\n\n\t  \t leading spaces"; trim(s);
        CHECK(s == "leading spaces");
        s = "\n\n\n  \t\t\n  spaces \n \t middle"; trim(s);
        CHECK(s == "spaces \n \t middle");
    }
    SECTION("Trailing") {
        std::string s = "word word word   \n \t"; trim(s);
        CHECK(s == "word word word");
        s = "middle\n\n\nspace\t\t\t"; trim(s);
        CHECK(s == "middle\n\n\nspace");
    }
    SECTION("Leading and trailing") {
        std::string s = "             one\t \t \n  "; trim(s);
        CHECK(s == "one");
        s = "\n \t \n multiple words "; trim(s);
        CHECK(s == "multiple words");
        s = "  \n spaces \n\n\t  in    the \n\n\n\nmiddle \t\t\t \t"; trim(s);
        CHECK(s == "spaces \n\n\t  in    the \n\n\n\nmiddle");
    }
}

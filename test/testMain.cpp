/////////////////////////////////////////////////////////////////////////////
// Name:        testMain.cpp
// Purpose:     Unit tests for char_view.
// Author:      Piotr Likus
// Created:     07/12/2014
// Last change: 27/12/2014
// Version:     0.1
// License:     BSD
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <exception>
#include <algorithm>
#include <cctype>
#include <string>

#include "char_view.h"

using namespace std;
using namespace sbt;

namespace std {

    // to_string is not implemented in tested version of GCC/MinGW
    // a little too long story about it: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52015
    template <typename T>
    std::string to_string(T value) {
      ostringstream out;
      out << value;
      return out.str();
    }
}
inline void Assert(bool assertion, const char *message = NULL)
{
    if( !assertion ) {
        if (message != NULL)
          throw runtime_error(std::string("assertion = [")+std::string(message)+"]");
        else
          throw runtime_error("assertion failed");
    }
}

inline void Assert(bool assertion, const std::string &message)
{
    Assert(assertion, message.c_str());
}

template <typename Func>
inline void AssertThrows(Func assertion, const char *message = NULL)
{
    bool throwFound = false;
    try {
        assertion();
    }
    catch(...) {
        throwFound = true;
    }

    if( !throwFound ) {
        if (message != NULL)
          throw runtime_error(std::string("assertion.throws = [")+std::string(message)+"]");
        else
          throw runtime_error("assertion.throws failed");
    }
}

template <typename ObjectType, typename Func>
void AssertThrowsMethod(ObjectType obj, Func func, const char *message = NULL)
{
    bool throwFound = false;
    try {
      (obj.*func)();
    }
    catch(...) {
        throwFound = true;
    }

    if( !throwFound ) {
        if (message != NULL)
          throw runtime_error(std::string("assertion.throws = [")+std::string(message)+"]");
        else
          throw runtime_error("assertion.throws failed");
    }
}

template<typename Func>
void testFunc(const char *name, Func f, bool &failed)
{
    try {
        f();
        cout << "Test [" << std::string(name) << "] " << "succeeded\n";
    }
    catch(exception &e) {
        cout << "Test [" << std::string(name) << "] failed!, error: " << e.what() << "\n";
        failed = true;
    }
}

#ifdef CV_STRING_SUFFIX
    bool TestSuffix()
    {
        constexpr auto s1 = "An orange tree"_cv;
        Assert(s1.size() > 0);
        Assert("std::"_cv.size() == 5, "'std::'_cv.size() == 5");

        constexpr auto sw1 = L"Test wide string"_cv;
        Assert(sw1.size() > 0);
        return true;
    }
#endif

    bool TestConstruct1()
    {
        // this will not compile - no view for NULL string
        // constexpr char_view s2;
        constexpr auto s1 = char_view("Test string 1", 14);
        Assert(s1.size() > 0, "size > 0");
        Assert(sizeof(s1) == sizeof(size_t)+sizeof(char *), std::string("size of char_view = ")+to_string((sizeof(s1))));
        return true;
    }

    bool TestConstruct2()
    {
        constexpr auto s1 = char_view("Test string 2");
        Assert(s1.size() > 0, "size > 0");
        return true;
    }

    bool TestConstruct3()
    {
        constexpr char_view s1("Test string 2a");
        Assert(s1.size() > 0, "size > 0");
        return true;
    }

    bool TestCopy2String()
    {
        constexpr auto s1 = char_view("Test string 3");
        std::string s2 = s1;
        Assert(s2.size() > 0, "size > 0");
        return true;
    }

    bool TestCopy2ConstStr()
    {
        constexpr auto s1 = char_view("Test string 4");
        auto s2 = s1;
        Assert(s2.size() == s1.size(), "s2.size == s1.size");
        Assert(s2.size() > 0, "size > 0");
        return true;
    }

    bool TestStream()
    {
        constexpr char_view s1("Test string 4");
        std::ostringstream	out;
        out << s1;
        Assert(s1.size() > 0, "size > 0");

        out.seekp(0, ios::end);
        stringstream::pos_type offset = out.tellp();
        Assert(s1.size() == offset, "size == size of stream");

        return true;
    }

    bool TestSubstr() {
        constexpr char_view s1("Test string 5");
        constexpr char_view s2 = s1.substr(0, 5);
        Assert(s2.size() == 5);
        constexpr char_view s3 = s1.substr(12, 5);
        Assert(s3.size() == 1);
        return true;
    }

    bool TestEmpty() {
        constexpr char_view s1("Test string 6");
        constexpr char_view s2("");
        Assert(!s1.empty());
        Assert(s2.empty());
        return true;
    }

    bool TestArray()
    {
        constexpr char_view s1("Ab");
        Assert(s1[0] == 'A');
        Assert(s1[1] == 'b');

        return true;
    }

    struct TestThrowsStrAt {
        TestThrowsStrAt(const char_view &str): m_str(str) {}
        bool operator()() {
          return m_str.at(0) == m_str.at(1);
        }
    private:
        char_view m_str;
    };

    bool TestAt()
    {
        constexpr char_view s1("Ab");
        Assert(s1.at(0) == 'A');
        Assert(s1.at(1) == 'b');

#ifdef CS_RANGE_ERR_THROWS
        constexpr char_view s2("");
        TestThrowsStrAt ta(s2);
        AssertThrows(ta, "throws at");
#endif

        return true;
    }

    bool TestFrontBack() {
        constexpr char_view s1("Abcdefg");
        Assert(s1.front() == 'A', "front == [A]");
        Assert(s1.back() == 'g', "back == [g]");

        Assert(static_cast<std::string>(s1.front(3)) == "Abc", "string(front(3)) == [Abc]");
        Assert(s1.front(3) == "Abc", "front(3) == [Abc]");
        Assert(s1.back(3) == "efg", "back(3) == [efg]");

        Assert(s1.front(0) == "", "front(0) == []");
        Assert(s1.back(0) == "", "back() == []");

        std::string str = {"Abc"};
        Assert(s1.front(3) == str, "front(3) == std::string");
        Assert(str == "Abc", "std::string == [Abc]");

#ifdef CS_RANGE_ERR_THROWS
        constexpr char_view s2("");
        //AssertThrowsMethod(s2, &char_view::front, "throws front");
        //AssertThrowsMethod(s2, &char_view::back, "throws back");
        // for overloads more complicated syntax is required
        AssertThrowsMethod(s2, (char (char_view::*)() const)&char_view::front, "throws front");
        AssertThrowsMethod(s2, (char (char_view::*)() const)&char_view::back, "throws back");
#endif
        return true;
    }

    bool TestHashCode() {
        constexpr char_view s1("Abcdefg");
        Assert(s1.hash_code() == char_view("Abcdefg").hash_code());
        Assert(s1.hash_code() != char_view("aAbcdefg").hash_code());
        Assert(s1.hash_code() != char_view("").hash_code());

        return true;
    }

    bool TestHashCodeSwitch() {
        std::string s1("abc");

#ifdef CV_DEF_RECURSIVE
        switch (char_view(s1.c_str()).hash_code()) {
            case char_view("bde").hash_code():
                Assert(false, "Switch / hash_code");
                break;
            case char_view("").hash_code():
                Assert(false, "Switch / hash_code 2");
                break;
            case char_view("abc").hash_code():
                break;
            default:
                Assert(false, "Switch / default");
                return false;
        }
#else
        size_t code0 = char_view(s1.c_str()).hash_code();
        size_t code1 = char_view("bde").hash_code();
        size_t code2 = char_view("").hash_code();
        size_t code3 = char_view("abc").hash_code();

        if (code0 == code1) {
           Assert(false, "Switch / hash_code");
        } else if (code0 == code2) {
           Assert(false, "Switch / hash_code 2");
        } else if (code0 == code3) {
        } else {
           Assert(false, "Switch / default");
           return false;
        }
 #endif
        return true;
    }

    bool CompareHash(const char *text) {
        return (details::str_hash(text) == details::no_inline<char>::str_hash_loop(text));
    }

    void AssertCompareHash(const char *text) {
        Assert(CompareHash(text), text);
    }

    bool TestHashComp() {
        AssertCompareHash("12345");
        AssertCompareHash("a");
        AssertCompareHash("");
        AssertCompareHash("AZ10.:");

        return true;
    }

    bool TestStartsWith() {
        constexpr char_view s1("Abcdefg");
        Assert(s1.starts_with("Abcd"), "Abcdefg starts with Abcd");
        Assert(s1.starts_with("A"), "Abcdefg starts with A");
        Assert(s1.starts_with(""), "Abcdefg starts with []");
        Assert(s1.starts_with("Abcdefg"), "Abcdefg starts with Abcdefg");
        Assert(!s1.starts_with("Abcdefgi"), "Abcdefg starts with Abcdefg");
        Assert(!s1.starts_with("bbb"), "Abcdefg starts with bbb");
        Assert(!s1.starts_with("abc"), "Abcdefg starts with abc");
        Assert(!s1.starts_with("bc"), "Abcdefg starts with bc");

        constexpr char_view s2("A");
        Assert(s2.starts_with("A"), "A starts with A");
        Assert(!s2.starts_with("Abcd"), "A starts with Abcd");
        Assert(s2.starts_with(""), "A starts with []");
        return true;
    }

    bool TestEndsWith() {
        constexpr char_view s1("Abcdefg");
        Assert(s1.ends_with("Abcdefg"));
        Assert(s1.ends_with("defg"));
        Assert(s1.ends_with("fg"));
        Assert(s1.ends_with("g"));
        Assert(s1.ends_with(""));
        Assert(!s1.ends_with("bAbcdefg"));
        Assert(!s1.ends_with("Abcdefgb"));
        Assert(!s1.ends_with("aefg"));
        Assert(!s1.ends_with("bbb"));
        Assert(!s1.ends_with("abc"));
        Assert(!s1.ends_with("Abc"));
        Assert(!s1.ends_with("def"));

        constexpr char_view s2("A");
        Assert(s2.ends_with("A"), "A ends with A");
        Assert(s2.ends_with(""), "A ends with []");
        Assert(!s2.ends_with("bA"), "A ends with bA");

        constexpr char_view s3("aabbccaa");
        Assert(s3.ends_with("a"), "aabbccaa ends with a");
        Assert(s3.ends_with("aa"), "aabbccaa ends with aa");
        Assert(!s3.ends_with("ca"), "aabbccaa ends with ca");
        Assert(!s3.ends_with("cc"), "aabbccaa ends with cc");
        return true;
    }

    bool TestEquals() {
        constexpr char_view s1("Abcdefg");

        Assert(s1.size() == 7, "size == 7");
        Assert(s1.equals("Abcdefgij", 7), "equals [Abcdefgij, 7]");
        Assert(s1.equals("Abcdefg", 7), "equals [Abcdefg, 7]");
        Assert(!s1.equals("Abcdefg", 6), "equals [Abcdefg, 6]");
        Assert(s1.equals("Abcdefg"), "equals [Abcdefg]");
        Assert(!s1.equals("aAbcdefg"), "equals [aAbcdefg]");
        Assert(!s1.equals("Abcdefga"), "equals [Abcdefga]");
        Assert(!s1.equals("Abcdef"), "equals [Abcdef]");
        Assert(!s1.equals(""), "equals []");

        constexpr char_view s2("A");
        Assert(s2.equals("Aaa", 1), "equals [Aaa, 1]");
        Assert(s2.equals("A", 1), "equals [A, 1]");
        Assert(s2.equals("A"), "equals [A]");
        Assert(!s2.equals("Aa", 2), "equals [Aa, 2]");
        Assert(!s2.equals("Aa"), "equals [Aa]");
        Assert(!s2.equals("aA", 2), "equals [aA, 2]");
        Assert(!s2.equals("aA"), "equals [aA]");
        Assert(!s2.equals("a, 1"), "equals [a, 1]");
        Assert(!s2.equals("a"), "equals [a]");
        Assert(!s2.equals("", 0), "equals [,0]");
        Assert(!s2.equals(""), "equals []");

        constexpr char_view s3("");
        Assert(s3.equals("", 0), "equals [, 0]");
        Assert(!s3.equals("B", 1), "equals [B, 1]");
        Assert(!s3.equals("B"), "equals [B]");
        Assert(!s3.equals("Bbb", 3), "equals [Bbb, 3]");
        Assert(!s3.equals("Bbb"), "equals [Bbb]");
        return true;
    }

    bool TestCompare() {
        constexpr char_view s1("Abcdefg");
        std::string s4("Abcdefg");
        int compareRes;

        Assert(s1.size() == 7, "size == 7");
        Assert(s1.compare("Abcdefgij", 7) == 0, "compare [Abcdefgij, 7]");
        Assert(s1.compare("Abcdefg", 7) == 0, "compare [Abcdefg, 7]");
        Assert(s4.compare(0, 7, "Abcdefg", 6) > 0, "compare-std [Abcdefg, 6]");
        Assert(s1.compare("Abcdefg", 6) > 0, "compare [Abcdefg, 6]");
        compareRes = s1.compare("Abcdefg");
        Assert(compareRes == 0,
               std::string("compare [Abcdefg] (=") +
               std::to_string(compareRes)+
               ")");
        Assert(s1.compare("Abcdefg") == 0, "compare [Abcdefg]");

        int compStd = s4.compare("aAbcdefg");
        Assert(compStd < 0,
               std::string("compare-std [aAbcdefg] (=") +
               std::to_string(compStd)+
               ")");

        Assert(s1.size() == 7, "size() == 7");
        compareRes = s1.compare("aAbcdefg");
        Assert(compareRes < 0,
               std::string("compare [aAbcdefg] (=") +
               std::to_string(compareRes)+
               ")");
        Assert(s1.compare("Abcdefga") < 0, "compare [Abcdefga]");
        Assert(s1.compare("Abcdef") > 0, "compare [Abcdef]");
        Assert(s4.compare("A") > 0, "Abcdefg compare-std [A]");
        Assert(s1.compare("A") > 0, "Abcdefg compare [A]");
        Assert(s1.compare("") > 0, "compare []");

        constexpr char_view s2("A");
        Assert(s2.compare("Aaa", 1) == 0, "compare [Aaa, 1]");
        Assert(s2.compare("A", 1) == 0, "compare [A, 1]");
        Assert(s2.compare("A") == 0, "compare [A]");
        Assert(s2.compare("Aa", 2) < 0, "compare [Aa, 2]");
        Assert(s2.compare("Aa") < 0, "compare [Aa]");
        Assert(s2.compare("aA", 2) < 0, "compare [aA, 2]");
        Assert(s2.compare("aA") < 0, "compare [aA]");
        Assert(s2.compare("a, 1") < 0, "compare [a, 1]");
        Assert(s2.compare("a") < 0, "compare [a]");
        Assert(s2.compare("", 0) > 0, "compare [,0]");
        Assert(s2.compare("") > 0, "compare []");

        constexpr char_view s3("");
        Assert(s3.compare("", 0) == 0, "compare [, 0]");
        Assert(s3.compare("B", 1) < 0, "compare [B, 1]");
        Assert(s3.compare("B") < 0, "compare [B]");
        Assert(s3.compare("Bbb", 3) < 0, "compare [Bbb, 3]");
        Assert(s3.compare("Bbb") < 0, "compare [Bbb]");
        return true;
    }

    constexpr bool TestCharPtrEqual(const char_view &s1) {
        return (s1 == "Abcdefg");
    }

    bool TestCompOperators() {
        constexpr char_view s1("Abcdefg");
        std::string s1_std("Abcdefg");
        std::string s2("Abc");
        char b1[] = "Abcdefg";

        Assert(s1_std > s2, "s1_std > s2");
        Assert(s1 > s2, "s1 > s2");
        Assert(s1 >= s2, "s1 >= s2");
        Assert(s2 < s1, "s2 < s1");
        Assert(s2 <= s1, "s2 <= s1");
        Assert(s1 != s2, "s1 != s2");
        Assert(s2 != s1, "s2 != s1");
        Assert(s1 == b1, "s1 == b1");
        Assert(s1 <= b1, "s1 <= b1");
        Assert(s1 >= b1, "s1 >= b1");
        Assert(TestCharPtrEqual(s1), "TestCharPtrEqual");

        return true;
    }

    bool TestContains() {
        constexpr char_view s1("Abcdefg");

        Assert(s1.contains("", 0), "contains [, 0]");
        Assert(s1.contains(""), "contains []");
        Assert(s1.contains("A", 1), "contains [A, 1]");
        Assert(s1.contains("A"), "contains A");
        Assert(s1.contains("Ab", 2), "contains [Ab, 2]");
        Assert(s1.contains("Ab"), "contains Ab");
        Assert(s1.contains("Abcdefg", 7), "contains [Abcdefg, 7]");
        Assert(s1.contains("Abcdefg"), "contains Abcdefg");
        Assert(s1.contains("Abcdefgij", 7), "contains [Abcdefgij, 7]");
        Assert(!s1.contains("Abcdefgij"), "contains Abcdefgij");
        Assert(s1.contains("bcd"), "contains bcd");
        Assert(s1.contains("efg"), "contains efg");
        Assert(s1.contains("g"), "contains g");
        Assert(!s1.contains("z"), "contains z");
        return true;
    }

    bool TestFind() {
        constexpr char_view s1("Abcdefg");

        Assert(s1.find_cn("", 0) != char_view::npos, "find [, 0]");
        Assert(s1.find("") != char_view::npos, "find []");
        Assert(s1.find_cn("A", 1) != char_view::npos, "find [A, 1]");
        Assert(s1.find("A") != char_view::npos, "find A");
        Assert(s1.find_cn("Ab", 2)!= char_view::npos, "find [Ab, 2]");
        Assert(s1.find("Ab")!= char_view::npos, "find Ab");
        Assert(s1.find_cn("Abcdefg", 7)!= char_view::npos, "find [Abcdefg, 7]");
        Assert(s1.find("Abcdefg")!= char_view::npos, "find Abcdefg");
        Assert(s1.find_cn("Abcdefgij", 7)!= char_view::npos, "find [Abcdefgij, 7]");
        Assert(s1.find("Abcdefgij") == char_view::npos, "find Abcdefgij");
        Assert(s1.find("bcd") != char_view::npos, "find bcd");
        Assert(s1.find("efg") != char_view::npos, "find efg");
        Assert(s1.find("g") != char_view::npos, "find g");
        Assert(s1.find("z") == char_view::npos, "find z");
        return true;
    }

    bool TestRFind() {
        constexpr char_view s1("The sixth sick sheik's sixth sheep's sick.");

        Assert(s1.rfind_cn("", 0) != char_view::npos, "rfind [, 0]");
        Assert(s1.rfind("") != char_view::npos, "rfind []");
        Assert(s1.rfind_cn("T", 1) != char_view::npos, "rfind [T, 1]");
        Assert(s1.rfind("T") != char_view::npos, "rfind T");
        Assert(s1.rfind_cn("Th", 2)!= char_view::npos, "rfind [Th, 2]");
        Assert(s1.rfind("Th")!= char_view::npos, "rfind Th");

        Assert(s1.rfind("sixth") >4, "rfind sixth > 4");
        Assert(s1.rfind("i") != char_view::npos, "rfind i");
        Assert(s1.rfind("q") == char_view::npos, "rfind q");
        Assert(s1.rfind("sick.") != char_view::npos, "rfind [sick.]");
        return true;
    }

    bool TestFindFirstOf() {
        constexpr char_view s1("The sixth sick sheik's sixth sheep's sick.");

        Assert(s1.find_first_of_cn("", 0) == char_view::npos, "find_first_of [, 0]");
        Assert(s1.find_first_of("") == char_view::npos, "find_first_of []");

        Assert(s1.find_first_of("T") == 0, "find_first_of [T]");
        Assert(s1.find_first_of("s") == 4, "find_first_of [s]");
        Assert(s1.find_first_of(".") == s1.size() - 1, "find_first_of [.]");
        Assert(s1.find_first_of(",") == char_view::npos, "find_first_of [,]");

        Assert(s1.find_first_of("esp") == 2, "find_first_of [esp]");
        Assert(s1.find_first_of(".se") == 2, "find_first_of [.se]");
        Assert(s1.find_first_of("The") == 0, "find_first_of [The]");
        Assert(s1.find_first_of("ehT") == 0, "find_first_of [ehT]");

        return true;
    }

    bool TestFindFirstNotOf() {
        constexpr char_view s1("The sixth sick sheik's sixth sheep's sick.");

        Assert(s1.find_first_not_of_cn("", 0) != char_view::npos, "find_first_not_of [, 0]");
        Assert(s1.find_first_not_of("") != char_view::npos, "find_first_not_of []");

        Assert(s1.find_first_not_of("a") == 0, "find_first_not_of [a]");
        Assert(s1.find_first_not_of("The ") == 4, "find_first_not_of [The ]");
        Assert(s1.find_first_not_of("The sixth sick sheik's sixth sheep's sick") == s1.size() - 1, "find_first_not_of [-full-head-]");
        Assert(s1.find_first_not_of("The sixth sick sheik's sixth sheep's sick.") == char_view::npos, "find_first_not_of [-full-text-]");

        Assert(s1.find_first_not_of("esp") == 0, "find_first_not_of [esp]");
        Assert(s1.find_first_not_of(".se") == 0, "find_first_not_of [.se]");
        Assert(s1.find_first_not_of("The") == 3, "find_first_not_of [The]");
        Assert(s1.find_first_not_of("ehT") == 3, "find_first_not_of [ehT]");

        return true;
    }

    bool TestFindLastOf() {
        constexpr char_view s1("Controlling complexity is the essence of computer programming. - BR");

        Assert(s1.find_last_of_cn("", 0) == char_view::npos, "find_last_of [, 0]");
        Assert(s1.find_last_of("") == char_view::npos, "find_last_of []");

        // last character
        Assert(s1.find_last_of_cn("R", 1) == s1.size() - 1, "find_last_of [R, 1]");
        Assert(s1.find_last_of_cn("Riea", 1) == s1.size() - 1, "find_last_of [Riea, 1]");
        Assert(s1.find_last_of("R") == s1.size() - 1, "find_last_of [R]");

        // first character
        Assert(s1.find_last_of_cn("C", 1) == 0, "find_last_of [C, 1]");
        Assert(s1.find_last_of("C") == 0, "find_last_of [C]");

        // mid character, single
        Assert(s1.find_last_of_cn("B", 1) == s1.size() - 2, "find_last_of [B, 1]");
        Assert(s1.find_last_of("B") == s1.size() - 2, "find_last_of [B]");

        // mid character, multiple
        Assert(s1.find_last_of_cn("g", 1) == s1.size() - 7, "find_last_of [g, 1]");
        Assert(s1.find_last_of_cn("gBR", 1) == s1.size() - 7, "find_last_of [gBR, 1]");
        Assert(s1.find_last_of("g") == s1.size() - 7, "find_last_of [g]");

        Assert(s1.find_last_of_cn("i", 1) == s1.size() - 7 - 2, "find_last_of [i, 1]");
        Assert(s1.find_last_of("i") == s1.size() - 7 - 2, "find_last_of [i]");

        // set, search last character
        Assert(s1.find_last_of_cn("Rz!?", 4) == s1.size() - 1, "find_last_of [Rz!?, 4]");
        Assert(s1.find_last_of("Rz!?") == s1.size() - 1, "find_last_of [Rz!?]");

        // set, search first character
        Assert(s1.find_last_of_cn("z!?C", 4) == 0, "find_last_of [z!?C, 4]");
        Assert(s1.find_last_of("z!?C") == 0, "find_last_of [z!?C]");

        // set, search mid character, single
        Assert(s1.find_last_of_cn("Biae", 4) == s1.size() - 2, "find_last_of [Biae, 4]");
        Assert(s1.find_last_of("Biae") == s1.size() - 2, "find_last_of [Biae]");

        // set, search mid character, multiple
        Assert(s1.find_last_of_cn("iaeg", 4) == s1.size() - 7, "find_last_of [iaeg, 4]");
        Assert(s1.find_last_of("iaeg") == s1.size() - 7, "find_last_of [iaeg]");

        // non-existing set
        Assert(s1.find_last_of_cn("z!?q%$", 6) == char_view::npos, "find_last_of [z!?q%$, 6]");
        Assert(s1.find_last_of("z!?q%$") == char_view::npos, "find_last_of [z!?q%$]");
        return true;
    }

    bool TestFindLastNotOf() {
        constexpr char_view s1("Nine people can’t make a baby in a month. - FB");

        Assert(s1.find_last_not_of_cn("", 0) != char_view::npos, "find_last_not_of [, 0]");
        Assert(s1.find_last_not_of("") != char_view::npos, "find_last_not_of []");

        // last character
        Assert(s1.find_last_not_of_cn("Nine polca’tmkbyih.-F", 21) == s1.size() - 1, "find_last_not_of [Nine polca’tmkbyih.-F, 21]");
        Assert(s1.find_last_not_of("Nine polca’tmkbyih.-F") == s1.size() - 1, "find_last_not_of [R]");

        // first character
        Assert(s1.find_last_not_of_cn("ine polca’tmkbyih.-FB", 21) == 0, "find_last_not_of [ine polca’tmkbyih.-FB, 21]");
        Assert(s1.find_last_not_of("ine polca’tmkbyih.-FB") == 0, "find_last_not_of [ine polca’tmkbyih.-FB]");

        // mid character, single
        Assert(s1.find_last_not_of_cn("Nine polca’tmkbyih.-B", 21) == s1.size() - 2, "find_last_not_of [Nine polca’tmkbyih.-B, 21]");
        Assert(s1.find_last_not_of("Nine polca’tmkbyih.-B") == s1.size() - 2, "find_last_not_of [Nine polca’tmkbyih.-B]");

        // mid character, multiple
        Assert(s1.find_last_not_of_cn("Nine plca’tmkbyih.-FB", 21) == s1.size() - 10, "find_last_not_of [Nine plca’tmkbyih.-FB, 21]");
        Assert(s1.find_last_not_of("Nine plca’tmkbyih.-FB") == s1.size() - 10, "find_last_not_of [Nine plca’tmkbyih.-FB]");

        // set, search last character
        Assert(s1.find_last_not_of_cn("Rz!?", 4) == s1.size() - 1, "find_last_not_of [Rz!?, 4]");
        Assert(s1.find_last_not_of("Rz!?") == s1.size() - 1, "find_last_not_of [Rz!?]");

        // set, search mid character, single
        Assert(s1.find_last_not_of_cn("Biae", 4) == s1.size() - 2, "find_last_not_of [Biae, 4]");
        Assert(s1.find_last_not_of("Biae") == s1.size() - 2, "find_last_not_of [Biae]");

        // set, search mid character, multiple
        Assert(s1.find_last_not_of_cn("iaeg", 4) == s1.size() - 1, "find_last_not_of [iaeg, 4]");
        Assert(s1.find_last_not_of("iaeg") == s1.size() - 1, "find_last_not_of [iaeg]");

        // non-existing result
        Assert(s1.find_last_not_of_cn("Nine polca’tmkbyih.-FB", 22) == char_view::npos, "find_last_not_of [Nine polca’tmkbyih.-B, 22]");
        Assert(s1.find_last_not_of("Nine polca’tmkbyih.-FB") == char_view::npos, "find_last_not_of [Nine polca’tmkbyih.-FB]");
        return true;
    }

    bool TestAlgReverse() {
        constexpr char_view s1("Abcdefg");
        char destiny[] = "Abcdefg";

        std::reverse_copy(std::begin(s1), std::end(s1), std::begin(destiny));
        char_view s2(destiny, s1.size());
        Assert(s2 == "gfedcbA", "s2 == [gfedcbA]");

        return true;
    }

    bool TestAlgAllOf() {
        constexpr char_view s1("019217871789");

        Assert(all_of(s1.begin(), s1.end(), ::isdigit));
        return true;
    }

    // does not work with literals
    template<typename T>
    bool HasStdPrefix(const T& value) {
        return (value.substr(0, 5) == "std::");
    }

    // for literals
    bool HasStdPrefix(const char *value) {
        std::string s1(value);
        return (s1.substr(0, 5) == "std::");
    }

    // for cs literals & char_view (compile-time calculation enabled)
    constexpr bool HasStdPrefix(const char_view &value) {
        return value.starts_with("std::"_cv);
    }

    // for verification of handling std::string inside char_view
    bool HasStdPrefixStd(const char_view &value) {
        std::string s1("std::");
        Assert(value.substr(0,5).size() == 5, "value.substr(0,5).size() == 5");
        Assert(s1.size() == 5, "s1.size() == 5");
        return value.starts_with(s1);
    }

    bool TestGenerics1a() {
        constexpr char_view s1("std::list");
        Assert(HasStdPrefix(s1), "HasStdPrefix:1a");

        return true;
    }

    bool TestGenerics1b() {
        constexpr auto s1 = "std::list"_cv;
        Assert(HasStdPrefix(s1), "HasStdPrefix:1b");

        return true;
    }

    bool TestGenerics1c() {
        Assert(HasStdPrefix("std::list"_cv), "HasStdPrefix:1c");

        return true;
    }

    bool TestGenerics1d() {
        Assert(HasStdPrefixStd("std::list"_cv), "HasStdPrefix:1d");

        return true;
    }

    bool TestGenerics2() {
        std::string s2("std::queue");

        Assert(HasStdPrefix(s2), "HasStdPrefix:2");

        return true;
    }

    bool TestGenerics3a() {
        Assert(HasStdPrefix(std::string("std::list")), "HasStdPrefix:3a");

        return true;
    }

    bool TestGenerics3b() {
        Assert(HasStdPrefix("std::list"), "HasStdPrefix:3b");

        return true;
    }

    bool TestUtilToString() {
        constexpr char_view s1("Abcdefg");
        Assert(to_string(s1).length() == s1.size());

        return true;
    }

    bool TestTrim() {
        Assert(to_string("test1"_cv.trim()).length() == 5, "trim(non-white).len == 5");
        Assert(to_string("tes 1"_cv.trim()).length() == 5, "trim(mid-white).len == 5");
        Assert(to_string("  \t test1"_cv.trim()).length() == 5, "trim(left-white).len == 5");
        Assert(to_string("  \t tes 1"_cv.trim()).length() == 5, "trim(left-mid-white).len == 5");
        Assert(to_string("test1  \t "_cv.trim()).length() == 5, "trim(right-white).len == 5");
        Assert(to_string("t st1  \t "_cv.trim()).length() == 5, "trim(right-mid-white).len == 5");
        Assert(to_string("  \t test1  \t "_cv.trim()).length() == 5, "trim(both-white).len == 5");
        Assert(to_string("  \t t st1  \t "_cv.trim()).length() == 5, "trim(both-mid-white).len == 5");
        Assert(to_string("  \t\t "_cv.trim()).length() == 0, "trim(white).len == 0");
        Assert(to_string(" "_cv.trim()).length() == 0, "trim(single-white).len == 0");
        Assert(to_string(""_cv.trim()).length() == 0, "trim(empty-string).len == 0");

        constexpr auto sw1 = L"  \t wide1  \t "_cv;
        Assert(sw1.trim().size() == 5, "trim(wide-string).len == 5");

        constexpr auto sw2a = u"  \t u16t1  \t "_cv;
        Assert(sw2a.trim().size() == 5, "trim(u16t-string).len == 5");

        constexpr auto sw2b = U"  \t u32t1  \t "_cv;
        Assert(sw2b.trim().size() == 5, "trim(u32t-string).len == 5");

        return true;
    }

#define TEST_FUNC(a) testFunc(#a, Test##a, errorFound)

int main()
{
    bool errorFound = false;
#ifdef CV_STRING_SUFFIX
    TEST_FUNC(Suffix);
#endif
    TEST_FUNC(Construct1);
    TEST_FUNC(Construct2);
    TEST_FUNC(Construct3);
    TEST_FUNC(Copy2String);
    TEST_FUNC(Copy2ConstStr);
    TEST_FUNC(Stream);
    TEST_FUNC(Array);
    TEST_FUNC(At);
    TEST_FUNC(Substr);
    TEST_FUNC(Empty);
    TEST_FUNC(FrontBack);
    TEST_FUNC(HashCode);
    TEST_FUNC(HashCodeSwitch);
    TEST_FUNC(HashComp);
    TEST_FUNC(StartsWith);
    TEST_FUNC(EndsWith);
    TEST_FUNC(Equals);
    TEST_FUNC(Compare);
    TEST_FUNC(CompOperators);
    TEST_FUNC(Contains);
    TEST_FUNC(Find);
    TEST_FUNC(RFind);
    TEST_FUNC(FindFirstOf);
    TEST_FUNC(FindFirstNotOf);
    TEST_FUNC(FindLastOf);
    TEST_FUNC(FindLastNotOf);
    TEST_FUNC(AlgReverse);
    TEST_FUNC(AlgAllOf);

    TEST_FUNC(Generics1a);
    TEST_FUNC(Generics1b);
    TEST_FUNC(Generics1c);
    TEST_FUNC(Generics1d);
    TEST_FUNC(Generics2);
    TEST_FUNC(Generics3a);
    TEST_FUNC(Generics3b);

    TEST_FUNC(UtilToString);
    TEST_FUNC(Trim);

    if (errorFound) {
        cout << "Failures!\n";
        return EXIT_FAILURE;
    } else {
        cout << "Success!\n";
        return EXIT_SUCCESS;
    }
}

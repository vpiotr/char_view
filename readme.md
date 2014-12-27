Introduction
================
This is readme file for char_view - immutable string view class with compile-time processing 
with support for C++ string literals, std::strings and char buffers.
Library is currently implemented as header-only solution.

License
================
Software is published under BSD license, see license.txt for details.

Project home
================
https://github.com/vpiotr/char_view

Purpose
================
* for lightweight string literal handling in C++ way (with functions from std::string)
* to support compile-time properties (length, hash) and search methods (find, contains, equals, etc)
* to support strings in switch statement

Benefits
================
* allows compile-time string function calculations:

        using namespace sbt;
        
        // check if string has prefix "std::"
        constexpr bool HasStdPrefix(const char_view &value) {
            return value.starts_with("std::"_cv);
        }

        // assert calculated in compile time
        bool TestPrefix() {
            constexpr char_view s1("std::list");
            Assert(HasStdPrefix(s1), "HasStdPrefix");

            return true;
        }
    
* passing parameter as literal does not involve deep copying - has complexity = O(1) 

* allows using strings in switch statement:

        bool is_command_ok(const std::string &str) 
        {
            switch (char_view(str.c_str()).hash_code()) 
            {
                case char_view("clear").hash_code():
                case char_view("print").hash_code():
                case char_view("list").hash_code():
                    break;
                default:
                    return false;
            }
            return true;
        }
    
* implements special string literal for easy construction:
  
        constexpr auto s1 = "Test string"_cv;       
    

Other features    
======================
* size of any char_view object is 8 for GCC x32 (size_t + one pointer) 
* does not use dynamic memory allocation in all cases when std::string is not involved
* range checking & throwing on error is optional - implemented as policies (template parameters)
* works with various char types: char, wchar_t, char16_t, char32_t 
* can be used with literals, char arrays, zstrings & std::string
    
Supported environments
======================
* compiler: any compiler with C++11 support (constexpr required)
* tested with GCC 4.8.1 with C++11 support active (-std=c++11)
* test build project prepared in Code::Blocks 13

External dependencies
=====================
* just C++ standard library, including std::basic_string
    
Performance
========================
In cases where code can be calculated in compile time (literals processing) functions using
char_view will be much much faster than any other functions.
In best cases functions return just single value calculated during compilation.

Known issues
========================
* not all overloads of std::string are implemented for functions like find_zzz (substr should be enough)
* class does not check if provided "char *" pointer is literal or something else, so any char pointer can be provided and memory control must be done elsewhere
    
Other similar efforts
========================
* articles    
    * [Constants and Constant Expressions in C++11](http://www.codeproject.com/Articles/417719/Constants-and-Constant-Expressions-in-Cplusplus) - Mikhail Semenov, 2012 - article describing constexpr 
    * [Parsing strings at compile-time — Part I](https://akrzemi1.wordpress.com/2011/05/11/parsing-strings-at-compile-time-part-i/) - Andrzej Krzemieñski, 2011 - article describing how to employ constexpr for string processing 
* papers
    * [N4236, std::basic_string_literal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4236.html) - standard proposition by Michael Price, 2014
    * [N4121, std::string_literal<n>](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4121.pdf) - Andrew Tomazos, 2014
    * [c++14 proposal - string_view](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3762.html)
* experimental standard part(?), excluded from C++14    
    * [std::experimental::basic_string_view](http://en.cppreference.com/w/cpp/experimental/basic_string_view)
* open source solutions    
    * [constexpr_string](http://sourceforge.net/projects/constexprstr/) - compile time string processing library
    * [char_view<> at sourceforge](http://conststring.sourceforge.net/) - thread-safe, immutable string class by Maxim Yegorushkin 
    * [Boost string_literal](http://www.boost.org/doc/libs/1_55_0/boost/log/utility/string_literal.hpp) - literal wrapper by Andrey Semashev 
    * [Boost char_view](http://www.boost.org/doc/libs/1_34_0/libs/test/doc/tutorials/char_view.hpp) - class for testing(?) - by Gennadiy Rozental
    * [Chromium.org StringPiece](http://src.chromium.org/viewvc/chrome/trunk/src/base/strings/string_piece.h?view=markup)
    * [MNMLSTC Core string_view](http://mnmlstc.github.io/core/string.html)
    * [Bloomberg bslstl::StringRef](https://github.com/bloomberg/bde/blob/master/groups/bsl/bslstl/bslstl_stringref.h)

Contact information
====================
* [Google+](https://plus.google.com/114326541605789029332/) 
* [LinkedIn](http://pl.linkedin.com/pub/piotr-likus/2/307/7b9/)

Documentation
====================
To build documentation, use Code::Blocks 13 and command:

    DoxyBlocks \ Extract documentation

Output will be generated into: "./help/html/" directory in html format.

Release History
====================
See the [CHANGELOG](doc/CHANGELOG).

    
/////////////////////////////////////////////////////////////////////////////
// Name:        char_view.h
// Purpose:     Compile-time support for string literal algorithms for C++.
//              Includes functions from Java (hash_code, starts_with, ends_with).
// Author:      Piotr Likus
// Created:     07/12/2014
// Last change: 20/12/2014
// Version:     0.1
// License:     BSD
/////////////////////////////////////////////////////////////////////////////

#ifndef _CHAR_VIEW_H__
#define _CHAR_VIEW_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file char_view.h
///
/// String literal & char buffer compile-time view class.
/// Provides easier & direct string literal handling.
///
/// \mainpage char_view - manual
/// \author Piotr Likus
/// \par Introduction
/// char_view is a immutable string view class with support for compile-time processing.
/// Supports C++ string literals, std::strings and char buffers.
/// Library is currently implemented as header-only solution.
///
/// \par Usage examples
///
/// String literal declaration
/// \code{.cpp}
///    constexpr auto s1 = "Test string"_cv;
/// \endcode
///
/// Function which performs string scanning in compile time:
/// \code{.cpp}
///    // check if string has prefix "std::"
///    constexpr bool HasStdPrefix(const char_view &value) {
///        return value.starts_with("std::"_cv);
///    }
///
///    constexpr char_view s1("std::list");
///    assert(HasStdPrefix(s1));
/// \endcode
/// \par Reference
/// \link sbt::basic_char_view \endlink - main class for this library
/// \par Additional information
///
/// * readme.md - system requirements
/// * CHANGELOG - change history
/// * license.txt - license information (BSD)

// ----------------------------------------------------------------------------
// Config section
// ----------------------------------------------------------------------------
// define to have string suffix handling
#define CV_STRING_SUFFIX

// define to enable range checking
#define CV_DEF_RANGE_CHECK
// define to throw on range error
#define CV_DEF_RANGE_ERR_THROWS
// define to activate by default recursive functions, required if caller function is constexpr
#define CV_DEF_RECURSIVE

// ----------------------------------------------------------------------------
// Symbol calculation section
// ----------------------------------------------------------------------------
#define CV_NO_INLINE_VC __declspec(noinline)
#define CV_NO_INLINE_GCC __attribute__((noinline))

#if defined(_MSC_VER)
#define CV_NO_INLINE CV_NO_INLINE_VC
#else
#define CV_NO_INLINE CV_NO_INLINE_GCC
#endif

#ifndef CV_DEF_RANGE_CHECK
#undef CV_DEF_RANGE_ERR_THROWS
#endif // CV_DEF_RANGE_CHECK

// ----------------------------------------------------------------------------
// Includes section
// ----------------------------------------------------------------------------
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace sbt
{

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

/// Policy which enables recursive versions of functions
struct RecursivePolicyEnabled {};
/// Policy which enables iterative versions of functions
struct RecursivePolicyDisabled {};

template<typename T>
struct RecursivePolicyTag {
    typedef RecursivePolicyDisabled type;
    typedef std::false_type value_type;
};

template <>
struct RecursivePolicyTag<RecursivePolicyEnabled> {
    typedef RecursivePolicyEnabled type;
    typedef std::true_type value_type;
};

/// Policy which throws exceptions on error and does not store error information
template<class charT>
class ErrorPolicyThrowing {
protected:
    /// if error returns errorResult, otherwise '\0' (workaround for throwing inside constexpr function)
    constexpr charT signal_range_check_error(bool error, charT errorResult) {
       return (!error)?'\0':throw std::runtime_error("ERROR: basic_char_view - index out of bounds");
    }
};

/// Policy which returns empty value on error
template<class charT>
class ErrorPolicyEmptyValue {
protected:
    /// if error returns errorResult, otherwise '\0' (workaround for throwing inside constexpr function)
    constexpr charT signal_range_check_error(bool error, charT errorResult) {
       return (error)?errorResult:'\0';
    }
};

/// Policy which activates range checking
class RangeCheckPolicyEnabled {
protected:
     constexpr bool check_index_out_of_bounds(size_t a_size, size_t a_index) const {
         return (a_index >= a_size);
     }

     constexpr bool check_is_empty(size_t a_size) const {
         return (a_size == 0);
     }
};

/// Policy with range checking disabled: for ultra-fast access
class RangeCheckPolicyDisabled {
protected:
     constexpr bool check_index_out_of_bounds(size_t a_size, size_t a_index) const {
         return false;
     }

     constexpr bool check_is_empty(size_t a_size) const {
         return false;
     }
};

typedef char default_char_t;

#ifdef CV_DEF_RANGE_ERR_THROWS
template<class charT = default_char_t>
using default_error_policy = ErrorPolicyThrowing<charT>;
#else
template<class charT = default_char_t>
using default_error_policy = ErrorPolicyEmptyValue<charT>;
#endif // CV_DEF_RANGE_ERR_THROWS

#ifdef CV_DEF_RECURSIVE
typedef RecursivePolicyEnabled default_recursive_policy;
#else
typedef RecursivePolicyDisabled default_recursive_policy;
#endif

#ifdef CV_DEF_RANGE_CHECK
typedef RangeCheckPolicyEnabled default_range_check_policy;
#else
typedef RangeCheckPolicyDisabled default_range_check_policy;
#endif

/// Internal namespace - contents not for use outside of library.
namespace details
{
    // struct implementing iterative versions of functions
    template<class charT>
    struct no_inline {

        // DJB Hash function (original author unknown)
        CV_NO_INLINE static unsigned int str_hash_loop(const charT* str, size_t pos = 0, size_t limit = -1)
        {
            const int MAGIC_NUM = 5381;

            size_t end_pos = pos;
            while ((limit != 0) && str[end_pos]) {
                ++end_pos;
                --limit;
            }

            int result = MAGIC_NUM;
            while(end_pos > pos) {
              --end_pos;
              result = str[end_pos] ^ (33 * result);
            }

            return result;
        }

        // Calculate length of zero-ended string
        CV_NO_INLINE static size_t length(const charT* str)
        {
            size_t res = 0;
            while(*str++)
                ++res;
            return res;
        }

        // calculate number of equal characters in both strings
        CV_NO_INLINE static size_t common_length(const charT* content, const charT *search_text, size_t content_limit, ssize_t search_limit = -1)
        {
            if (search_limit == 0)
                return 0;

            int res = 0;
            if (search_limit < 0) {
              while((content_limit-- > 0) && (search_text[res] == content[res]) && (content[res]))
                res++;
            } else {
              while((content_limit-- > 0) && (search_limit-- > 0) && (search_text[res] == content[res]) && (content[res]))
                res++;
            }

            return res;
        }
    };

    // Calculate length of zero-ended string
    template<class charT>
	size_t constexpr length(const charT* str)
	{
		return *str ? 1 + length(str + 1) : 0;
	}

    /*
     * Calculate hash of string     * param[in] str input text (can be zero-ended)
     * param[in] pos starting pos
     * param[in] limit maximum number of characters to be used
     * return Integer number equal to hash value.
     */
    template<class charT>
    unsigned int constexpr str_hash(const charT* str, size_t pos = 0, size_t limit = -1)
    {
        // DJB Hash function (original author unknown)
        //return (limit == 0 || !str[pos]) ? 5381 : (str_hash(str, pos + 1, limit - 1) * 33) ^ str[pos];
        return (limit == 0 || !str[pos]) ? 5381 : str[pos] ^ (33 * str_hash(str, pos + 1, limit - 1));
    }

    // Checks if content text is started with a provided search_text.    // param[in] content input text to be scanned
    // param[in] search_text text to be found, can be zero-ended.
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text, -1 if unknown
    // return Returns true if whole search_text is found at position 0 in content.
    template<class charT>
	bool constexpr starts_with(const charT* content, const charT *search_text, size_t content_limit, ssize_t search_limit = -1)
	{
		return
		  (search_limit == 0)?true:
		     (content_limit == 0)?
		        (
                  (search_limit < 0)?
                     (search_text[0] == '\0'):
                     false
                )
		        :
                (content[0] != search_text[0])?
                   (search_text[0] == '\0'):
                   (!content[0]?true:
                      starts_with(content + 1, search_text + 1, content_limit - 1, search_limit - 1)
                   )
                ;
	}

    // Returns number of characters common in both strings at the start.    // param[in] content input text to be scanned
    // param[in] search_text second text, can be zero-ended
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text, -1 if unknown
    // param[in] match_len number of common characters already found
    // return Returns number of common (equal) characters.
    template<class charT>
	size_t constexpr common_length(const charT* content, const charT *search_text, size_t content_limit, ssize_t search_limit = -1, size_t match_len = 0)
	{
		return
		  (search_limit == 0)?match_len:
		     (content_limit == 0)?
		        match_len:
                (content[0] != search_text[0])?
                   match_len:
                   (!content[0]?match_len:
                      common_length(content + 1, search_text + 1, content_limit - 1, search_limit - 1, match_len + 1)
                   )
                ;
	}

    // Checks if content ending part (tail) is equal to a given string.    // param[in] content input text to be scanned
    // param[in] search_text text to be found.
    // param[in] content_pos position of last character to be checked inside content
    // param[in] search_pos position of last character to be checked inside search_text
    // return Returns true if search_text is equal to last characters of content.
    template<class charT>
	bool constexpr ends_with(const charT* content, const charT *search_text,
                             ssize_t content_pos, ssize_t search_pos)
	{
		return
          (search_pos < 0)?true:
	    	  (content_pos < 0)?false:
	             (content[content_pos] != search_text[search_pos])?
                        false:
                        ends_with(content, search_text, content_pos - 1, search_pos - 1);
	}

    // Compare strings.    // Returns value < 0 if search_text is shorter if first non-matching character has less value than in content text.
    // Returns value > 0 if search_text is longer if first non-matching character has greater value than in content text.
    // Returns value == 0 if search_text is equal to content.
    // param[in] content input text to be scanned
    // param[in] search_text text to be found, can be zero-ended.
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text
    // return Returns integer compare result.
    template<class charT>
	int constexpr compare(const charT* content, const charT *search_text, size_t content_limit, ssize_t search_limit = -1)
	{
		return
		  (search_limit == 0)?
                 (
                   (content_limit == 0)?0:1
                 )
	             :
                 (content_limit == 0)?
                    (!search_text[0]?0:-1)
                    :
                    (content[0] > search_text[0])?1:
                       (content[0] < search_text[0])?-1:
                       (!content[0]?0:
                          compare(content + 1, search_text + 1, content_limit - 1, search_limit - 1)
                       )
                ;
	}

    // Check if given string is inside provided content.    // param[in] content input text to be scanned
    // param[in] search_text text to be found
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text
    // return Returns true if found, otherwise false.
    template<class charT>
	bool constexpr contains(const charT* content, const charT *search_text, size_t content_limit, size_t search_limit) {
	    return
	      (content_limit < search_limit)?false:
             (content_limit == 0)?(search_limit == 0):
                 (compare(content, search_text, search_limit, search_limit) == 0)?true:
                    contains(content + 1, search_text, content_limit - 1, search_limit);
	}

    // Find position of a given search string found inside content string.    // param[in] content input text to be scanned
    // param[in] search_text text to be found
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text
    // param[in] offset current position inside content
    // return Returns -1 if not found, otherwise zero-based position of search_text inside content
    template<class charT>
	ssize_t constexpr index_of(const charT* content, const charT *search_text, size_t content_limit, size_t search_limit, size_t offset = 0) {
	    return
	      (content_limit < search_limit)?-1:
             (content_limit == 0)?
                 ((search_limit == 0)?offset:-1):
                 //(compare(content, search_text, search_limit, search_limit) == 0)?offset:
                    starts_with(content, search_text, search_limit, search_limit)?offset:
                       index_of(content + 1, search_text, content_limit - 1, search_limit, offset + 1);
	}

    // Find position of first character that is from a given character set.    // param[in] content input text to be scanned
    // param[in] search_set character set
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_set
    // param[in] offset current position inside content
    // return Returns -1 if not found, otherwise zero-based position of character inside content
    template<class charT>
	ssize_t constexpr index_of_any(const charT* content, const charT *search_set, size_t content_limit, size_t search_limit, size_t offset = 0) {
	    return
             (content_limit == 0)?
                 -1:
                 (search_limit == 0?-1:
                   (index_of(search_set, content, search_limit, 1) >= 0)?offset:
                       index_of_any(content + 1, search_set, content_limit - 1, search_limit, offset + 1));
	}

    // Find position of first character that is not from a given character set.    // param[in] content input text to be scanned
    // param[in] search_set character set
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_set
    // param[in] offset current position inside content
    // return Returns -1 if not found, otherwise zero-based position of character inside content
    template<class charT>
	ssize_t constexpr index_of_nonmatching(const charT* content, const charT *search_set, size_t content_limit, size_t search_limit, size_t offset = 0) {
	 	    return
             (content_limit == 0)?
                 ((search_limit == 0)?offset:-1):
                 (index_of(search_set, content, search_limit, 1) < 0)?offset:
                     index_of_nonmatching(content + 1, search_set, content_limit - 1, search_limit, offset + 1);
	}

    // Find last position of a given string in content.    // param[in] content input text to be scanned
    // param[in] search_text text to be found
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_text
    // param[in] offset current position inside content
    // param[in] found_pos last found position, -1 if not found yet
    // return Returns -1 if not found, otherwise zero-based position of character inside content
    template<class charT>
	ssize_t constexpr last_index_of(const charT* content, const charT *search_text, size_t content_limit, size_t search_limit, size_t offset = 0, ssize_t found_pos = -1) {
	    return
	      (content_limit < search_limit)?found_pos:
             (content_limit == 0)?
                 ((search_limit == 0)?offset:found_pos):
                 (compare(content, search_text, search_limit, search_limit) == 0)?
                    last_index_of(content + 1, search_text, content_limit - 1, search_limit, offset + 1, offset):
                    last_index_of(content + 1, search_text, content_limit - 1, search_limit, offset + 1, found_pos);
	}

    // Find last position in content of character matching any character from a given character set.    // param[in] content input text to be scanned
    // param[in] search_set set of characters
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_set
    // param[in] offset current position inside content
    // param[in] found_pos last found position, -1 if not found yet
    // return Returns -1 if not found, otherwise zero-based position of character inside content
    template<class charT>
	ssize_t constexpr last_index_of_any(const charT* content, const charT *search_set, size_t content_limit, size_t search_limit, size_t offset = 0, ssize_t found_pos = -1) {
	    return
             (content_limit == 0)?
                 ((search_limit == 0)?-1:found_pos):
                 (index_of(search_set, content, search_limit, 1) >= 0)?
                    last_index_of_any(content + 1, search_set, content_limit - 1, search_limit, offset + 1, offset):
                    last_index_of_any(content + 1, search_set, content_limit - 1, search_limit, offset + 1, found_pos);
	}

    // Find last position in content of character not matching any character from a given character set.    // param[in] content input text to be scanned
    // param[in] search_set set of characters
    // param[in] content_limit number of characters in content
    // param[in] search_limit number of characters in search_set
    // param[in] offset current position inside content
    // param[in] found_pos last found position, -1 if not found yet
    // return Returns -1 if not found, otherwise zero-based position of character inside content
    template<class charT>
	ssize_t constexpr last_index_of_nonmatching(const charT* content, const charT *search_set, size_t content_limit, size_t search_limit, size_t offset = 0, ssize_t found_pos = -1) {
	    return
             (content_limit == 0)?
                 ((search_limit == 0)?offset:found_pos):
                 (index_of(search_set, content, search_limit, 1) < 0)?
                    last_index_of_nonmatching(content + 1, search_set, content_limit - 1, search_limit, offset + 1, offset):
                    last_index_of_nonmatching(content + 1, search_set, content_limit - 1, search_limit, offset + 1, found_pos);
	}
}

/**
  * @brief Read-only view for character containers: literals, std::string, char buffers.
  * Implements compilation-time and runtime-calculated functions.
  *  * Uses the following template arguments:
  * - charT: character data type (char, wchar, char16_t etc)
  * - RecursivePolicy: informs if recursive functions should be used or not (required for compile-time evaluation)
  * - RangeCheckPolicy: performs (or not) range checking for function arguments
  * - ErrorPolicy: performs throw (or not) on range check errors (if active)
  *
  * Uses function names based on std::string with addition of some functions from Java's string type.
  *
  * In general, there are the following function overloads defined:
  * - f(charT *): for functions expecting that string ends with '\0'
  * - f(charT *, size_t): for functions expecting strings with specified size (size does not include '\0')
  * - f_cn(charT *, size_t a_len): overload added to eliminate possibility to mistake this function with std::string equivalent
  * - f(basic_char_view): for functions handling char_view type (same as "this")
  * - f(std::basic_string): for functions handling standard strings
  */
template<class charT,
         typename RecursivePolicy = default_recursive_policy,
         typename RangeCheckPolicy = default_range_check_policy,
         typename ErrorPolicy = default_error_policy<charT>>
class basic_char_view: private RangeCheckPolicy, private ErrorPolicy
{
	const charT* m_str;
	const size_t m_size;

    size_t length(const charT* str, RecursivePolicyDisabled) {
        return details::no_inline<charT>::length(str);
    }

    constexpr size_t length(const charT* str, RecursivePolicyEnabled) {
        return details::length(str);
    }

public:
    typedef const charT *const_iterator;
    typedef basic_char_view<charT, RecursivePolicy, RangeCheckPolicy, ErrorPolicy> this_type;

    /// null position (undefined)
    static const size_t npos = -1;

	constexpr basic_char_view(const charT* a_str, int a_size) : m_str(a_str), m_size(a_size) {
	};

	constexpr basic_char_view(const charT* a_str):
	    m_str(a_str),
	    m_size(
          details::length(a_str)
        )
    {
	};

	constexpr basic_char_view(const charT* a_str, RecursivePolicyEnabled):
	    m_str(a_str),
	    m_size(
          details::length(a_str)
        )
    {
	};

	basic_char_view(const charT* a_str, RecursivePolicyDisabled):
	    m_str(a_str),
	    m_size(
          details::no_inline<charT>::length(a_str)
        )
    {
	};

    /// returns character at a given position, fails if out of bounds
	constexpr charT operator[](size_t index) const
	{
		return
		    (this_type::check_index_out_of_bounds(m_size, index)?
               this_type::signal_range_check_error(true, '\0'):
               m_str[index]);
	}

    /// returns character at a given position, fails if out of bounds
	constexpr charT at(size_t index) const
	{
		return (*this)[index];
	}

    /// returns first character, by default throws if string is empty
	constexpr charT front() const
	{
		return (*this)[0];
	}

    /// returns last character, by default throws if string is empty
	constexpr charT back() const
	{
		return
		  this_type::check_is_empty(m_size)?
		      this_type::signal_range_check_error(true, '\0'):
		      (*this)[m_size - 1];
	}

    /// returns first n characters, if string is shorter than n returns whole string
	constexpr this_type front(size_t a_len) const
	{
		return substr(0, (a_len <= m_size)?a_len:m_size);
	}

    /// returns last n characters, if string is shorter than n returns whole string
	constexpr this_type back(size_t a_len) const
	{
		return (a_len == 0)?this_type(""):
		         (a_len <= m_size)?substr(m_size - a_len, a_len):
		             substr(0, m_size);
	}

    /// returns string size
	constexpr size_t size() const { return m_size; }

    /// returns true if string is empty (size == 0)
	constexpr bool empty() const { return (m_size == 0); }

    /// returns pointer to character data, data is not required to be ending with '\0', contains size() characters
	constexpr const charT* data() const noexcept { return m_str; }

    /// converts string view to standard string
	operator std::basic_string<charT>() const { return std::basic_string<charT>(m_str, m_size); }

    /// @brief Extracts part of string as view, index must be inside range or function will fail.
    /// @details If string part is longer than contained string, only characters contained inside view will be returned.
    /// @param[in] index starting position, zero-based.
    /// @param[in] len number of characters to be returned, if ommited all characters left will be returned
    /// @return returns new char view object
    constexpr this_type substr(size_t index = 0, size_t len = npos) const {
       return
              (
                this_type::check_index_out_of_bounds(m_size, index)?
                  this_type("",
                            this_type::signal_range_check_error(true, '\0')):
                  this_type(
                              m_str + index,
                              (index + len <= m_size)?
                                len:
                                m_size - index
                           )
              );
    }

    /// Returns position of first character.
    constexpr const_iterator begin() const {
        return m_str;
    }

    /// Returns position after last character.
    constexpr const_iterator end() const {
        return m_str + m_size;
    }

    /// Returns position of first character.
    constexpr const_iterator cbegin() const {
        return m_str;
    }

    /// Returns position after last character.
    constexpr const_iterator cend() const {
        return m_str + m_size;
    }

private:
    unsigned int hash_code(RecursivePolicyDisabled) const {
        return details::no_inline<charT>::str_hash_loop(m_str, 0, m_size);
    }

    constexpr unsigned int hash_code(RecursivePolicyEnabled) const {
        return details::str_hash(m_str, 0, m_size);
    }

public:
    /// @brief Calculate hash value for contained string.
    /// @details Note: values are not unique and can be equal for different strings (but it's difficult to achieve).
    constexpr unsigned int hash_code() const {
        return hash_code(typename RecursivePolicyTag<RecursivePolicy>::type());
    }

private:
    bool starts_with(const charT* a_str, RecursivePolicyDisabled) const {
        std::basic_string<charT> str(a_str);
        return (str.substr(0, m_size) == std::basic_string<charT>(m_str, str.size()));
    }

    constexpr bool starts_with(const charT* a_str, RecursivePolicyEnabled) const {
        return details::starts_with(m_str, a_str, m_size);
    }

    bool starts_with(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        std::basic_string<charT> str(a_str, std::min(a_len, m_size));
        return (str == std::basic_string<charT>(m_str, m_size));
    }

    constexpr bool starts_with(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::starts_with(m_str, a_str, m_size, a_len);
    }

    bool starts_with(const this_type &a_str, RecursivePolicyDisabled) const {
        return (a_str == substr(0, a_str.size()));
    }

    constexpr bool starts_with(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::starts_with(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    bool starts_with(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return (a_str == std::basic_string<charT>(m_str, a_str.size()));
    }

    bool starts_with(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::starts_with(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup starts_with
    /// @brief Check if this string has specified prefix.
    /// @param[in] a_str prefix string
    /// @return returns true if first n characters of this string are equal to specified prefix string where n is equal to length of prefix.
    //@{
    /// @brief Check if this string has specified prefix.
    /// @details Overload for zstring.
    constexpr bool starts_with(const charT* a_str) const {
        return starts_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr bool starts_with(const charT* a_str, size_t a_len) const {
        return starts_with(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr bool starts_with(const this_type &a_str) const {
        return starts_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    bool starts_with(const std::basic_string<charT> &a_str) const {
        return starts_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}
private:
    bool ends_with(const charT* a_str, RecursivePolicyDisabled) const {
        std::basic_string<charT> str(a_str);
        if (str.size() > m_size)
            return false;
        if (!m_size)
            return true;
        return (std::basic_string<charT>(m_str, m_size).substr(m_size - str.size(), str.size()) == str);
    }

    constexpr bool ends_with(const charT* a_str, RecursivePolicyEnabled) const {
        return details::ends_with(m_str, a_str, (ssize_t)m_size - 1, (ssize_t)length(a_str, RecursivePolicyEnabled()) - 1);
    }

    bool ends_with(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        std::basic_string<charT> str(a_str, a_len);
        if (str.size() > m_size)
            return false;
        if (!m_size)
            return true;
        return (std::basic_string<charT>(m_str, m_size).substr(m_size - str.size(), str.size()) == str);
    }

    constexpr bool ends_with(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::ends_with(m_str, a_str, (ssize_t)m_size - 1, (ssize_t)a_len - 1);
    }

    bool ends_with(const this_type &a_str, RecursivePolicyDisabled) const {
        if (a_str.size() > m_size)
            return false;
        if (!m_size)
            return true;
        return (this->substr(m_size - a_str.size(), a_str.size()) == a_str);
    }

    constexpr bool ends_with(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::ends_with(m_str, a_str.m_str, (ssize_t)m_size - 1, (ssize_t)a_str.m_size - 1);
    }

    bool ends_with(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        if (a_str.size() > m_size)
            return false;
        if (!m_size)
            return true;
        return (this->substr(m_size - a_str.size(), a_str.size()) == a_str);
    }

    bool ends_with(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::ends_with(m_str, a_str.c_str(), (ssize_t)m_size - 1, (ssize_t)a_str.size() - 1);
    }

public:
    /// \defgroup ends_with
    /// @brief Check if this string has specified suffix.
    /// @param[in] a_str suffix string
    /// @return returns true if last n characters of this string are equal to specified suffix string where n is equal to length of suffix.
    //@{
    /// @brief Check if this string has specified suffix.
    /// @details Overload for zstring.
    constexpr bool ends_with(const charT* a_str) const {
        return ends_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr bool ends_with(const charT* a_str, size_t a_len) const {
        return ends_with(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr bool ends_with(const this_type &a_str) const {
        return ends_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    bool ends_with(const std::basic_string<charT> &a_str) const {
        return ends_with(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    bool equals(const charT* a_str, RecursivePolicyDisabled) const {
        return (details::no_inline<charT>::common_length(m_str, a_str, m_size, -1) == m_size) && (a_str[m_size] == '\0');
    }

    constexpr bool equals(const charT* a_str, RecursivePolicyEnabled) const {
        return (details::common_length(m_str, a_str, m_size, -1) == m_size) && (a_str[m_size] == '\0');
    }

    bool equals(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return (a_len != m_size)?false:(details::no_inline<charT>::common_length(m_str, a_str, m_size, m_size) == m_size);
    }

    constexpr bool equals(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return (a_len != m_size)?false:details::starts_with(m_str, a_str, m_size, m_size);
    }

    bool equals(const this_type &a_str, RecursivePolicyDisabled) const {
        return (a_str.m_size != m_size)?false:(details::no_inline<charT>::common_length(m_str, a_str.m_str, m_size, m_size) == m_size);
    }

    constexpr bool equals(const this_type &a_str, RecursivePolicyEnabled) const {
        return (a_str.m_size != m_size)?false:details::starts_with(m_str, a_str.m_str, m_size, m_size);
    }

    bool equals(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return (a_str.size() != m_size)?false:(details::no_inline<charT>::common_length(m_str, a_str.c_str(), m_size, m_size) == m_size);
    }

    bool equals(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return (a_str.size() != m_size)?false:details::starts_with(m_str, a_str.c_str(), m_size, m_size);
    }

public:
    /// \defgroup equals
    /// @brief Check if this string has the same contents as a specified string.
    /// @param[in] a_str string to test
    /// @return returns true if length and all characters are equal in both strings.
    //@{
    /// @brief Check if this string has the same contents as a specified string.
    /// @details Overload for zstring.
    constexpr bool equals(const charT* a_str) const {
        return equals(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr bool equals(const charT* a_str, size_t a_len) const {
        return equals(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr bool equals(const this_type &a_str) const {
        return equals(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    bool equals(const std::basic_string<charT> &a_str) const {
        return equals(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    int compare(const charT* a_str, RecursivePolicyDisabled) const {
        std::string s(m_str, m_size);
        return s.compare(a_str);
    }

    constexpr int compare(const charT* a_str, RecursivePolicyEnabled) const {
        return details::compare(m_str, a_str, m_size);
    }

    int compare(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        std::string s(m_str, m_size);
        return s.compare(0, m_size, a_str, a_len);
    }

    constexpr int compare(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::compare(m_str, a_str, m_size, a_len);
    }

public:
    /// \defgroup compare
    /// @brief Compare two strings: this string and provided parameter string.
    /// @param[in] a_str string to test
    /// @return returns value < 0 if specified string is longer than this string or first non-matching character is greater in parameter string
    /// returns value > 0 if specified string is shorter than this string or first non-matching character is lower in parameter string
    /// returns value = 0 if both strings are the equal
    //@{
    /// @brief Compare two strings: this string and provided parameter string.
    /// @details Overload for zstring.
    constexpr int compare(const charT* a_str) const {
        return compare(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr int compare(const charT* a_str, size_t a_len) const {
        return compare(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr int compare(const this_type &a_str) const {
        return details::compare(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    /// @brief overload for standard string
    int compare(const std::basic_string<charT> &a_str) const {
        return details::compare(m_str, a_str.c_str(), m_size, a_str.size());
    }
    //@}

private:
    bool contains(const charT* a_str, RecursivePolicyDisabled) const {
        return (std::basic_string<charT>(m_str, m_size).find(a_str) != std::string::npos);
    }

    constexpr bool contains(const charT* a_str, RecursivePolicyEnabled) const {
        return details::contains(m_str, a_str, m_size, length(a_str, RecursivePolicyEnabled()));
    }

    bool contains(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return (std::basic_string<charT>(m_str, m_size).find(a_str, 0, a_len) != std::string::npos);
    }

    constexpr bool contains(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::contains(m_str, a_str, m_size, a_len);
    }

    bool contains(const this_type &a_str, RecursivePolicyDisabled) const {
        return (std::basic_string<charT>(m_str, m_size).find(a_str.m_str, 0, a_str.m_size) != std::string::npos);
    }

    constexpr bool contains(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::contains(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    bool contains(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return (std::basic_string<charT>(m_str, m_size).find(a_str) != std::string::npos);
    }

    bool contains(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::contains(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup contains
    /// @brief Check if this string contains character sequance equal to provided string.
    /// @param[in] a_str string to test, can be shorter than this string.
    /// @return returns true if this string contains provided string.
    //@{
    /// @brief Compare two strings: this string and provided parameter string.
    /// @details Overload for zstring.
    constexpr bool contains(const charT* a_str) const {
        return contains(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr bool contains(const charT* a_str, size_t a_len) const {
        return contains(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr bool contains(const this_type &a_str) const {
        return contains(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    bool contains(const std::basic_string<charT> &a_str) const {
        return contains(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t find(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find(a_str);
    }

    constexpr size_t find(const charT* a_str, RecursivePolicyEnabled) const {
        return details::index_of(m_str, a_str, m_size, length(a_str, RecursivePolicyEnabled()));
    }

    size_t find(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find(a_str, 0, a_len);
    }

    constexpr size_t find(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::index_of(m_str, a_str, m_size, a_len);
    }

    size_t find(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find(a_str.m_str, 0, a_str.m_size);
    }

    constexpr size_t find(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::index_of(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    size_t find(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find(a_str);
    }

    size_t find(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::index_of(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup find
    /// @brief Find position of a_str string inside this string.
    /// @param[in] a_str string to be found, can be shorter than this string.
    /// @return returns position of a_str (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find position of a_str string inside this string.
    /// @details Overload for zstring.
    constexpr size_t find(const charT* a_str) const {
        return find(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t find_cn(const charT* a_str, size_t a_len) const {
        return find(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t find(const this_type &a_str) const {
        return find(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t find(const std::basic_string<charT> &a_str) const {
        return find(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t rfind(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).rfind(a_str);
    }

    constexpr size_t rfind(const charT* a_str, RecursivePolicyEnabled) const {
        return details::last_index_of(m_str, a_str, m_size, length(a_str, RecursivePolicyEnabled()));
    }

    size_t rfind(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).rfind(a_str, 0, a_len);
    }

    constexpr size_t rfind(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::last_index_of(m_str, a_str, m_size, a_len);
    }

    size_t rfind(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).rfind(a_str.m_str, 0, a_str.m_size);
    }

    constexpr size_t rfind(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    size_t rfind(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).rfind(a_str);
    }

    size_t rfind(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup rfind
    /// @brief Find last occurance of a_str string inside this string.
    /// @param[in] a_str string to be found, can be shorter than this string.
    /// @return returns position of a_str (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find last position of a_str string inside this string.
    /// @details Overload for zstring.
    constexpr size_t rfind(const charT* a_str) const {
        return rfind(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t rfind_cn(const charT* a_str, size_t a_len) const {
        return rfind(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t rfind(const this_type &a_str) const {
        return rfind(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t rfind(const std::basic_string<charT> &a_str) const {
        return rfind(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t find_first_of(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_of(a_str);
    }

    constexpr size_t find_first_of(const charT* a_str, RecursivePolicyEnabled) const {
        return details::index_of_any(m_str, a_str, m_size, details::length(a_str));
    }

    size_t find_first_of(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_of(a_str, 0, a_len);
    }

    constexpr size_t find_first_of(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::index_of_any(m_str, a_str, m_size, a_len);
    }

    size_t find_first_of(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_of(a_str.m_str, 0, a_str.m_size);
    }

    constexpr size_t find_first_of(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::index_of_any(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    size_t find_first_of(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_of(a_str);
    }

    size_t find_first_of(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::index_of_any(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup find_first_of
    /// @brief Find position of first character in this string which is included in provided character set.
    /// @param[in] a_str character set
    /// @return returns position of first matching character inside this string (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find position of first character in this string which is included in provided character set.
    /// @details Overload for zstring.
    constexpr size_t find_first_of(const charT* a_str) const {
        return find_first_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t find_first_of_cn(const charT* a_str, size_t a_len) const {
        return find_first_of(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t find_first_of(const this_type &a_str) const {
        return find_first_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t find_first_of(const std::basic_string<charT> &a_str) const {
        return find_first_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t find_first_not_of(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_not_of(a_str);
    }

    constexpr size_t find_first_not_of(const charT* a_str, RecursivePolicyEnabled) const {
        return details::index_of_nonmatching(m_str, a_str, m_size, details::length(a_str));
    }

    size_t find_first_not_of(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_not_of(a_str, 0, a_len);
    }

    constexpr size_t find_first_not_of(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::index_of_nonmatching(m_str, a_str, m_size, a_len);
    }

    size_t find_first_not_of(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_not_of(a_str.m_str, 0, a_str.m_size);
    }

    constexpr size_t find_first_not_of(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::index_of_nonmatching(m_str, a_str.m_str, m_size, a_str.size());
    }

    size_t find_first_not_of(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_first_not_of(a_str);
    }

    size_t find_first_not_of(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::index_of_nonmatching(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup find_first_not_of
    /// @brief Find position of first character in this string which is NOT included in provided character set.
    /// @param[in] a_str character set
    /// @return returns position of first non-matching character inside this string (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find position of first character in this string which is NOT included in provided character set.
    /// @details Overload for zstring.
    constexpr size_t find_first_not_of(const charT* a_str) const {
        return find_first_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t find_first_not_of_cn(const charT* a_str, size_t a_len) const {
        return find_first_not_of(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t find_first_not_of(const this_type &a_str) const {
        return find_first_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t find_first_not_of(const std::basic_string<charT> &a_str) const {
        return find_first_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t find_last_of(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_of(a_str);
    }

    constexpr size_t find_last_of(const charT* a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_any(m_str, a_str, m_size, details::length(a_str));
    }

    size_t find_last_of(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_of(a_str, std::string::npos, a_len);
    }

    constexpr size_t find_last_of(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::last_index_of_any(m_str, a_str, m_size, a_len);
    }

    size_t find_last_of(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_of(a_str.m_str, std::string::npos, a_str.m_size);
    }

    constexpr size_t find_last_of(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_any(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    size_t find_last_of(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_of(a_str);
    }

    size_t find_last_of(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_any(m_str, a_str.c_str(), m_size, a_str.size());
    }
public:
    /// \defgroup find_last_of
    /// @brief Find last position of character in this string which is included in provided character set.
    /// @param[in] a_str character set
    /// @return returns position of last matching character inside this string (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find last position of character in this string which is included in provided character set.
    /// @details Overload for zstring.
    constexpr size_t find_last_of(const charT* a_str) const {
        return find_last_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t find_last_of_cn(const charT* a_str, size_t a_len) const {
        return find_last_of(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t find_last_of(const this_type &a_str) const {
        return find_last_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t find_last_of(const std::basic_string<charT> &a_str) const {
        return find_last_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}

private:
    size_t find_last_not_of(const charT* a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_not_of(a_str);
    }

    constexpr size_t find_last_not_of(const charT* a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_nonmatching(m_str, a_str, m_size, length(a_str, RecursivePolicyEnabled()));
    }

    size_t find_last_not_of(const charT* a_str, size_t a_len, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_not_of(a_str, std::string::npos, a_len);
    }

    constexpr size_t find_last_not_of(const charT* a_str, size_t a_len, RecursivePolicyEnabled) const {
        return details::last_index_of_nonmatching(m_str, a_str, m_size, a_len);
    }

    size_t find_last_not_of(const this_type &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_not_of(a_str.m_str, std::string::npos, a_str.m_size);
    }

    constexpr size_t find_last_not_of(const this_type &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_nonmatching(m_str, a_str.m_str, m_size, a_str.m_size);
    }

    size_t find_last_not_of(const std::basic_string<charT> &a_str, RecursivePolicyDisabled) const {
        return std::basic_string<charT>(m_str, m_size).find_last_not_of(a_str);
    }

    size_t find_last_not_of(const std::basic_string<charT> &a_str, RecursivePolicyEnabled) const {
        return details::last_index_of_nonmatching(m_str, a_str.c_str(), m_size, a_str.size());
    }

public:
    /// \defgroup find_last_not_of
    /// @brief Find last position of character in this string which is NOT included in provided character set.
    /// @param[in] a_str character set
    /// @return returns position of last non-matching character inside this string (zero-based) or basic_char_view::npos if not found.
    //@{
    /// @brief Find last position of character in this string which is NOT included in provided character set.
    /// @details Overload for zstring.
    constexpr size_t find_last_not_of(const charT* a_str) const {
        return find_last_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for character buffer
    /// @param[in] a_len number of characters inside a_str string
    constexpr size_t find_last_not_of_cn(const charT* a_str, size_t a_len) const {
        return find_last_not_of(a_str, a_len, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for char_view
    constexpr size_t find_last_not_of(const this_type &a_str) const {
        return find_last_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }

    /// @brief overload for standard string
    size_t find_last_not_of(const std::basic_string<charT> &a_str) const {
        return find_last_not_of(a_str, typename RecursivePolicyTag<RecursivePolicy>::type());
    }
    //@}
};

typedef basic_char_view<char> char_view;
typedef basic_char_view<wchar_t> wchar_view;
typedef basic_char_view<char16_t> char16_view;
typedef basic_char_view<char32_t> char32_view;

// ----------------------------------------------------------------------------
// Global functions, operators
// ----------------------------------------------------------------------------

#ifdef CV_STRING_SUFFIX
constexpr char_view operator "" _cv(const char* s, std::size_t n) { return char_view(s, n); }
constexpr wchar_view operator "" _cv(const wchar_t* s, std::size_t n) { return wchar_view(s, n); }
constexpr char16_view operator "" _cv(const char16_t* s, std::size_t n) { return char16_view(s, n); }
constexpr char32_view operator "" _cv(const char32_t* s, std::size_t n) { return char32_view(s, n); }
#endif

}; // namespace

template<typename charT>
  constexpr bool operator<(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) > 0; }
template<typename charT>
  constexpr bool operator<=(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) >= 0; }
template<typename charT>
  constexpr bool operator>(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) < 0; }
template<typename charT>
  constexpr bool operator>=(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) <= 0; }
template<typename charT>
  constexpr bool operator==(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.equals(ltr); }
template<typename charT>
  constexpr bool operator!=(const sbt::basic_char_view<charT>& ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) != 0; }

template<typename charT>
  constexpr bool operator<(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) > 0; }
template<typename charT>
  constexpr bool operator<=(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) >= 0; }
template<typename charT>
  constexpr bool operator>(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) < 0; }
template<typename charT>
  constexpr bool operator>=(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) <= 0; }
template<typename charT>
  constexpr bool operator==(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.equals(ltr); }
template<typename charT>
  constexpr bool operator!=(const char* ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) != 0; }

template<typename charT>
  constexpr bool operator<(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.compare(rtr) < 0; }
template<typename charT>
  constexpr bool operator<=(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.compare(rtr) <= 0; }
template<typename charT>
  constexpr bool operator>(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.compare(rtr) > 0; }
template<typename charT>
  constexpr bool operator>=(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.compare(rtr) >= 0; }
template<typename charT>
  constexpr bool operator==(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.equals(rtr); }
template<typename charT>
  constexpr bool operator!=(const sbt::basic_char_view<charT>& ltr, const char* rtr) { return ltr.compare(rtr) != 0; }

template<typename charT>
    bool operator<(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) > 0; }
template<typename charT>
    bool operator<=(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) >= 0; }
template<typename charT>
    bool operator>(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) < 0; }
template<typename charT>
    bool operator>=(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) <= 0; }
template<typename charT>
    bool operator==(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.equals(ltr); }
template<typename charT>
    bool operator!=(const std::string &ltr, const sbt::basic_char_view<charT>& rtr) { return rtr.compare(ltr) != 0; }

template<typename charT>
    bool operator<(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.compare(rtr) < 0; }
template<typename charT>
    bool operator<=(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.compare(rtr) <= 0; }
template<typename charT>
    bool operator>(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.compare(rtr) > 0; }
template<typename charT>
    bool operator>=(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.compare(rtr) >= 0; }
template<typename charT>
    bool operator==(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.equals(rtr); }
template<typename charT>
    bool operator!=(const sbt::basic_char_view<charT>& ltr, const std::string &rtr) { return ltr.compare(rtr) != 0; }

namespace std {
    /// standard hash function specialization for char_view
    template <typename charT> struct hash<sbt::basic_char_view<charT>>
    {
        size_t operator()(const sbt::basic_char_view<charT> &x) const
        {
          return x.hash_code();
        }
    };

    /// standard function overload for char_view
    template <typename charT>
    std::string to_string(const sbt::basic_char_view<charT> &str) {
      return static_cast<std::string>(str);
    }
}

/// standard operator overload for char_view
template <typename charT>
std::ostream& operator<<(std::ostream& os, const sbt::basic_char_view<charT> &str)
{
	os.write(str.data(), str.size());
	return os;
}

#endif // _CHAR_VIEW_H__

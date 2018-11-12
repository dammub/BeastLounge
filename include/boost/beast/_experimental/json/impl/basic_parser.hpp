//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_JSON_IMPL_BASIC_PARSER_HPP
#define BOOST_BEAST_JSON_IMPL_BASIC_PARSER_HPP

#include <boost/beast/_experimental/json/error.hpp>
#include <boost/beast/core/buffers_range.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace beast {
namespace json {

/*
    https://www.json.org/

    json
        element

    value
        object
        array
        string
        number
        "true"
        "false"
        "null"

    object
        '{' ws '}'
        '{' members '}'

    members
        member
        member ',' members

    member
        ws string ws ':' element

    array
        '[' ws ']'
        '[' elements ']'

    elements
        element
        element ',' elements

    element
        ws value ws

    string
        '"' characters '"'
    characters
        ""
        character characters
    character
        '0020' . '10ffff' - '"' - '\'
        '\' escape
    escape
        '"'
        '\'
        '/'
        'b'
        'n'
        'r'
        't'
        'u' hex hex hex hex

    hex
        digit
        'A' . 'F'
        'a' . 'f'

    number
        int frac exp

    int
        digit
        onenine digits
        '-' digit
        '-' onenine digits

    digits
        digit
        digit digits

    digit
        '0'
        onenine

    onenine
        '1' . '9'

    frac
        ""
        '.' digits

    exp
        ""
        'E' sign digits
        'e' sign digits

    sign
        ""
        '+'
        '-'

    ws
        ""
        '0009' ws
        '000a' ws
        '000d' ws
        '0020' ws
*/

//------------------------------------------------------------------------------

namespace detail {

inline
bool
parser_base::
is_ws(char c) noexcept
{
    return
        c == '\0x20' ||
        c == '\0x0d' ||
        c == '\0x0a' ||
        c == '\0x09';
}

inline
bool
parser_base::is_digit(char c) noexcept
{
    return static_cast<unsigned char>(c-'0') < 10;
}

} // detail

//------------------------------------------------------------------------------

template<class Derived>
basic_parser<Derived>::
basic_parser()
{
    push_state(state::json);
}

template<class Derived>
template<class ConstBufferSequence>
void
basic_parser<Derived>::
write(ConstBufferSequence const& buffers, error_code& ec)
{
    ec.assign(0, ec.category());
    for(auto const b : beast::buffers_range(buffers))
    {
        write(b, ec);
        if(ec)
            return;
    }
}

template<class Derived>
void
basic_parser<Derived>::
write_eof(error_code& ec)
{
    switch(current_state())
    {
    case state::ws:
    case state::end:
        break;

    default:
        ec = error::syntax;
        return;
    }
    ec.assign(0, ec.category());
}

template<class Derived>
void
basic_parser<Derived>::
write(char const* it, std::size_t len, error_code& ec)
{
    auto const last = it + len;

    while(it < last)
    {
    loop:
        switch(current_state())
        {
        case state::json:
            replace_state(state::element);
            break;

        case state::element:
            replace_state(state::ws);
            push_state(state::value);
            push_state(state::ws);
            break;

        case state::ws:
            do
            {
                if(is_ws(*it))
                    continue;
                pop_state();
                goto loop;
            }
            while(++it < last);
            break;

        case state::value:
        {
            switch(*it)
            {
            // object
            case '{':
                break;

            // array
            case '[':
                break;

            // string
            case '"':
                break;

            // number
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                break;

            // true
            case 't':
                ++it;
                replace_state(state::true_1);
                break;

            // false
            case 'f':
                ++it;
                replace_state(state::false_1);
                break;

            // null
            case 'n':
                ++it;
                replace_state(state::null_1);
                break;

            default:
                ec = error::syntax;
                break;
            }
            goto loop;
        }

        //
        // true
        //

        case state::true_1:
            if(*it != 'r')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::true_2);
            break;

        case state::true_2:
            if(*it != 'u')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::true_3);
            break;

        case state::true_3:
            if(*it != 'e')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            impl().on_true(ec);
            if(ec)
                return;
            pop_state();
            break;

        //
        // false
        //

        case state::false_1:
            if(*it != 'a')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::false_2);
            break;

        case state::false_2:
            if(*it != 'l')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::false_3);
            break;

        case state::false_3:
            if(*it != 's')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::false_4);
            break;

        case state::false_4:
            if(*it != 'e')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            impl().on_false(ec);
            if(ec)
                return;
            pop_state();
            break;

        //
        // null
        //

        case state::null_1:
            if(*it != 'u')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::null_2);
            break;

        case state::null_2:
            if(*it != 'l')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            replace_state(state::null_3);
            break;

        case state::null_3:
            if(*it != 'l')
            {
                ec = error::syntax;
                return;
            }
            ++it;
            impl().on_null(ec);
            if(ec)
                return;
            pop_state();
            break;

        }
    }
}

//------------------------------------------------------------------------------

template<class Derived>
auto
basic_parser<Derived>::
current_state() const noexcept ->
    state
{
    if(! st_stack_.empty())
        return st_stack_.back();
    return state::end;
}

template<class Derived>
void
basic_parser<Derived>::
push_state(state st)
{
    st_stack_.push_back(st);
}

template<class Derived>
void
basic_parser<Derived>::
pop_state()
{
    BOOST_ASSERT(! st_stack_.empty());
    st_stack_.pop_back();
}

template<class Derived>
void
basic_parser<Derived>::
replace_state(state st)
{
    BOOST_ASSERT(! st_stack_.empty());
    st_stack_.back() = st;
}

} // json
} // beast
} // boost

#endif

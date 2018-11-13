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
write(boost::asio::const_buffer buffer, error_code& ec)
{
    auto p = static_cast<char const*>(buffer.data());
    auto n = buffer.size();
    auto const p0 = p;
    auto const p1 = p0 + n;
    ec.assign(0, ec.category());
    BOOST_ASSERT(current_state() != state::end);
loop:
    switch(current_state())
    {
    case state::json:
        replace_state(state::element);
        goto loop;

    case state::element:
        replace_state(state::ws);
        push_state(state::value);
        push_state(state::ws);
        goto loop;

    case state::ws:
        while(p < p1)
        {
            if(! is_ws(*p))
            {
                pop_state();
                goto loop;
            }
            ++p;
        }
        break;

    case state::value:
    {
        if(p >= p1)
            break;
        switch(*p)
        {
        // object
        case '{':
            ++p;
            replace_state(state::object1);
            impl().on_object_begin(ec);
            if(ec)
                return;
            goto loop;

        // array
        case '[':
            ++p;
            replace_state(state::array_);
            impl().on_array_begin(ec);
            goto loop;

        // string
        case '"':
            ++p;
            replace_state(state::string2);
            goto loop;

        // number
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            // store *p
            ++p;
            replace_state(state::number);
            goto loop;

        // true
        case 't':
            if(p + 4 <= p1)
            {
                if(
                    p[1] != 'r' ||
                    p[2] != 'u' ||
                    p[3] != 'e')
                {
                    ec = error::syntax;
                    return;
                }
                p = p + 4;
                replace_state(state::true4);
                goto loop;
            }
            ++p;
            replace_state(state::true1);
            goto loop;

        // false
        case 'f':
            if(p + 5 <= p1)
            {
                if(
                    p[1] != 'a' ||
                    p[2] != 'l' ||
                    p[3] != 's' ||
                    p[4] != 'e')
                {
                    ec = error::syntax;
                    return;
                }
                p = p + 4;
                replace_state(state::false5);
                goto loop;
            }
            ++p;
            replace_state(state::false1);
            goto loop;

        // null
        case 'n':
            if(p + 4 <= p1)
            {
                if(
                    p[1] != 'u' ||
                    p[2] != 'l' ||
                    p[3] != 'l')
                {
                    ec = error::syntax;
                    return;
                }
                p = p + 4;
                replace_state(state::null4);
                goto loop;
            }
            ++p;
            replace_state(state::null1);
            goto loop;

        default:
            ec = error::syntax;
            return;
        }
        break;
    }

    //
    // object
    //

    case state::object1:
        replace_state(state::object2);
        push_state(state::ws);
        goto loop;

    case state::object2:
        if(p >= p1)
            break;
        if(*p == '}')
        {
            ++p;
            replace_state(state::object4);
            goto loop;
        }
        if(*p != '"')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::object3);
        push_state(state::element);
        push_state(state::colon);
        push_state(state::ws);
        push_state(state::string2);
        goto loop;

    case state::object3:
        if(p >= p1)
            break;
        if(*p == '}')
        {
            ++p;
            replace_state(state::object4);
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::object3);
        push_state(state::element);
        push_state(state::colon);
        push_state(state::ws);
        push_state(state::string1);
        push_state(state::ws);
        goto loop;

    case state::object4:
        impl().on_object_end(ec);
        if(ec)
            return;
        pop_state();
        goto loop;

    case state::colon:
        if(p >= p1)
            break;
        if(*p != ':')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        pop_state();
        goto loop;

    //
    // array
    //

    case state::array_:
        break;

    //
    // string
    //

    case state::string1:
        if(p >= p1)
            break;
        if(*p != '"')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::string2);
        BOOST_FALLTHROUGH;

    case state::string2:
        impl().on_string_begin(ec);
        if(ec)
            return;
        replace_state(state::string3);
        BOOST_FALLTHROUGH;

    case state::string3:
    {
        while(p < p1)
        {
            if(*p == '"')
            {
                impl().on_string_end(ec);
                if(ec)
                    return;
                ++p;
                pop_state();
                goto loop;
            }
            if(is_control(*p))
            {
                ec = error::syntax;
                return;
            }
            ++p;
        }
        break;
    }

    //
    // number
    //

    case state::number:
        break;

    //
    // true
    //

    case state::true1:
        if(p >= p1)
            break;
        if(*p != 'r')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::true2);
        BOOST_FALLTHROUGH;

    case state::true2:
        if(p >= p1)
            break;
        if(*p != 'u')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::true3);
        BOOST_FALLTHROUGH;

    case state::true3:
        if(p >= p1)
            break;
        if(*p != 'e')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::true4);
        BOOST_FALLTHROUGH;

    case state::true4:
        impl().on_true(ec);
        if(ec)
            return;
        pop_state();
        goto loop;

    //
    // false
    //

    case state::false1:
        if(p >= p1)
            break;
        if(*p != 'a')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::false2);
        BOOST_FALLTHROUGH;

    case state::false2:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::false3);
        BOOST_FALLTHROUGH;

    case state::false3:
        if(p >= p1)
            break;
        if(*p != 's')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::false4);
        BOOST_FALLTHROUGH;

    case state::false4:
        if(p >= p1)
            break;
        if(*p != 'e')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::false5);
        BOOST_FALLTHROUGH;

    case state::false5:
        impl().on_false(ec);
        if(ec)
            return;
        pop_state();
        goto loop;

    //
    // null
    //

    case state::null1:
        if(p >= p1)
            break;
        if(*p != 'u')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::null2);
        BOOST_FALLTHROUGH;

    case state::null2:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::null3);
        BOOST_FALLTHROUGH;

    case state::null3:
        if(p >= p1)
            break;
        if(*p != 'l')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::null4);
        BOOST_FALLTHROUGH;

    case state::null4:
        impl().on_null(ec);
        if(ec)
            return;
        pop_state();
        goto loop;

    case state::end:
        break;
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

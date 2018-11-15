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
#include <boost/beast/core/string.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace beast {
namespace json {

/*  References:

    https://www.json.org/

    RFC 7159: The JavaScript Object Notation (JSON) Data Interchange Format
    https://tools.ietf.org/html/rfc7159
*/

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

//------------------------------------------------------------------------------

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
        m_ = 0;
        e_ = 0;
        neg_ = false;
        edir_ = 1;
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
            replace_state(state::array1);
            impl().on_array_begin(ec);
            goto loop;

        // string
        case '"':
            ++p;
            replace_state(state::string2);
            goto loop;

        // number
        case '0':
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
        case '-':
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
                p = p + 5;
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

    case state::array1:
        replace_state(state::array2);
        push_state(state::ws);
        goto loop;

    case state::array2:
        if(p >= p1)
            break;
        if(*p == ']')
        {
            ++p;
            replace_state(state::array4);
            goto loop;
        }
        replace_state(state::array3);
        push_state(state::element);
        goto loop;

    case state::array3:
        if(p >= p1)
            break;
        if(*p == ']')
        {
            ++p;
            replace_state(state::array4);
            goto loop;
        }
        if(*p != ',')
        {
            ec = error::syntax;
            return;
        }
        ++p;
        replace_state(state::array3);
        push_state(state::element);
        push_state(state::ws);
        goto loop;

    case state::array4:
        impl().on_array_end(ec);
        if(ec)
            return;
        pop_state();
        goto loop;

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
        goto loop;

    case state::string2:
        impl().on_string_begin(ec);
        if(ec)
            return;
        replace_state(state::string3);
        goto loop;

    case state::string3:
    {
        auto const first = p;
        while(p < p1)
        {
            if(*p == '"')
            {
                impl().on_string_piece(
                    string_view(first, p - first), ec);
                if(ec)
                    return;
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
        if(p >= p1)
            break;
        if(*p == '-')
        {
            ++p;
            neg_ = true;
        }
        replace_state(state::number_mant1);
        goto loop;

    case state::number_mant1:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa digit
            ec = error::syntax;
            return;
        }
        if(*p != '0')
        {
            replace_state(state::number_mant2);
            goto loop;
        }
        ++p;
        replace_state(state::number_fract1);
        goto loop;

    case state::number_mant2:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                replace_state(state::number_fract1);
                goto loop;
            }
            m_ = (10 * m_) + (*p++ - '0');
        }
        break;

    case state::number_fract1:
        if(p >= p1)
            break;
        if(*p == '.')
        {
            ++p;
            replace_state(state::number_fract2);
            goto loop;
        }
        if(is_digit(*p))
        {
            // unexpected digit after zero
            ec = error::syntax;
            return;
        }
        replace_state(state::number_exp);
        goto loop;

    case state::number_fract2:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected mantissa fraction digit
            ec = error::syntax;
            return;
        }
        replace_state(state::number_fract3);
        goto loop;

    case state::number_fract3:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                replace_state(state::number_exp);
                goto loop;
            }
            m_ = (10 * m_) + (*p++ - '0');
            --e_;
        }
        break;

    case state::number_exp:
        if(p >= p1)
            break;
        if(*p == 'e' || *p == 'E')
        {
            ++p;
            replace_state(state::number_exp_sign);
            goto loop;
        }
        replace_state(state::number_end);
        goto loop;

    case state::number_exp_sign:
        if(p >= p1)
            break;
        if(*p == '+')
        {
            ++p;
        }
        if(*p == '-')
        {
            ++p;
            edir_ = -1;
        }
        replace_state(state::number_exp_digits1);
        goto loop;

    case state::number_exp_digits1:
        if(p >= p1)
            break;
        if(! is_digit(*p))
        {
            // expected exponent digit
            ec = error::syntax;
            return;
        }
        replace_state(state::number_exp_digits2);
        goto loop;

    case state::number_exp_digits2:
        while(p < p1)
        {
            if(! is_digit(*p))
            {
                replace_state(state::number_end);
                goto loop;
            }
            e_ = (10 * e_) + (*p++ - '0');
        }
        break;

    case state::number_end:
        impl().on_number(ec);
        if(ec)
            return;
        m_ = 0;
        e_ = 0;
        neg_ = false;
        edir_ = 1;
        pop_state();
        goto loop;

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
        if(p < p1)
        {
            // unexpected extra characters
            ec = error::syntax;
            return;
        }
        break;
    }
}

template<class Derived>
void
basic_parser<Derived>::
write_eof(error_code& ec)
{
    auto const fail =
        [this, &ec]
        {
            char c = 0;
            write(boost::asio::const_buffer(&c, 1), ec);
            BOOST_ASSERT(ec);
        };

    while(current_state() != state::end)
    {
        // pop all states that
        // allow "" (empty string)
        switch(current_state())
        {
        case state::number_mant2:
        case state::number_fract1:
        case state::number_fract3:
        case state::number_exp:
        case state::number_exp_digits2:
            replace_state(state::number_end);
            write(boost::asio::const_buffer(), ec);
            if(ec)
                return;
            break;

        case state::ws:
            pop_state();
            break;

        default:
            return fail();
        }
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

//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_JSON_BASIC_PARSER_HPP
#define BOOST_BEAST_JSON_BASIC_PARSER_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/buffer.hpp>

#include <vector>

namespace boost {
namespace beast {
namespace json {

namespace detail {

struct parser_base
{
    static
    bool
    is_ws(char c) noexcept
    {
        return
            c == ' '  || c == '\t' ||
            c == '\r' || c == '\n';
    }

    static
    bool
    is_digit(char c) noexcept
    {
        return static_cast<unsigned char>(c-'0') < 10;
    }

    static
    bool
    is_control(char c)
    {
        return static_cast<unsigned char>(c) < 32;
    }
};

} // detail

template<class Derived>
class basic_parser : private detail::parser_base
{
protected:
    basic_parser();

public:
    template<class ConstBufferSequence>
    void
    write(ConstBufferSequence const& buffers, error_code& ec);

    void
    write(boost::asio::const_buffer buffer, error_code& ec);

    void
    write_eof(error_code& ec);

private:
    enum class state
    {
        json,
        element,
        ws,
        value,

        object1, object2, object3, object4, colon,

        array_,
        string1, string2, string3,
        number,
        true1,   true2,  true3,  true4,
        false1, false2, false3, false4, false5,
        null1,   null2,  null3,  null4,

        end
    };

    Derived&
    impl()
    {
        return *static_cast<Derived*>(this);
    }

    state current_state() const noexcept;
    void push_state(state st);
    void pop_state();
    void replace_state(state st);

    std::vector<state> st_stack_;

};

} // json
} // beast
} // boost

#include <boost/beast/_experimental/json/impl/basic_parser.hpp>

#endif

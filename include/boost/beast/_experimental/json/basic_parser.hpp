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
    is_ws(char c) noexcept;

    static
    bool
    is_digit(char c) noexcept;
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
    write(boost::asio::const_buffer buffer, error_code& ec)
    {
        write(static_cast<char const*>(
            buffer.data()), buffer.size(), ec);
    }

    void
    write_eof(error_code& ec);

private:
    enum class state
    {
        json,
        element,
        ws,
        value,
        object,
        array_,
        string,
        number,
        true_1,  true_2,  true_3,
        false_1, false_2, false_3, false_4,
        null_1,  null_2,  null_3,
        end
    };

    Derived&
    impl()
    {
        return *static_cast<Derived*>(this);
    }

    void
    write(char const* it, std::size_t len, error_code& ec);

    state current_state() const noexcept;
    void push_state(state st);
    void pop_state();
    void set_state(state st);

    std::vector<state> st_stack_;

};

} // json
} // beast
} // boost

#include <boost/beast/_experimental/json/impl/basic_parser.hpp>

#endif

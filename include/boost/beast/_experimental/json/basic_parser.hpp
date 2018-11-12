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

namespace boost {
namespace beast {
namespace json {

template<class Derived>
class basic_parser
{
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
        value
    };

    Derived&
    impl()
    {
        return *static_cast<Derived*>(this);
    }

    void
    write(char const* it, std::size_t len, error_code& ec);

    state st_;
};

} // json
} // beast
} // boost

#include <boost/beast/_experimental/json/impl/basic_parser.hpp>

#endif

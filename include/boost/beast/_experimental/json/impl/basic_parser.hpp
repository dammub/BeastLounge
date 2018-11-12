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

#include <boost/beast/core/buffers_range.hpp>

namespace boost {
namespace beast {
namespace json {

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
}

template<class Derived>
void
basic_parser<Derived>::
write(char const* it, std::size_t len, error_code& ec)
{
    auto const last = it + len;

    switch(state_)
    {
    case state::json:
        push_state(state::);
    }
}

} // json
} // beast
} // boost

#endif

//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_JSON_ERROR_HPP
#define BOOST_BEAST_JSON_ERROR_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/_experimental/json/detail/error.hpp>
#include <boost/beast/core/error.hpp>

namespace boost {
namespace beast {
namespace json {

/// Error codes returned by JSON operations
enum class error
{
    syntax = 1
};

/// Error conditions corresponding to JSON errors
enum class condition
{
    parse_error = 1
};

} // websocket
} // beast
} // boost

#include <boost/beast/_experimental/json/impl/error.ipp>

#endif

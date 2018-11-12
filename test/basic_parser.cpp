//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/_experimental/json/basic_parser.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include <boost/beast/core/buffers_range.hpp>
#include <boost/beast/core/error.hpp>

namespace boost {
namespace beast {
namespace json {

class basic_parser_test : public beast::unit_test::suite
{
public:
    void run() override
    {
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,basic_parser);

} // json
} // beast
} // boost

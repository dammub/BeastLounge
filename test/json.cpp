//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
//#include <>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include <boost/beast/core/error.hpp>

namespace boost {
namespace beast {

namespace json {

template<class Derived>
class basic_parser
{
    Derived&
    impl()
    {
        return *static_cast<Derived*>(this);
    }

public:
    template<class ConstBufferSequence>
    void
    write(ConstBufferSequence const& buffers, error_code& ec);
};

template<class Derived>
template<class ConstBufferSequence>
void
basic_parser<Derived>::
write(ConstBufferSequence const& buffers, error_code& ec)
{
}

} // json

//------------------------------------------------------------------------------

class json_test : public beast::unit_test::suite
{
public:
    void run() override
    {
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(beast,core,json);

} // beast
} // boost

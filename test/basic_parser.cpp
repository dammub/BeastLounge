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
#include <boost/beast/core/string.hpp>
#include <boost/asio/buffer.hpp>

namespace boost {
namespace beast {
namespace json {

class basic_parser_test : public beast::unit_test::suite
{
public:
    class test_parser
        : public basic_parser<test_parser>
    {
        friend class basic_parser<test_parser>;

    public:
        test_parser() = default;

        void
        on_object_begin(error_code&)
        {
        }

        void
        on_object_end(error_code&)
        {
        }

        void
        on_array_begin(error_code&)
        {
        }

        void
        on_array_end(error_code&)
        {
        }

        void
        on_string_begin(error_code&)
        {
        }

        void
        on_string_end(error_code&)
        {
        }

        void
        on_true(error_code&)
        {
        }

        void
        on_false(error_code&)
        {
        }

        void
        on_null(error_code&)
        {
        }
    };

    void
    good(string_view s)
    {
        error_code ec;
        test_parser p;
        p.write(boost::asio::const_buffer(
            s.data(), s.size()), ec);
        if(BEAST_EXPECT(! ec))
        {
            p.write_eof(ec);
            if(BEAST_EXPECT(! ec))
                return;
        }
        log << "fail: \"" << s << "\"\n";
    }

    void
    bad(string_view s)
    {
        error_code ec;
        test_parser p;
        p.write(boost::asio::const_buffer(
            s.data(), s.size()), ec);
        if(! ec)
        {        
            p.write_eof(ec);
            if(BEAST_EXPECT(ec))
                return;
        }
        else
        {
            pass();
            return;
        }
        log << "fail: \"" << s << "\"\n";
    }

    void
    testString()
    {
        good(R"_("")_");
        good(R"_("x")_");
        good(R"_("xy")_");
        good(R"_("x y")_");

        bad (R"_("\t")_");

    }

    void
    testParse()
    {
        good("{}");
        good("{ }");
        good("{ \t }");
        good("{ \"x\" : null }");
        bad ("{{}}");

        good("true");
        good(" true");
        good("true ");
        good("\ttrue");
        good("true\t");
        good("\r\n\t true\r\n\t ");
        bad ("truu");
        bad ("tu");
        bad ("t");

        good("false");
        bad ("fals");
        bad ("fel");
        bad ("f");

        good("null");
        bad ("nul");
        bad ("no");
        bad ("n");
    }

    void run() override
    {
        testString();
        testParse();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,basic_parser);

} // json
} // beast
} // boost

#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

template <std::size_t I = 0,
          typename DpfKey,
          typename ConcreteT>
void assign_leaf(DpfKey & dpf0, DpfKey & dpf1, const ConcreteT y_shr0, const ConcreteT y_shr1)
{
    asio::io_context io_context;
    std::thread server([&io_context, &dpf0, &y_shr0]()
    {
        asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 31337));
        asio::ip::tcp::socket peer{io_context};
        asio::ip::tcp::endpoint ep{};
        acceptor.accept(peer, ep);
        auto future = dpf0.template async_assign_leaf<I>(peer, y_shr0, asio::use_future);
        io_context.run();
        future.wait();
    });
    std::thread client([&io_context, &dpf1, &y_shr1]()
    {
        asio::ip::tcp::socket peer(io_context);
        asio::ip::tcp::resolver resolver(io_context);
        asio::connect(peer, resolver.resolve("localhost", "31337"));
        auto future = dpf1.template async_assign_leaf<I>(peer, y_shr1, asio::use_future);
        io_context.run();
        future.wait();
    });
    client.join();
    server.join();
}

TEST(WildcardTest, SingleLeafFailEvalBeforeVernalization)
{
    using input_type = uint8_t;
    using output_type = dpf::wildcard_value<uint32_t>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    input_type x = 0xAA;
    output_type y;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);

    ASSERT_THROW(dpf::eval_point(dpf0, x), std::runtime_error);
    ASSERT_THROW(dpf::eval_point(dpf1, x), std::runtime_error);

    input_type from = 0x33, to = 0xCC;
    ASSERT_THROW(dpf::eval_interval(dpf0, from, to), std::runtime_error);
    ASSERT_THROW(dpf::eval_interval(dpf1, from, to), std::runtime_error);

    ASSERT_THROW(dpf::eval_full(dpf0), std::runtime_error);
    ASSERT_THROW(dpf::eval_full(dpf1), std::runtime_error);

    std::array<input_type, 16> points{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ASSERT_THROW(dpf::eval_sequence(dpf0, std::begin(points), std::end(points)), std::runtime_error);
    ASSERT_THROW(dpf::eval_sequence(dpf1, std::begin(points), std::end(points)), std::runtime_error);

    ASSERT_THROW(dpf::eval_sequence_breadth_first(dpf0, std::begin(points), std::end(points)), std::runtime_error);
    ASSERT_THROW(dpf::eval_sequence_breadth_first(dpf1, std::begin(points), std::end(points)), std::runtime_error);

    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(points), std::end(points)),
         recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(points), std::end(points));
    ASSERT_THROW(dpf::eval_sequence(dpf0, recipe0), std::runtime_error);
    ASSERT_THROW(dpf::eval_sequence(dpf1, recipe1), std::runtime_error);
}

TEST(WildcardTest, MultiLeafFailEvalBeforeVernalization)
{
    using input_type = uint8_t;
    using output_type0 = uint32_t;
    using output_type1 = dpf::wildcard_value<uint32_t>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type0, output_type1>;
    input_type x = 0xAA;
    output_type0 y0 = 0xAAAAAAAA;
    output_type1 y1;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1);

    ASSERT_NO_THROW((dpf::eval_point<0>(dpf0, x)));
    ASSERT_NO_THROW((dpf::eval_point<0>(dpf1, x)));
    ASSERT_THROW((dpf::eval_point<1>(dpf0, x)), std::runtime_error);
    ASSERT_THROW((dpf::eval_point<1>(dpf1, x)), std::runtime_error);
    ASSERT_THROW((dpf::eval_point<0, 1>(dpf0, x)), std::runtime_error);
    ASSERT_THROW((dpf::eval_point<0, 1>(dpf1, x)), std::runtime_error);

    input_type from = 0x33, to = 0xCC;
    ASSERT_NO_THROW((dpf::eval_interval<0>(dpf0, from, to)));
    ASSERT_NO_THROW((dpf::eval_interval<0>(dpf1, from, to)));
    ASSERT_THROW((dpf::eval_interval<1>(dpf0, from, to)), std::runtime_error);
    ASSERT_THROW((dpf::eval_interval<1>(dpf1, from, to)), std::runtime_error);
    ASSERT_THROW((dpf::eval_interval<0, 1>(dpf0, from, to)), std::runtime_error);
    ASSERT_THROW((dpf::eval_interval<0, 1>(dpf1, from, to)), std::runtime_error);

    ASSERT_NO_THROW((dpf::eval_full<0>(dpf0)));
    ASSERT_NO_THROW((dpf::eval_full<0>(dpf1)));
    ASSERT_THROW((dpf::eval_full<1>(dpf0)), std::runtime_error);
    ASSERT_THROW((dpf::eval_full<1>(dpf1)), std::runtime_error);
    ASSERT_THROW((dpf::eval_full<0, 1>(dpf0)), std::runtime_error);
    ASSERT_THROW((dpf::eval_full<0, 1>(dpf1)), std::runtime_error);

    std::array<input_type, 16> points{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ASSERT_NO_THROW((dpf::eval_sequence<0>(dpf0, std::begin(points), std::end(points))));
    ASSERT_NO_THROW((dpf::eval_sequence<0>(dpf1, std::begin(points), std::end(points))));
    ASSERT_THROW((dpf::eval_sequence<1>(dpf0, std::begin(points), std::end(points))), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<1>(dpf1, std::begin(points), std::end(points))), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<0, 1>(dpf0, std::begin(points), std::end(points))), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<0, 1>(dpf1, std::begin(points), std::end(points))), std::runtime_error);

    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(points), std::end(points)),
         recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(points), std::end(points));
    ASSERT_NO_THROW((dpf::eval_sequence<0>(dpf0, recipe0)));
    ASSERT_NO_THROW((dpf::eval_sequence<0>(dpf1, recipe1)));
    ASSERT_THROW((dpf::eval_sequence<1>(dpf0, recipe0)), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<1>(dpf1, recipe1)), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<0, 1>(dpf0, recipe0)), std::runtime_error);
    ASSERT_THROW((dpf::eval_sequence<0, 1>(dpf1, recipe1)), std::runtime_error);
}

TEST(WildcardTest, SingleLeafSuccess)
{
    using input_type = uint8_t;
    using concrete_type = uint32_t;
    using output_type = dpf::wildcard_value<concrete_type>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    input_type x = 0xAA, from = 0x33, to = 0xCC;
    output_type y;
    concrete_type y_exp = 0xAAAAAAAA,
                  y_shr0 = 0x12345678,
                  y_shr1 = y_exp - y_shr0,
                  zero_output = 0;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);

    assign_leaf(dpf0, dpf1, y_shr0, y_shr1);

    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i)
    {
        concrete_type y0 = dpf::eval_point(dpf0, i),
                      y1 = dpf::eval_point(dpf1, i);
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(y1 - y0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(y1 - y0), zero_output);
        }
    }

    auto [bufint0, iterint0] = dpf::eval_interval(dpf0, from, to);
    auto [bufint1, iterint1] = dpf::eval_interval(dpf1, from, to);
    auto itint0 = std::begin(iterint0),
         itint1 = std::begin(iterint1);
    for (std::size_t i = from; i <= to; ++i, ++itint0, ++itint1)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint1 - *itint0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint1 - *itint0), zero_output);
        }
    }
    ASSERT_EQ(itint0, std::end(iterint0));
    ASSERT_EQ(itint1, std::end(iterint1));

    auto [bufful0, iterful0] = dpf::eval_full(dpf0);
    auto [bufful1, iterful1] = dpf::eval_full(dpf1);
    auto itful0 = std::begin(iterful0),
         itful1 = std::begin(iterful1);
    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i, ++itful0, ++itful1)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful1 - *itful0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful1 - *itful0), zero_output);
        }
    }
    ASSERT_EQ(itful0, std::end(iterful0));
    ASSERT_EQ(itful1, std::end(iterful1));

    std::array<input_type, 16> points{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    auto [bufseq0, iterseq0] = dpf::eval_sequence(dpf0, std::begin(points), std::end(points));
    auto [bufseq1, iterseq1] = dpf::eval_sequence(dpf1, std::begin(points), std::end(points));
    auto itseq0 = std::begin(iterseq0),
         itseq1 = std::begin(iterseq1);
    for (std::size_t i = 0; i < points.size(); ++i, ++itseq0, ++itseq1)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq1 - *itseq0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq1 - *itseq0), zero_output);
        }
    }
    ASSERT_EQ(itseq0, std::end(iterseq0));
    ASSERT_EQ(itseq1, std::end(iterseq1));

    auto [bufbre0, iterbre0] = dpf::eval_sequence_breadth_first(dpf0, std::begin(points), std::end(points));
    auto [bufbre1, iterbre1] = dpf::eval_sequence_breadth_first(dpf1, std::begin(points), std::end(points));
    auto itbre0 = std::begin(iterbre0),
         itbre1 = std::begin(iterbre1);
    for (std::size_t i = 0; i < points.size(); ++i, ++itbre0, ++itbre1)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre1 - *itbre0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre1 - *itbre0), zero_output);
        }
    }
    ASSERT_EQ(itbre0, std::end(iterbre0));
    ASSERT_EQ(itbre1, std::end(iterbre1));

    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(points), std::end(points)),
         recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(points), std::end(points));
    auto [bufrec0, iterrec0] = dpf::eval_sequence(dpf0, recipe0);
    auto [bufrec1, iterrec1] = dpf::eval_sequence(dpf1, recipe1);
    auto itrec0 = std::begin(iterrec0),
         itrec1 = std::begin(iterrec1);
    for (std::size_t i = 0; i < points.size(); ++i, ++itrec0, ++itrec1)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec1 - *itrec0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec1 - *itrec0), zero_output);
        }
    }
    ASSERT_EQ(itrec0, std::end(iterrec0));
    ASSERT_EQ(itrec1, std::end(iterrec1));
}

TEST(WildcardTest, MultiLeafSuccess)
{
    using input_type = uint8_t;
    using output_type0 = uint32_t;
    using concrete_type = uint32_t;
    using output_type1 = dpf::wildcard_value<concrete_type>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type0, output_type1>;
    input_type x = 0xAA, from = 0x33, to = 0xCC;
    output_type0 y0 = 0x55555555,
                 zero_output0 = 0;
    output_type1 y1;
    concrete_type y_exp = 0xAAAAAAAA,
                  y_shr0 = 0x12345678,
                  y_shr1 = y_exp - y_shr0,
                  zero_output1 = 0;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1);

    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i)
    {
        concrete_type ypoi0 = dpf::eval_point<0>(dpf0, i),
                      ypoi1 = dpf::eval_point<0>(dpf1, i);
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(ypoi1 - ypoi0), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(ypoi1 - ypoi0), zero_output0);
        }
    }

    auto [bufint00, iterint00] = dpf::eval_interval<0>(dpf0, from, to);
    auto [bufint01, iterint01] = dpf::eval_interval<0>(dpf1, from, to);
    auto itint00 = std::begin(iterint00),
         itint01 = std::begin(iterint01);
    for (std::size_t i = from; i <= to; ++i, ++itint00, ++itint01)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint01 - *itint00), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint01 - *itint00), zero_output0);
        }
    }
    ASSERT_EQ(itint00, std::end(iterint00));
    ASSERT_EQ(itint01, std::end(iterint01));

    auto [bufful00, iterful00] = dpf::eval_full<0>(dpf0);
    auto [bufful01, iterful01] = dpf::eval_full<0>(dpf1);
    auto itful00 = std::begin(iterful00),
         itful01 = std::begin(iterful01);
    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i, ++itful00, ++itful01)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful01 - *itful00), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful01 - *itful00), zero_output0);
        }
    }
    ASSERT_EQ(itful00, std::end(iterful00));
    ASSERT_EQ(itful01, std::end(iterful01));

    std::array<input_type, 16> points{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                      0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    auto [bufseq00, iterseq00] = dpf::eval_sequence<0>(dpf0, std::begin(points), std::end(points));
    auto [bufseq01, iterseq01] = dpf::eval_sequence<0>(dpf1, std::begin(points), std::end(points));
    auto itseq00 = std::begin(iterseq00),
         itseq01 = std::begin(iterseq01);
    for (std::size_t i = 0; i < points.size(); ++i, ++itseq00, ++itseq01)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq01 - *itseq00), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq01 - *itseq00), zero_output0);
        }
    }
    ASSERT_EQ(itseq00, std::end(iterseq00));
    ASSERT_EQ(itseq01, std::end(iterseq01));

    auto [bufbre00, iterbre00] = dpf::eval_sequence_breadth_first<0>(dpf0, std::begin(points), std::end(points));
    auto [bufbre01, iterbre01] = dpf::eval_sequence_breadth_first<0>(dpf1, std::begin(points), std::end(points));
    auto itbre00 = std::begin(iterbre00),
         itbre01 = std::begin(iterbre01);
    for (std::size_t i = 0; i < points.size(); ++i, ++itbre00, ++itbre01)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre01 - *itbre00), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre01 - *itbre00), zero_output0);
        }
    }
    ASSERT_EQ(itbre00, std::end(iterbre00));
    ASSERT_EQ(itbre01, std::end(iterbre01));

    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(points), std::end(points)),
         recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(points), std::end(points));
    auto [bufrec00, iterrec00] = dpf::eval_sequence<0>(dpf0, recipe0);
    auto [bufrec01, iterrec01] = dpf::eval_sequence<0>(dpf1, recipe1);
    auto itrec00 = std::begin(iterrec00),
         itrec01 = std::begin(iterrec01);
    for (std::size_t i = 0; i < points.size(); ++i, ++itrec00, ++itrec01)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec01 - *itrec00), y0);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec01 - *itrec00), zero_output0);
        }
    }
    ASSERT_EQ(itrec00, std::end(iterrec00));
    ASSERT_EQ(itrec01, std::end(iterrec01));

    assign_leaf<1>(dpf0, dpf1, y_shr0, y_shr1);

    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i)
    {
        concrete_type ypoi0 = dpf::eval_point<1>(dpf0, i),
                      ypoi1 = dpf::eval_point<1>(dpf1, i);
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(ypoi1 - ypoi0), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(ypoi1 - ypoi0), zero_output1);
        }
    }

    auto [bufint10, iterint10] = dpf::eval_interval<1>(dpf0, from, to);
    auto [bufint11, iterint11] = dpf::eval_interval<1>(dpf1, from, to);
    auto itint10 = std::begin(iterint10),
         itint11 = std::begin(iterint11);
    for (std::size_t i = from; i <= to; ++i, ++itint10, ++itint11)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint11 - *itint10), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itint11 - *itint10), zero_output1);
        }
    }
    ASSERT_EQ(itint10, std::end(iterint10));
    ASSERT_EQ(itint11, std::end(iterint11));

    auto [bufful10, iterful10] = dpf::eval_full<1>(dpf0);
    auto [bufful11, iterful11] = dpf::eval_full<1>(dpf1);
    auto itful10 = std::begin(iterful10),
         itful11 = std::begin(iterful11);
    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i, ++itful10, ++itful11)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful11 - *itful10), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itful11 - *itful10), zero_output1);
        }
    }
    ASSERT_EQ(itful10, std::end(iterful10));
    ASSERT_EQ(itful11, std::end(iterful11));

    auto [bufseq10, iterseq10] = dpf::eval_sequence<1>(dpf0, std::begin(points), std::end(points));
    auto [bufseq11, iterseq11] = dpf::eval_sequence<1>(dpf1, std::begin(points), std::end(points));
    auto itseq10 = std::begin(iterseq10),
         itseq11 = std::begin(iterseq11);
    for (std::size_t i = 0; i < points.size(); ++i, ++itseq10, ++itseq11)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq11 - *itseq10), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itseq11 - *itseq10), zero_output1);
        }
    }
    ASSERT_EQ(itseq10, std::end(iterseq10));
    ASSERT_EQ(itseq11, std::end(iterseq11));

    auto [bufbre10, iterbre10] = dpf::eval_sequence_breadth_first<1>(dpf0, std::begin(points), std::end(points));
    auto [bufbre11, iterbre11] = dpf::eval_sequence_breadth_first<1>(dpf1, std::begin(points), std::end(points));
    auto itbre10 = std::begin(iterbre10),
         itbre11 = std::begin(iterbre11);
    for (std::size_t i = 0; i < points.size(); ++i, ++itbre10, ++itbre11)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre11 - *itbre10), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itbre11 - *itbre10), zero_output1);
        }
    }
    ASSERT_EQ(itbre10, std::end(iterbre10));
    ASSERT_EQ(itbre11, std::end(iterbre11));

    auto [bufrec10, iterrec10] = dpf::eval_sequence<1>(dpf0, recipe0);
    auto [bufrec11, iterrec11] = dpf::eval_sequence<1>(dpf1, recipe1);
    auto itrec10 = std::begin(iterrec10),
         itrec11 = std::begin(iterrec11);
    for (std::size_t i = 0; i < points.size(); ++i, ++itrec10, ++itrec11)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec11 - *itrec10), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<concrete_type>(*itrec11 - *itrec10), zero_output1);
        }
    }
    ASSERT_EQ(itrec10, std::end(iterrec10));
    ASSERT_EQ(itrec11, std::end(iterrec11));

    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i)
    {
        auto ypoi0 = dpf::eval_point<0, 1>(dpf0, i),
             ypoi1 = dpf::eval_point<0, 1>(dpf1, i);
        if (i == x)
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(ypoi1) - std::get<0>(ypoi0)), y0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(ypoi1) - std::get<1>(ypoi0)), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(ypoi1) - std::get<0>(ypoi0)), zero_output0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(ypoi1) - std::get<1>(ypoi0)), zero_output1);
        }
    }

    auto [bufintmul0, iterintmul0] = dpf::eval_interval<0, 1>(dpf0, from, to);
    auto [bufintmul1, iterintmul1] = dpf::eval_interval<0, 1>(dpf1, from, to);
    auto zipint0 = dpf::tuple_as_zip(iterintmul0),
         zipint1 = dpf::tuple_as_zip(iterintmul1);
    auto itintmul0 = std::begin(zipint0),
         itintmul1 = std::begin(zipint1);
    for (std::size_t i = from; i <= to; ++i, ++itintmul0, ++itintmul1)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itintmul1) - std::get<0>(*itintmul0)), y0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itintmul1) - std::get<1>(*itintmul0)), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itintmul1) - std::get<0>(*itintmul0)), zero_output0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itintmul1) - std::get<1>(*itintmul0)), zero_output1);
        }
    }
    ASSERT_EQ(itintmul0, std::end(zipint0));
    ASSERT_EQ(itintmul1, std::end(zipint1));

    auto [buffulmul0, iterfulmul0] = dpf::eval_full<0, 1>(dpf0);
    auto [buffulmul1, iterfulmul1] = dpf::eval_full<0, 1>(dpf1);
    auto zipful0 = dpf::tuple_as_zip(iterfulmul0),
         zipful1 = dpf::tuple_as_zip(iterfulmul1);
    auto itfulmul0 = std::begin(zipful0),
         itfulmul1 = std::begin(zipful1);
    for (std::size_t i = std::numeric_limits<input_type>::min(); i <= std::numeric_limits<input_type>::max(); ++i, ++itfulmul0, ++itfulmul1)
    {
        if (i == x)
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itfulmul1) - std::get<0>(*itfulmul0)), y0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itfulmul1) - std::get<1>(*itfulmul0)), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itfulmul1) - std::get<0>(*itfulmul0)), zero_output0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itfulmul1) - std::get<1>(*itfulmul0)), zero_output1);
        }
    }
    ASSERT_EQ(itfulmul0, std::end(zipful0));
    ASSERT_EQ(itfulmul1, std::end(zipful1));

    auto [bufseqmul0, iterseqmul0] = dpf::eval_sequence<0, 1>(dpf0, std::begin(points), std::end(points));
    auto [bufseqmul1, iterseqmul1] = dpf::eval_sequence<0, 1>(dpf1, std::begin(points), std::end(points));
    auto zipseq0 = dpf::tuple_as_zip(iterseqmul0),
         zipseq1 = dpf::tuple_as_zip(iterseqmul1);
    auto itseqmul0 = std::begin(zipseq0),
         itseqmul1 = std::begin(zipseq1);
    for (std::size_t i = 0; i < points.size(); ++i, ++itseqmul0, ++itseqmul1)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itseqmul1) - std::get<0>(*itseqmul0)), y0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itseqmul1) - std::get<1>(*itseqmul0)), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itseqmul1) - std::get<0>(*itseqmul0)), zero_output0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itseqmul1) - std::get<1>(*itseqmul0)), zero_output1);
        }
    }
    ASSERT_EQ(itseqmul0, std::end(zipseq0));
    ASSERT_EQ(itseqmul1, std::end(zipseq1));

    auto [bufrecmul0, iterrecmul0] = dpf::eval_sequence<0, 1>(dpf0, recipe0);
    auto [bufrecmul1, iterrecmul1] = dpf::eval_sequence<0, 1>(dpf1, recipe1);
    auto ziprec0 = dpf::tuple_as_zip(iterrecmul0),
         ziprec1 = dpf::tuple_as_zip(iterrecmul1);
    auto itrecmul0 = std::begin(ziprec0),
         itrecmul1 = std::begin(ziprec1);
    for (std::size_t i = 0; i < points.size(); ++i, ++itrecmul0, ++itrecmul1)
    {
        if (points[i] == x)
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itrecmul1) - std::get<0>(*itrecmul0)), y0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itrecmul1) - std::get<1>(*itrecmul0)), y_exp);
        }
        else
        {
            ASSERT_EQ(static_cast<output_type0>(std::get<0>(*itrecmul1) - std::get<0>(*itrecmul0)), zero_output0);
            ASSERT_EQ(static_cast<concrete_type>(std::get<1>(*itrecmul1) - std::get<1>(*itrecmul0)), zero_output1);
        }
    }
    ASSERT_EQ(itrecmul0, std::end(ziprec0));
    ASSERT_EQ(itrecmul1, std::end(ziprec1));
}

#include <boost/test/unit_test.hpp>

#include "../PlanetaMatchMakerServer/source/utilities/fixed_string.hpp"

BOOST_AUTO_TEST_SUITE(fixed_string_test)
	BOOST_AUTO_TEST_CASE(test_c_str_construct_alphabet) {
		const auto test = u8"ABCXYZ";
		BOOST_CHECK_EQUAL(std::string(test), pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_c_str_construct_japanese) {
		const auto test = u8"‚ ‚¢‚¤‚¦‚¨ŠI";
		BOOST_CHECK_EQUAL(std::string(test), pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_c_str_construct_mixed) {
		const auto test = u8"‚ ‚¢‚¤ABC‚¦‚¨ŠI";
		BOOST_CHECK_EQUAL(std::string(test), pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_std_string_construct_alphabet) {
		const std::string test = u8"ABCXYZ";
		BOOST_CHECK_EQUAL(test, pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_std_string_construct_japanese) {
		const std::string test = u8"‚ ‚¢‚¤‚¦‚¨ŠI";
		BOOST_CHECK_EQUAL(test, pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_std_string_construct_mixed) {
		const std::string test = u8"‚ ‚¢‚¤ABC‚¦‚¨ŠI";
		BOOST_CHECK_EQUAL(test, pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_construct_empty) {
		const std::string test{};
		BOOST_CHECK_EQUAL(test, pgl::fixed_string<32>(test).to_string());
	}

	BOOST_AUTO_TEST_CASE(test_construct_bound_length_string) {
		pgl::fixed_string<15>(u8"‚ ‚ ‚ ‚ ‚ ");
	}

	BOOST_AUTO_TEST_CASE(test_construct_too_long_string) {
		BOOST_CHECK_EXCEPTION(pgl::fixed_string<32>(u8"‚ ‚ ‚ ‚ ‚ ‚ ‚ ‚ ‚ ‚ ‚ "), std::out_of_range, [](auto) {return true; });
	}

	BOOST_AUTO_TEST_CASE(test_copy_construct) {
		const std::string test = u8"‚©‚«‚­‚¯‚±";
		const pgl::fixed_string<32> original(test);
		const auto copied(original);
		BOOST_CHECK_EQUAL(test, original.to_string());
		BOOST_CHECK_EQUAL(test, copied.to_string());
	}

	BOOST_AUTO_TEST_CASE(test_copy_operator) {
		const std::string test = u8"‚©‚«‚­‚¯‚±";
		const pgl::fixed_string<32> original(test);
		pgl::fixed_string<32> copied;
		copied = original;
		BOOST_CHECK_EQUAL(test, original.to_string());
		BOOST_CHECK_EQUAL(test, copied.to_string());
	}

	BOOST_AUTO_TEST_CASE(test_indexer_start) {
		const pgl::fixed_string<4> test(u8"abcd");
		BOOST_CHECK_EQUAL(test[0], u8'a');
	}

	BOOST_AUTO_TEST_CASE(test_indexer_middle) {
		const pgl::fixed_string<4> test(u8"abcd");
		BOOST_CHECK_EQUAL(test[1], u8'b');
	}

	BOOST_AUTO_TEST_CASE(test_indexer_end) {
		const pgl::fixed_string<4> test(u8"abcd");
		BOOST_CHECK_EQUAL(test[3], u8'd');
	}

	BOOST_AUTO_TEST_CASE(test_at_out_of_range) {
		const pgl::fixed_string<4> test(u8"abcd");
		BOOST_CHECK_EXCEPTION(test.at(4), std::out_of_range, [](auto) {return true; });
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_true) {
		const pgl::fixed_string<4> small(u8"abcd");
		const pgl::fixed_string<4> big(u8"abda");
		BOOST_CHECK(small < big);
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_false) {
		const pgl::fixed_string<4> small(u8"abcd");
		const pgl::fixed_string<4> big(u8"abda");
		BOOST_CHECK(!(big < small));
	}

	BOOST_AUTO_TEST_CASE(test_less_than_operator_same) {
		const pgl::fixed_string<4> test1(u8"abcd");
		const pgl::fixed_string<4> test2(u8"abcd");
		BOOST_CHECK(!(test1 < test2));
	}

	BOOST_AUTO_TEST_CASE(test_equal_operator_true) {
		const pgl::fixed_string<4> test1(u8"abcd");
		const pgl::fixed_string<4> test2(u8"abcd");
		BOOST_CHECK(test1 == test2);
	}

	BOOST_AUTO_TEST_CASE(test_equal_operator_false) {
		const pgl::fixed_string<4> test1(u8"abcd");
		const pgl::fixed_string<4> test2(u8"abdd");
		BOOST_CHECK(!(test1 == test2));
	}

	BOOST_AUTO_TEST_CASE(test_size_zero) {
		const pgl::fixed_string<32> test1{};
		BOOST_CHECK_EQUAL(0, test1.size());
	}

	BOOST_AUTO_TEST_CASE(test_size_alphabet) {
		const pgl::fixed_string<32> test1(u8"abcdefg");
		BOOST_CHECK_EQUAL(7, test1.size());
	}

	BOOST_AUTO_TEST_CASE(test_size_alphabet_bound) {
		const pgl::fixed_string<7> test1(u8"abcdefg");
		BOOST_CHECK_EQUAL(7, test1.size());
	}

	BOOST_AUTO_TEST_CASE(test_size_japanese) {
		const pgl::fixed_string<32> test1(u8"‚³‚µ‚·‚¹‚»");
		BOOST_CHECK_EQUAL(15, test1.size());
	}

	BOOST_AUTO_TEST_CASE(test_size_japanese_bound) {
		const pgl::fixed_string<15> test1(u8"‚³‚µ‚·‚¹‚»");
		BOOST_CHECK_EQUAL(15, test1.size());
	}


BOOST_AUTO_TEST_SUITE_END()

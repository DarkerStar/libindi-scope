/*****************************************************************************
 *
 * This file is part of libindi-scope.
 *
 * libindi-scope is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libindi-scope is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libindi-scope.  If not, see <https://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#define BOOST_TEST_MODULE scope_exit
#ifdef BOOST_TEST_DYN_LINK
#	include <boost/test/unit_test.hpp>
#else
#	include <boost/test/included/unit_test.hpp>
#endif // BOOST_TEST_DYN_LINK

#include <indi/scope.hpp>

namespace indi_test {

// Test exception type.
//
// Used to have a unique type for exceptions that can't possibly be thrown
// by non-test code.
struct exception {};

} // namespace indi_test

/*****************************************************************************
 * Basic operation tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(basic_operation_CASE_success)
{
	auto call_count = 0;
	auto func = [&call_count] { ++call_count; };

	{
		auto scope_guard = indi::scope_exit{func};

		// func should not be called before the scope exits
		BOOST_TEST(call_count == 0);
	}

	// func should have been called when the scope exits
	BOOST_TEST(call_count == 1);
}

BOOST_AUTO_TEST_CASE(basic_operation_CASE_fail)
{
	auto call_count = 0;
	auto func = [&call_count] { ++call_count; };

	try
	{
		auto scope_guard = indi::scope_exit{func};

		// func should not be called before the scope exits
		BOOST_TEST(call_count == 0);

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		// func should have been called when the scope exits
		BOOST_TEST(call_count == 1);
	}

	BOOST_TEST(call_count == 1);
}

/*****************************************************************************
 * Release operation tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(release_operation_CASE_success)
{
	auto call_count = 0;
	auto func = [&call_count] { ++call_count; };

	{
		auto scope_guard = indi::scope_exit{func};

		// func should not be called when initializing
		BOOST_TEST(call_count == 0);

		scope_guard.release();

		// func should not be called when released
		BOOST_TEST(call_count == 0);
	}

	// func should not be called when released
	BOOST_TEST(call_count == 0);
}

BOOST_AUTO_TEST_CASE(release_operation_CASE_fail)
{
	auto call_count = 0;
	auto func = [&call_count] { ++call_count; };

	try
	{
		auto scope_guard = indi::scope_exit{func};

		// func should not be called when initializing
		BOOST_TEST(call_count == 0);

		scope_guard.release();

		// func should not be called when released
		BOOST_TEST(call_count == 0);

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		// func should not be called when released
		BOOST_TEST(call_count == 0);
	}

	BOOST_TEST(call_count == 0);
}

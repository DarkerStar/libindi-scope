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
	auto called = false;

	{
		auto const _ = indi::scope_exit{[&called] { called = true; }};
		BOOST_TEST(not called, "function called before scope exit");
	}

	BOOST_TEST(called);
}

BOOST_AUTO_TEST_CASE(basic_operation_CASE_fail)
{
	auto called = false;

	try
	{
		auto const _ = indi::scope_exit{[&called] { called = true; }};
		BOOST_TEST(not called, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(called);
	}
}

/*****************************************************************************
 * Release operation tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(release_operation_CASE_success)
{
	auto called = false;

	{
		auto scope_guard = indi::scope_exit{[&called] { called = true; }};
		BOOST_TEST(not called, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(not called, "function called by release");
	}

	BOOST_TEST(not called, "function called despite release");
}

BOOST_AUTO_TEST_CASE(release_operation_CASE_fail)
{
	auto called = false;

	try
	{
		auto scope_guard = indi::scope_exit{[&called] { called = true; }};
		BOOST_TEST(not called, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(not called, "function called by release");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(not called, "function called despite release");
	}
}

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

#define BOOST_TEST_MODULE scope_fail
#ifdef BOOST_TEST_DYN_LINK
#	include <boost/test/unit_test.hpp>
#else
#	include <boost/test/included/unit_test.hpp>
#endif // BOOST_TEST_DYN_LINK

#include <indi/scope.hpp>

#include <indi/scope.test.hpp>

/*****************************************************************************
 * Basic operation tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_lvalue_CASE_success,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;

	// Artificial scope
	{
		auto func = Func{call_count};
		auto const _ = indi::scope_fail<Func&>{func};
		BOOST_TEST(call_count == 0, "function called before scope exit");
	}

	BOOST_TEST(call_count == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_rvalue_CASE_success,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	// Artificial scope
	{
		auto const _ = indi::scope_fail{Func{call_count}};
		BOOST_TEST(call_count == 0, "function called before scope exit");
	}

	BOOST_TEST(call_count == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_lvalue_CASE_fail,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;

	try
	{
		auto func = Func{call_count};
		auto const _ = indi::scope_fail<Func&>{func};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 1);
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_rvalue_CASE_fail,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	try
	{
		auto const _ = indi::scope_fail{Func{call_count}};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 1);
	}
}

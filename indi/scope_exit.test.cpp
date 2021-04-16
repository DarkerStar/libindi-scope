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

	{
		auto const _ = indi::scope_exit{[&call_count] { ++call_count; }};
		BOOST_TEST(call_count == 0, "function called before scope exit");
	}

	BOOST_TEST(call_count == 1);
}

BOOST_AUTO_TEST_CASE(basic_operation_CASE_fail)
{
	auto call_count = 0;

	try
	{
		auto const _ = indi::scope_exit{[&call_count] { ++call_count; }};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 1);
	}
}

/*****************************************************************************
 * Release operation tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(release_operation_CASE_success)
{
	auto call_count = 0;

	{
		auto scope_guard = indi::scope_exit{[&call_count] { ++call_count; }};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(call_count == 0, "function called by release");
	}

	BOOST_TEST(call_count == 0, "function called despite release");
}

BOOST_AUTO_TEST_CASE(release_operation_CASE_fail)
{
	auto call_count = 0;

	try
	{
		auto scope_guard = indi::scope_exit{[&call_count] { ++call_count; }};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(call_count == 0, "function called by release");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 0, "function called despite release");
	}
}

/*****************************************************************************
 * When initialization of the exit function data member fails, the exit
 * function argument passed to the constructor should be called.
 * 
 * (Reference: P0052r10 7.5.2.8)
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(exit_function_called_on_init_failure)
{
	auto call_count = 0;

	// Function object with `noexcept(false)` move-construction, which should
	// force scope_exit to do copy-construction, which will throw.
	class functor_t
	{
	public:
		explicit functor_t(int& counter) : _p_counter{&counter} {}
		functor_t(functor_t const& other) : _p_counter{other._p_counter} { throw indi_test::exception{}; }
		functor_t(functor_t&& other) noexcept(false) : _p_counter{other._p_counter} {}

		auto operator()() { ++(*_p_counter); }

	private:
		int* _p_counter = nullptr;
	};

	try
	{
		auto const _ = indi::scope_exit{functor_t{call_count}};
		BOOST_ERROR("function object copy constructor was not used");
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 1);
	}
}

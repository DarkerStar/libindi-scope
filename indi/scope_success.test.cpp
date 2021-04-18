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

#define BOOST_TEST_MODULE scope_success
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
		auto const _ = indi::scope_success<Func&>{func};
		BOOST_TEST(call_count == 0, "function called before scope exit");
	}

	BOOST_TEST(call_count == 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_rvalue_CASE_success,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	// Artificial scope
	{
		auto const _ = indi::scope_success{Func{call_count}};
		BOOST_TEST(call_count == 0, "function called before scope exit");
	}

	BOOST_TEST(call_count == 1);
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
		auto const _ = indi::scope_success<Func&>{func};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 0);
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
		auto const _ = indi::scope_success{Func{call_count}};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		throw indi_test::exception{};
	}
	catch (indi_test::exception const&)
	{
		BOOST_TEST(call_count == 0);
	}
}

/*****************************************************************************
 * When initialization of the exit function data member fails, the exit
 * function argument passed to the constructor should *NOT* be called.
 * 
 * (Reference: P0052r10 7.5.2.18)
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(exit_function_not_called_on_init_failure)
{
	// Function object with `noexcept(false)` move-construction, which should
	// force scope_success to do copy-construction, which will throw.
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

	auto call_count = 0;

	BOOST_CHECK_THROW(indi::scope_success{functor_t{call_count}}, indi_test::exception);
	BOOST_TEST(call_count == 0);
}

/*****************************************************************************
 * Special operations
 ****************************************************************************/

BOOST_AUTO_TEST_CASE_TEMPLATE(not_default_constructible, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_default_constructible_v<indi::scope_success<Func>>);
	BOOST_TEST(not std::is_default_constructible_v<indi::scope_success<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_copy_constructible, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_copy_constructible_v<indi::scope_success<Func>>);
	BOOST_TEST(not std::is_copy_constructible_v<indi::scope_success<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_copy_assignable, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_copy_assignable_v<indi::scope_success<Func>>);
	BOOST_TEST(not std::is_copy_assignable_v<indi::scope_success<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_move_assignable, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_move_assignable_v<indi::scope_success<Func>>);
	BOOST_TEST(not std::is_move_assignable_v<indi::scope_success<Func&>>);
}

// scope_success destructor is noexcept only if the wrapped function is noexcept.
BOOST_AUTO_TEST_CASE_TEMPLATE(destructor_noexcept_CASE_noexcept_functor, Func, indi_test::nonthrowing_functors<int>)
{
	BOOST_TEST(std::is_nothrow_destructible_v<indi::scope_success<Func>>);
	BOOST_TEST(std::is_nothrow_destructible_v<indi::scope_success<Func&>>);
}

// scope_success destructor is noexcept only if the wrapped function is noexcept.
BOOST_AUTO_TEST_CASE_TEMPLATE(destructor_noexcept_CASE_throwing_functor, Func, indi_test::throwing_functors<int>)
{
	BOOST_TEST(not std::is_nothrow_destructible_v<indi::scope_success<Func>>);
	BOOST_TEST(not std::is_nothrow_destructible_v<indi::scope_success<Func&>>);
}

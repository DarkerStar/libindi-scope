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

#include <memory>

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
		auto const _ = indi::scope_exit<Func&>{func};
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
		auto const _ = indi::scope_exit{Func{call_count}};
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
		auto const _ = indi::scope_exit<Func&>{func};
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
		auto const _ = indi::scope_exit{Func{call_count}};
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

BOOST_AUTO_TEST_CASE_TEMPLATE(
	release_operation_WITH_lvalue_CASE_success,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;

	// Artificial scope
	{
		auto func = Func{call_count};
		auto scope_guard = indi::scope_exit<Func&>{func};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(call_count == 0, "function called by release");
	}

	BOOST_TEST(call_count == 0, "function called despite release");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	release_operation_WITH_rvalue_CASE_success,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	// Artificial scope
	{
		auto scope_guard = indi::scope_exit{Func{call_count}};
		BOOST_TEST(call_count == 0, "function called before scope exit");

		scope_guard.release();
		BOOST_TEST(call_count == 0, "function called by release");
	}

	BOOST_TEST(call_count == 0, "function called despite release");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	release_operation_WITH_lvalue_CASE_fail,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;

	try
	{
		auto func = Func{call_count};
		auto scope_guard = indi::scope_exit<Func&>{func};
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

BOOST_AUTO_TEST_CASE_TEMPLATE(
	release_operation_WITH_rvalue_CASE_fail,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	try
	{
		auto scope_guard = indi::scope_exit{Func{call_count}};
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

	auto call_count = 0;

	BOOST_CHECK_THROW(indi::scope_exit{functor_t{call_count}}, indi_test::exception);
	BOOST_TEST(call_count == 1);
}

/*****************************************************************************
 * Move tests
 ****************************************************************************/

BOOST_AUTO_TEST_CASE_TEMPLATE(
	moving_WITH_lvalue,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;
	auto func = Func{call_count};

	auto p_scope_guard_1 = std::unique_ptr<indi::scope_exit<Func&>>{new indi::scope_exit<Func&>{func}};
	BOOST_TEST(call_count == 0); // sanity check

	auto p_scope_guard_2 = std::unique_ptr<indi::scope_exit<Func&>>{new indi::scope_exit{std::move(*p_scope_guard_1)}};
	BOOST_TEST(call_count == 0, "function called by moving scope guard");

	p_scope_guard_1.reset();
	BOOST_TEST(call_count == 0, "function called by releasing moved-from scope guard");

	p_scope_guard_2.reset();
	BOOST_TEST(call_count == 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	moving_WITH_rvalue,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	auto p_scope_guard_1 = std::unique_ptr<indi::scope_exit<Func>>{new indi::scope_exit{Func{call_count}}};
	BOOST_TEST(call_count == 0); // sanity check

	auto p_scope_guard_2 = std::unique_ptr<indi::scope_exit<Func>>{new indi::scope_exit{std::move(*p_scope_guard_1)}};
	BOOST_TEST(call_count == 0, "function called by moving scope guard");

	p_scope_guard_1.reset();
	BOOST_TEST(call_count == 0, "function called by releasing moved-from scope guard");

	p_scope_guard_2.reset();
	BOOST_TEST(call_count == 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	moving_WITH_lvalue_CASE_released,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;
	auto func = Func{call_count};

	auto p_scope_guard_1 = std::unique_ptr<indi::scope_exit<Func&>>{new indi::scope_exit<Func&>{func}};
	BOOST_TEST(call_count == 0); // sanity check

	p_scope_guard_1->release();

	auto p_scope_guard_2 = std::unique_ptr<indi::scope_exit<Func&>>{new indi::scope_exit{std::move(*p_scope_guard_1)}};
	BOOST_TEST(call_count == 0, "function called by moving scope guard");

	p_scope_guard_1.reset();
	BOOST_TEST(call_count == 0, "function called by releasing moved-from scope guard");

	p_scope_guard_2.reset();
	BOOST_TEST(call_count == 0, "function called despite release");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
	moving_WITH_rvalue_CASE_released,
	Func,
	indi_test::rvalue_functors<int>)
{
	auto call_count = 0;

	auto p_scope_guard_1 = std::unique_ptr<indi::scope_exit<Func>>{new indi::scope_exit{Func{call_count}}};
	BOOST_TEST(call_count == 0); // sanity check

	p_scope_guard_1->release();

	auto p_scope_guard_2 = std::unique_ptr<indi::scope_exit<Func>>{new indi::scope_exit{std::move(*p_scope_guard_1)}};
	BOOST_TEST(call_count == 0, "function called by moving scope guard");

	p_scope_guard_1.reset();
	BOOST_TEST(call_count == 0, "function called by releasing moved-from scope guard");

	p_scope_guard_2.reset();
	BOOST_TEST(call_count == 0, "function called despite release");
}

/*****************************************************************************
 * Special operations
 ****************************************************************************/

BOOST_AUTO_TEST_CASE_TEMPLATE(not_default_constructible, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_default_constructible_v<indi::scope_exit<Func>>);
	BOOST_TEST(not std::is_default_constructible_v<indi::scope_exit<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_copy_constructible, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_copy_constructible_v<indi::scope_exit<Func>>);
	BOOST_TEST(not std::is_copy_constructible_v<indi::scope_exit<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_copy_assignable, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_copy_assignable_v<indi::scope_exit<Func>>);
	BOOST_TEST(not std::is_copy_assignable_v<indi::scope_exit<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(not_move_assignable, Func, indi_test::all_functors<int>)
{
	BOOST_TEST(not std::is_move_assignable_v<indi::scope_exit<Func>>);
	BOOST_TEST(not std::is_move_assignable_v<indi::scope_exit<Func&>>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(destructor_noexcept, Func, indi_test::nonthrowing_functors<int>)
{
	BOOST_TEST(std::is_nothrow_destructible_v<indi::scope_exit<Func>>);
	BOOST_TEST(std::is_nothrow_destructible_v<indi::scope_exit<Func&>>);
}

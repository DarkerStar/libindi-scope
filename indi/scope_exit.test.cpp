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

#include <tuple>

#include <indi/scope.hpp>

namespace indi_test {

/*****************************************************************************
 * Test exception type.
 *
 * Used to have a unique type for exceptions that can't possibly be thrown
 * by non-test code.
 ****************************************************************************/

struct exception {};

/*****************************************************************************
 * Test function objects.
 *
 * All of these function objects take a reference to a counter `int`. Each
 * time the call operator is used, the counter is incremented.
 ****************************************************************************/

// Base class for function objects.
//
// Handles all the boilerplate machinery for getting and keeping the reference
// to the counter, and incrementing it.
template <typename T>
class functor_base
{
public:
	constexpr explicit functor_base(T& counter) noexcept : _p_counter{&counter} {}

	constexpr auto increment_counter() const noexcept -> void { ++(*_p_counter); }

protected:
	T* _p_counter = nullptr;
};

// Basic functor.
template <typename T>
struct functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	auto operator()() { functor_base<T>::increment_counter(); }
};

// Functor with `const` call.
template <typename T>
struct const_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	auto operator()() const { functor_base<T>::increment_counter(); }
};

// Functor with `noexcept` call.
template <typename T>
struct noexcept_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	auto operator()() noexcept { functor_base<T>::increment_counter(); }
};

// Functor with call operator that is both `const` and `noexcept`.
template <typename T>
struct const_noexcept_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	auto operator()() const noexcept { functor_base<T>::increment_counter(); }
};

// Functor that is move(-construct)-only.
template <typename T>
struct move_only_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	move_only_functor_t(move_only_functor_t&&) noexcept = default;

	auto operator()() { functor_base<T>::increment_counter(); }

	move_only_functor_t(move_only_functor_t const&) = delete;

	auto operator=(move_only_functor_t const&) -> move_only_functor_t& = delete;
	auto operator=(move_only_functor_t&&) -> move_only_functor_t& = delete;
};

// Functor that is copy(-construct)-only.
template <typename T>
struct copy_only_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	copy_only_functor_t(copy_only_functor_t const&) = default;

	auto operator()() { functor_base<T>::increment_counter(); }

	copy_only_functor_t(copy_only_functor_t&&) = delete;

	auto operator=(copy_only_functor_t const&) -> copy_only_functor_t& = delete;
	auto operator=(copy_only_functor_t&&) -> copy_only_functor_t& = delete;
};

// Functor that with `noexcept` copy and non-`noexcept` move.
//
// Because the move constructor might throw, it should never be used by
// `scope_exit`. Instead, the copy constructor should be used.
template <typename T>
struct move_throws_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	move_throws_functor_t(move_throws_functor_t const&) = default;
	move_throws_functor_t(move_throws_functor_t&&) noexcept(false) = default;

	auto operator()() { functor_base<T>::increment_counter(); }

	auto operator=(move_throws_functor_t const&) -> move_throws_functor_t& = delete;
	auto operator=(move_throws_functor_t&&) -> move_throws_functor_t& = delete;
};

// Functor that is non-copyable, and non-movable.
//
// Because it cannot be copied or moved, it can only be used as an lvalue
// reference.
template <typename T>
struct immobile_functor_t : functor_base<T>
{
	using functor_base<T>::functor_base;

	auto operator()() { functor_base<T>::increment_counter(); }

	immobile_functor_t(immobile_functor_t const&) = delete;
	immobile_functor_t(immobile_functor_t&&) = delete;

	auto operator=(immobile_functor_t const&) -> immobile_functor_t& = delete;
	auto operator=(immobile_functor_t&&) -> immobile_functor_t& = delete;
};

// List of functors that can be used as lvalue arguments to scope_exit.
template <typename T>
using lvalue_functors = std::tuple<
	functor_t<T>,
	const_functor_t<T>,
	noexcept_functor_t<T>,
	const_noexcept_functor_t<T>,
	move_only_functor_t<T>,
	copy_only_functor_t<T>,
	move_throws_functor_t<T>,
	immobile_functor_t<T>
>;

// List of functors that can be used as rvalue arguments to scope_exit.
template <typename T>
using rvalue_functors = std::tuple<
	functor_t<T>,
	const_functor_t<T>,
	noexcept_functor_t<T>,
	const_noexcept_functor_t<T>,
	//move_only_functor_t<T>,
	copy_only_functor_t<T>,
	move_throws_functor_t<T>
>;

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

BOOST_AUTO_TEST_CASE_TEMPLATE(
	basic_operation_WITH_lvalue_CASE_success,
	Func,
	indi_test::lvalue_functors<int>)
{
	auto call_count = 0;

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

	{
		auto const _ = indi::scope_exit{Func{call_count}};
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

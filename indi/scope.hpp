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

#ifndef INDI_INC_scope
#define INDI_INC_scope

#include <type_traits>
#include <utility>

namespace indi {
inline namespace v1 {

namespace _detail_X_scope {

// move_init_if_noexcept<T, U>(U&&)
//
// Works almost identically to `std::forward<U>(u)`, except that if
// constructing a `T` from a `U&&` may throw an exception, it will return a
// lvalue reference (not a rvalue reference).
//
// When used as the initializer of a data-member:
//      template <typename U>
//      type(U&& u) : _t{move_init_if_noexcept<T, U>(u)} {}
// it will construct `_t` with an rvalue if and only if `U` is not an lvalue
// reference AND the construction will not throw. Otherwise, it will construct
// `_t` with an lvalue. (In other words, it will only move-construct `_t` if
// that will not throw, otherwise it will copy-construct `_t`.)

template <typename T, typename U>
constexpr auto move_init_if_noexcept(U& u) noexcept -> decltype(auto)
{
	// If U is a lvalue reference, we can't move-construct in any case, so
	// return a lvalue reference.
	if constexpr (std::is_lvalue_reference_v<U> )
	{
		return static_cast<std::remove_reference_t<U>&>(u);
	}
	else
	{
		// U is an rvalue, so we can potentially move-construct a T (by
		// returning a rvalue reference).
		//
		// However, if that operation would *NOT* be noexcept, do a
		// copy-construct (by returning an lvalue reference) instead.
		if constexpr (std::is_nothrow_constructible_v<T, U>)
			return static_cast<std::remove_reference_t<U>&&>(u);
		else
			return static_cast<std::remove_reference_t<U>&>(u);
	}
}

} // namespace _detail_X_scope

template <typename EF>
class scope_exit
{
public:
	// 7.5.2.3 requirements.
	static_assert((std::is_object_v<EF> and std::is_destructible_v<EF>) or std::is_lvalue_reference_v<EF>);
	static_assert(std::is_invocable_v<std::remove_reference_t<EF>>);

	template <typename EFP>
	explicit scope_exit(EFP&& f)
		noexcept(std::is_nothrow_constructible_v<EF, EFP> or std::is_nothrow_constructible_v<EF, EFP&>)
	try :
		_exit_function{_detail_X_scope::move_init_if_noexcept<EF, EFP>(f)},
		_execute_on_destruction{true}
	{
		// 7.5.2.5 requirements.
		static_assert(not std::is_same_v<std::remove_cvref_t<EFP>, scope_exit>);
		static_assert(std::is_nothrow_constructible_v<EF, EFP> or std::is_constructible_v<EF, EFP&>);
	}
	catch (...)
	{
		f();
	}

	scope_exit(scope_exit&& rhs) noexcept(std::is_nothrow_move_constructible_v<EF> or std::is_nothrow_copy_constructible_v<EF>) {}

	~scope_exit()
	{
		if (_execute_on_destruction)
			_exit_function();
	}

	auto release() noexcept -> void
	{
		_execute_on_destruction = false;
	}

	// Non-copyable.
	scope_exit(scope_exit const&) = delete;
	auto operator=(scope_exit const&) -> scope_exit& = delete;

	// No move-assignment.
	auto operator=(scope_exit&&) -> scope_exit& = delete;

private:
	EF _exit_function;
	bool _execute_on_destruction = true;
};

template <typename EF>
scope_exit(EF) -> scope_exit<EF>;

} // inline namespace v1
} // namespace indi

#endif // include guard

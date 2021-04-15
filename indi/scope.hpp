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

template <typename EF>
class scope_exit
{
public:
	template <typename EFP>
	explicit scope_exit(EFP&& f) noexcept(std::is_nothrow_constructible_v<EF, EFP> or std::is_nothrow_constructible_v<EF, EFP&>)
		: _func{std::forward<EFP>(f)}
	{}

	scope_exit(scope_exit&& rhs) noexcept(std::is_nothrow_move_constructible_v<EF> or std::is_nothrow_copy_constructible_v<EF>) {}

	~scope_exit()
	{
		if (_call)
			_func();
	}

	auto release() noexcept -> void
	{
		_call = false;
	}

	// Non-copyable.
	scope_exit(scope_exit const&) = delete;
	auto operator=(scope_exit const&) -> scope_exit& = delete;

	// No move-assignment.
	auto operator=(scope_exit&&) -> scope_exit& = delete;

private:
	EF _func;
	bool _call = true;
};

template <typename EF>
scope_exit(EF) -> scope_exit<EF>;

} // inline namespace v1
} // namespace indi

#endif // include guard

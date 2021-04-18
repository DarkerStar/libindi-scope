# libindi-scope

[![C++ version](https://img.shields.io/badge/c%2B%2B%20version-c%2B%2B20-brightgreen)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Licence](https://img.shields.io/github/license/DarkerStar/libindi-scope)](https://github.com/DarkerStar/libindi-scope/blob/main/LICENSE)

An implementation of the proposed (P0052) C++ `<scope>` header

<table>
  <tbody>
    <tr>
      <td>Latest release</td>
      <td>
        <a href="https://github.com/DarkerStar/libindi-scope/releases"><img src="https://img.shields.io/github/release/DarkerStar/libindi-scope?sort=semver" alt="Latest release"/></a>
        <a href="https://github.com/DarkerStar/libindi-scope/releases"><img src="https://img.shields.io/github/release-date/DarkerStar/libindi-scope" alt="Latest release date"/></a>
      </td>
    </tr>
    <tr>
      <td>CI Status</td>
      <td>
        <a href="https://travis-ci.com/github/DarkerStar/libindi-scope"><img src="https://img.shields.io/travis/DarkerStar/libindi-scope/main?label=main" alt="Travis CI (main branch)"/></a>
        <a href="https://travis-ci.com/github/DarkerStar/libindi-scope"><img src="https://img.shields.io/travis/DarkerStar/libindi-scope/develop?label=develop" alt="Travis CI (develop branch)"/></a>
      </td>
    </tr>
    <tr>
      <td>Issues</td>
      <td>
        <a href="https://github.com/DarkerStar/libindi-scope/issues"><img src="https://img.shields.io/github/issues/DarkerStar/libindi-scope" alt="Open issues"/></a>
        <a href="https://github.com/DarkerStar/libindi-scope/pulls"><img src="https://img.shields.io/github/issues-pr/DarkerStar/libindi-scope" alt="Open pull requests"/></a>
      </td>
    </tr>
    <tr>
      <td>P0052 revision</td>
      <td>
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0052r10.pdf"><img src="https://img.shields.io/badge/revision-10-brightgreen" alt="P0052 revision"/></a>
      </td>
    </tr>
  </tbody>
</table>

## Overview

C++ standards proposal P0052 describes a new standard library header `<scope>` that includes scope guards.
Scope guards provide ad-hoc RAII functionality, which is particularly useful when using C library facilities in modern C++ code.

This project is all about the single header `scope.hpp`, which aims to be a drop-in replacement for `<scope>` (except that everything is in the `indi` namespace, rather than `std`).
The goal is that you can take the `scope.hpp` header and drop it into any project to be able to use P0052’s scope guards.
(And if and when `<scope>` is standardized, you can simply replace the include directive and namespace, and everything should work exactly the same.)

## Library

The following is only a brief overview of the provided library facilities.
For details, see the revision of P0052 that is currently tracked by this project.

### Scope guards

The header `scope.hpp` provides 3 scope guards:

*   `scope_exit`
*   `scope_success`
*   `scope_fail`

All three scope guards can be initialized with a function or reference to a function, which is then called when the scope guard is destroyed.
The only difference between the three scope guards are the conditions when the function is called.

| Scope guard     | When function is called                                                    |
|-----------------|----------------------------------------------------------------------------|
| `scope_exit`    | Whenever the scope guard is destroyed.                                     |
| `scope_success` | Only if the scope guard is destroyed normally (*not* via stack unwinding). |
| `scope_fail`    | Only if the scope guard is destroyed via stack unwinding.                  |

Example:

```lang-c++
auto f()
{
    auto const s1 = scope_exit   {[] { std::cout << "exit!"; }};
    auto const s2 = scope_success{[] { std::cout << "good!"; }};
    auto const s3 = scope_fail   {[] { std::cout << "fail!"; }};

    // [...]

    // "fail!" will be printed ONLY if an exception was thrown above.
    // "good!" will be printed ONLY if an exception was *NOT* thrown.
    // "exit!" will be printed, no matter what happened above.
}
```

All scope guards also have a `release()` member function, that prevents the wrapped function from being called under any circumstances.

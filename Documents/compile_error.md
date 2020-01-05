# Compile Error

## Boost

### MSVC: error C2039: 'value': is not a member of 'boost::proto'

- Version: Boost Library 1.70.0-1.71.0
- Platform: MSVC
- Reference: https://github.com/boostorg/proto/issues/20

Replace `#if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1700))` to `#if BOOST_WORKAROUND(BOOST_MSVC, < 1800)` around line 230 of `boost/proto/generate.hpp`.

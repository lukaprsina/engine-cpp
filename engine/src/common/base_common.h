#pragma once

#define ENG_ASSERT(condition) assert(condition)

#define BIT(x) (1 << x)

#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG)
#define ENG_DEBUG
#endif
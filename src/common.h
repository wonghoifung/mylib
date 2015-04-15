#ifndef COMMON_HEADER
#define COMMON_HEADER

template<typename To, typename From>
inline To implicit_cast(From const &f) {
	return f;
}

#endif


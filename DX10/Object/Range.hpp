#pragma once


template <typename T>
struct Range {
	Range(T core, T min_value, T max_value) : core(core), min_value(min_value), max_value(max_value) {}

	bool isValid() const {
		return min_value <= core && core <= max_value;
	}

	template <class O>
	Range<T>& operator = (O o) {
		core = (T)o;
		tassert(isValid());
		return *this;
	}
	template <class O>
	Range<T>& operator += (O o) {
		core += (T)o;
		tassert(isValid());
		return *this;
	}
	template <class O>
	Range<T>& operator -= (O o) {
		core -= (T)o;
		tassert(isValid());
		return *this;
	}

	template <class O> bool operator <(O o) const { return core < o; }
	template <class O> bool operator <=(O o) const { return core <= o; }
	template <class O> bool operator >=(O o) const { return core >= o; }
	template <class O> bool operator >(O o) const { return core > o; }

	const T data() const {
		return core;
	}

private:
	T core = 0.f;
	T max_value = 0.f;
	T min_value = 0.f;
};
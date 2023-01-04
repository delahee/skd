#pragma once

#include <algorithm>
#include <cctype>

namespace rd {
	class PCG32 {
	protected:
		uint64_t _state = 0;
		uint64_t _inc = 0;

		static inline const int S32_MAX = 0x7fffffff;

	public:
		/// Initialize a random number generator.
		PCG32() {

		}

		/// Initialize a random number generator. Specified in two parts, state initializer (a.k.a. seed) and a sequence selection constant (a.k.a. stream id).
		/// <param name="state">State initializer (a.k.a. seed).</param>
		/// <param name="streamID">Sequence selection constant (a.k.a. stream id). Defaults to 0.</param>
		PCG32(uint64_t state, uint64_t streamID = 0)
		{
			_state = 0;
			_inc = (streamID << 1) | 1;
			PCG32();
			_state += state;
			PCG32();
		}

		/// <summary>
		/// Generate a uniformly distributed number.
		/// </summary>
		/// <returns>A uniformly distributed 32bit unsigned integer.</returns>
		uint32_t iter()
		{
			uint64_t oldState = _state;
			// Advance internal state
			_state = (_state * 6364136223846793005ul + _inc);
			// Calculate output function (XSH RR), uses old state for max ILP
			uint64_t xorshifted = (uint64_t)(((oldState >> 18) ^ oldState) >> 27);
			int rot = (int)(oldState >> 59);
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		}

		/// Generate a uniformly distributed number, r, where 0 <= r < <paramref name="bound"/>.
		/// <param name="bound">Exclusive upper bound of the number to generate.</param>
		/// <returns>A uniformly distributed 32bit unsigned integer strictly less than <paramref name="bound"/>.</returns>
		uint32_t iter(uint32_t bound)
		{
			// To avoid bias, we need to make the range of the RNG a multiple of
			// bound, which we do by dropping output less than a threshold.
			uint32_t threshold = ((uint32_t)-(int)bound) % bound;

			// Uniformity guarantees that this loop will terminate.  In practice, it
			// should usually terminate quickly; on average (assuming all bounds are
			// equally likely), 82.25% of the time, we can expect it to require just
			// one iteration.  In the worst case, someone passes a bound of 2^31 + 1
			// (i.e., 2147483649), which invalidates almost 50% of the range.  In 
			// practice, bounds are typically small and only a tiny amount of the range
			// is eliminated.
			while (true)
			{
				uint32_t r = iter();
				if (r >= threshold)
					return r % bound;
			}
		}

		/// Generate a random number with an exclusive <paramref name="maxValue"/> upperbound.
		/// <param name="maxValue">Exclusive upper bound.</param>
		/// <returns>A random number with an exclusive <paramref name="maxValue"/> upperbound.</returns>
		int next(int maxValue)
		{
			if (maxValue < 0)
				maxValue = 0;

			return (int)iter((uint32_t)maxValue);
		}

		/// Generate a random number ranging from <paramref name="minValue"/> to <paramref name="maxValue"/>.
		/// <param name="minValue">Lower bound.</param>
		/// <param name="maxValue">Upper bound.</param>
		/// <returns>A random number ranging from <paramref name="minValue"/> to <paramref name="maxValue"/>.</returns>
		int dice(int minValue, int maxValue)
		{
			if (maxValue < minValue)
				maxValue = minValue;

			return (int)(minValue + iter((uint32_t)((int64_t)maxValue - minValue)));
		};

		/// <summary>
		/// Generate a random float ranging from 0.0f to 1.0f.
		/// </summary>
		/// <returns>A random float ranging from 0.0f to 1.0f.</returns>
		float diceF()
		{
			// This is quite hackish because we want to avoid BitConverter, but who cares?
			int bound = S32_MAX / 2 - 1;
			return next(bound) * 1.0f / bound;
		};

		/// Generate a random float ranging from <paramref name="minValue"/> to <paramref name="maxValue"/>.
		/// <param name="minValue">Lower bound.</param>
		/// <param name="maxValue">Upper bound.</param>
		/// <returns>A random float ranging from <paramref name="minValue"/> to <paramref name="maxValue"/>.</returns>
		float diceF(float minValue, float maxValue)
		{
			if (maxValue < minValue)
				maxValue = minValue;

			return minValue + (maxValue - minValue) * diceF();
		};

		/// Generate a random bool.
		/// <returns>A random bool.</returns>
		bool toss() {
			return diceF() <= 0.5f;
		};
	};
}

#pragma once

namespace rd {
	class Bits {
	public:
		static inline void set(uint32_t& bitset, uint32_t flags) {
			bitset |= flags;
		};

		static inline void unset(uint32_t& bitset, uint32_t flags) {
			bitset &= ~flags;
		}

		static inline void clear(uint32_t& bitset, uint32_t flags) {
			bitset &= ~flags;
		};

		static inline uint32_t get(uint32_t bitset, uint32_t flags) {
			return (bitset & flags) == flags;
		};

		static inline bool is(uint32_t bitset, uint32_t flags) {
			return get(bitset, flags);
		}; 
		
		static inline bool hasOne(uint32_t bitset, uint32_t flags) {
			return (bitset & flags) !=0;
		};

		static inline void toggle(uint32_t& bitset, uint32_t bits, bool _OnOff) {
			if (_OnOff)
				set(bitset, bits);
			else
				clear(bitset, bits);
		};

		static inline void toggle(uint32_t& bitset, uint32_t bits) {
			if ( !is(bitset,bits) )
				set(bitset, bits);
			else
				clear(bitset, bits);
		};
	};
}
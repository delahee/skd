#pragma once

#include "EASTL/vector.h"

namespace rd {
	class BitArray {
	protected:
		//this how many bit per int you store
		static const uint32_t		BIT_SHIFT = 5;//32bit
		static const uint32_t		BIT_WIDTH = 1<<5;
		static const uint32_t		BIT_MASK = (1 << 5)-1;
		eastl::vector<uint32_t>		buffer;
	public:
		inline void resize(uint32_t _Index){
			buffer.resize(_Index >> BIT_SHIFT);
		}

		inline void set(uint32_t _Index){
			uint32_t cell = _Index >> BIT_SHIFT;
			if(buffer.size() < cell + 1) 
				buffer.resize(cell + 1,0);
			buffer[cell] |= 1<<(_Index&BIT_MASK);
		};

		inline void clear(uint32_t _Index){
			uint32_t nth = _Index & ((1 << BIT_SHIFT) - 1);
			buffer[_Index >> BIT_SHIFT] &= ~(1 << nth);
		};

		inline void clear(){
			buffer.clear();
		};

		inline bool get(uint32_t _Index) const{
			uint32_t cell = _Index >> BIT_SHIFT;
			if (cell >= buffer.size())
				return false;
			uint32_t bit = 1 << (_Index & BIT_MASK);
			return (buffer[cell] & bit) == bit;
		};

		inline bool is(uint32_t _Index) const{
			return get(_Index);
		};

		inline size_t getCellCount() const {
			return buffer.size();
		}; 
		
		inline size_t getMaxIdx() const {
			return getCellCount() << BIT_SHIFT;
		};

		inline void toggle(uint32_t _Index, bool _OnOff) {
			if (_OnOff)
				set(_Index);
			else
				clear(_Index);
		};

		inline bool operator[](int _Index) const
		{
			return get((uint32_t)_Index);
		};

	};
}
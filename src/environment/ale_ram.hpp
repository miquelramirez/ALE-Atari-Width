/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare and 
 *   the Reinforcement Learning and Artificial Intelligence Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  ale_ram.hpp
 *
 *  A class that encapsulates the Atari 2600 RAM. Code is provided inline for
 *   efficiency reasonss.
 *  
 **************************************************************************** */

#ifndef __ALE_RAM_HPP__
#define __ALE_RAM_HPP__

#include <string.h>
#include <bitset>

typedef unsigned char byte_t;

#define RAM_SIZE (128)

/** A simple wrapper around the Atari RAM. */ 
class ALERAM { 
  public:
    ALERAM();
    ALERAM(const ALERAM &rhs);

    ALERAM& operator=(const ALERAM &rhs);

    /** Byte accessors */ 
    byte_t get(unsigned int x) const;
    byte_t *byte(unsigned int x);
   
    /** Bit accessor */
    byte_t get(unsigned int byte, unsigned int bit ) const;

    /** Returns the whole array (equivalent to byte(0)). */
    byte_t *array() const { return (byte_t*)(m_ram); }

    void print() const;
    void print( unsigned int i) const;
    
    size_t size() const { return sizeof(m_ram); }
    /** Returns whether two copies of the RAM are equal */
    bool equals(const ALERAM &rhs) const;

	void reset(){
		for (size_t i = 0; i < size(); i++){
			
			m_ram[i & 0x7F] = (byte_t)0;
		}

	}
	void or_op(const ALERAM &rhs){
		for (size_t i = 0; i < size(); i++){
			
			m_ram[i & 0x7F] = m_ram[i & 0x7F] | rhs.get(i);
		}
	}

	bool new_bit(const ALERAM &rhs){
		for (size_t i = 0; i < size(); i++){
			
			byte_t  b = (m_ram[i & 0x7F] ^ rhs.get(i)) & rhs.get(i) ;
			if (b != (byte_t)0){
				// std::cout <<"b = "<< (bitset<8>) b << std::endl;
				// print(i);
				// rhs.print(i);
				return true;
			}
		}
		return false;
	}

  protected:
    byte_t m_ram[RAM_SIZE];
};

inline ALERAM::ALERAM() {
}

inline ALERAM::ALERAM(const ALERAM &rhs) {
  // Copy data over
  memcpy(m_ram, rhs.m_ram, sizeof(m_ram));
}

inline ALERAM& ALERAM::operator=(const ALERAM &rhs) {
  // Copy data over 
  memcpy(m_ram, rhs.m_ram, sizeof(m_ram));

  return *this;
}


inline void ALERAM::print() const {
  for (size_t i = 0; i < size(); i++){
	  std::cout << i <<" = "<< (bitset<8>) m_ram[i & 0x7F] << std::endl;
  }
}

inline void ALERAM::print( unsigned int i) const {

	  std::cout << i <<" = "<< (bitset<8>) m_ram[i & 0x7F] << std::endl;

}

inline bool ALERAM::equals(const ALERAM &rhs) const {
  return (memcmp(m_ram, rhs.m_ram, size()) == 0);
}

// Byte accessors 
inline byte_t ALERAM::get(unsigned int x) const {
  // Wrap RAM around the first 128 bytes
  return m_ram[x & 0x7F]; 
}



inline byte_t* ALERAM::byte(unsigned int x) {
  return &m_ram[x & 0x7F]; 
}

// Bit accessors
inline byte_t ALERAM::get(unsigned int byte, unsigned int  bit ) const {
  // Wrap RAM around the first 128 bytes
    return (m_ram[byte & 0x7F] & (1 << bit) ) > 0; 
}


#endif // __ALE_RAM_HPP__


// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file my_bit_array.h
/// @class MyBitArray
/// @brief Class for storing array efficiently
///
/// This class implements a data structure for storing array of integer values in a memory efficient manner.
/// Not every value would require a fixed width memory space, thus the class uses only the necessary
/// amount of space required to store the value. The implementation is similar to \ref MyPackedArray , but consumes less than or equal
/// memory space.

#pragma once

#include "common.h"
#include "utils.h"

namespace ufal {
namespace korektor {

class MyBitArray {
 private:
  uint32_t num_bytes; ///< Number of bytes required to store the values
  unsigned char* data; ///< Pointer to the actual data
  vector<uint32_t> output_mask; ///< Maximum value that can be stored for different bit size



 public:

  /// @brief Get the number bytes required to store the data
  ///
  /// @return The number of bytes (integer)
  inline uint32_t GetNumberOfBytes() const
  {
    return num_bytes;
  }

  /// @brief Get the value at index
  ///
  /// @param index Index value
  /// @param num_bits Number of bits required to store the value
  inline uint32_t GetValueAt(uint32_t index, unsigned num_bits) const
  {
    uint32_t byte_pointer = index >> 3;
    uint32_t bit_offset = index % 8;

    uint64_t retVal = *((uint64_t*)&(data[byte_pointer]));
    retVal = (retVal >> bit_offset) & output_mask[num_bits];

    return (uint32_t)retVal;

//    uint64_t retVal = 0;
//    unsigned char* ukaz = (unsigned char*)&retVal;
//
//    uint32_t byte_pointer = index >> 3;
//    uint32_t bit_pointer = index % 8;
//
//    int bits_left = num_bits;
//    ukaz[0] = data[byte_pointer];
//    bits_left -= 8 - bit_pointer;
//    uint32_t offset = 1;
//
//    while (bits_left > 0)
//    {
//      bits_left -= 8;
//      ukaz[offset] = data[byte_pointer + offset];
//      offset++;
//    }
//
//    retVal = (retVal >> bit_pointer) & output_mask[num_bits];
//
//    return retVal;
  }

  /// @brief Destructor that frees the memory of data
  ~MyBitArray()
  {
    if (data != NULL)
      delete[] data;
  }

  /// @brief Constructor initialization from another \see MyBitArray object
  ///
  /// @param val Object of type
  MyBitArray(const MyBitArray& val)
  {
    num_bytes = val.num_bytes;
    output_mask = val.output_mask;

    data = new unsigned char[num_bytes];

    for (uint32_t i = 0; i < num_bytes; i++)
    {
      data[i] = val.data[i];
    }
  }

  /// @brief Constructor initialization using assignment operator
  ///
  /// @param val Object of type
  MyBitArray& operator=(const MyBitArray& val)
  {
    if (this != &val)
    {
      unsigned char* new_data = new unsigned char[val.num_bytes];

      for (uint32_t i = 0; i < val.num_bytes; i++)
      {
        new_data[i] = val.data[i];
      }

      num_bytes = val.num_bytes;
      output_mask = val.output_mask;

      if (data != NULL)
        delete[] data;
      data = new_data;

    }
    return *this;
  }

  /// @brief Default constructor
  MyBitArray(): num_bytes(0), data(NULL) {}

  /// @brief Constructor initialization from input stream/file
  ///
  /// @param ifs Input stream
  MyBitArray(istream &ifs)
  {
    string check_string = MyUtils::ReadString(ifs);

    FATAL_CONDITION(check_string == "MBA", check_string);


    output_mask.push_back(0);
    uint32_t curr_mask = 0;
    for (unsigned i = 1; i <= 32; i++)
    {
      curr_mask = curr_mask << 1;
      curr_mask++;
      output_mask.push_back(curr_mask);
    }


    ifs.read((char*)&num_bytes, sizeof(uint32_t));

    //TODO: initialization of the output masks;
    //ifs.read((char*)&output_mask, sizeof(uint32_t));

    data = new unsigned char[num_bytes];

    ifs.read((char*)data, sizeof(unsigned char) * num_bytes);

  }

  /// @brief Write the output to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs) const
  {
    MyUtils::WriteString(ofs, "MBA");
    ofs.write((char*)&num_bytes, sizeof(uint32_t));

    //TODO: output masks!
    //ofs.write((char*)&output_mask, sizeof(uint32_t));
    ofs.write((char*)data, sizeof(unsigned char) * num_bytes);

  }

  /// @brief Constructor initialization from vector of pairs
  ///
  /// @param values Vector of pairs of integers
  MyBitArray(const vector<pair<uint32_t, unsigned> > &values)
  {
    uint32_t bits_needed = 0;
    for (uint32_t i = 0; i < values.size(); i++)
    {
      bits_needed += values[i].second;
    }

    num_bytes = (bits_needed + 7) / 8;

    output_mask.push_back(0);
    uint32_t curr_mask = 0;
    for (unsigned i = 1; i <= 32; i++)
    {
      curr_mask = curr_mask << 1;
      curr_mask++;
      output_mask.push_back(curr_mask);
    }

    data = new unsigned char[num_bytes];

    for (uint32_t i = 0; i < num_bytes; i++)
      data[i] = 0;

    uint32_t byte_pointer;
    uint32_t bit_pointer;
    uint32_t bit_position = 0;
    uint64_t curr_value;
    unsigned char* ukaz;
    for (uint32_t i = 0; i < values.size(); i++)
    {
      unsigned bites_left = values[i].second;
      byte_pointer = bit_position >> 3;
      bit_pointer = bit_position % 8;

      curr_value = values[i].first;

      curr_value = curr_value << bit_pointer;
      ukaz = (unsigned char*)&curr_value;
      bites_left -= 8 - bit_pointer;
      data[byte_pointer] = data[byte_pointer] | ukaz[0];

      uint32_t j = 1;
      while (bites_left > 0)
      {
        data[byte_pointer + j] = data[byte_pointer + j] | ukaz[j];
        bites_left -= 8;
        j++;
      }

      bit_position += values[i].second;
    }

    bit_position = 0;
    for (uint32_t i = 0; i < values.size(); i++)
    {
      assert(GetValueAt(bit_position, values[i].second) == values[i].first);
      bit_position += values[i].second;
    }

  }
};

SP_DEF(MyBitArray);

} // namespace korektor
} // namespace ufal

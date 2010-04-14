/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef _MY_PACKED_ARRAY_HPP_
#define _MY_PACKED_ARRAY_HPP_

#include "StdAfx.h"
#include "utils.hpp"

namespace ngramchecker {

	class MyPackedArray {
		private:
			uint32_t bits_per_value;
			uint32_t num_values;
			uint32_t num_bytes;
			unsigned char* data;
			uint32_t output_mask;



		public:

			void PrintBits() const;

			uint32_t GetSize() const;

			uint32_t GetNumberOfBytes() const;

			uint32_t GetBitsPerValue() const;

			inline uint32_t GetValueAt(uint32_t index) const
			{
				uint32_t bit = index * bits_per_value;
				uint32_t byte_pointer = bit >> 3;
				uint32_t bit_offset = bit % 8;

				ulong64 retVal = *((ulong64*)&(data[byte_pointer]));
				retVal = (retVal >> bit_offset) & output_mask;
				
				return (uint32_t)retVal;
			}


			~MyPackedArray();

			MyPackedArray() { data = NULL; }
			MyPackedArray(const MyPackedArray& val);

			MyPackedArray& operator=(const MyPackedArray& val);

			MyPackedArray(istream &ifs);

			MyPackedArray(const string filename);

			void WriteToStream(ostream &ofs) const;

			void SaveToFile(const string filename) const;

			MyPackedArray(const vector<uint32_t> &values);

			static void TestIt()
			{

				vector<uint32_t> vals;

				for (uint i = 0; i < 10000000; i++)
				{
					vals.push_back(MyUtils::randomR(0, 10000000));
				}
				
				MyPackedArray mpa = MyPackedArray(vals);

				for (uint j = 0; j < 300; j++)
				{
					for (uint i = 0; i < 10000000; i++)
					{
						FATAL_CONDITION(vals[i] == mpa.GetValueAt(i), "");
					}
				}
				cerr << "Test ok!" << endl;
			}
	};

}

#endif //_MY_PACKED_ARRAY_HPP_

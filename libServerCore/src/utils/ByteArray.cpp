#include "ByteArray.h"
#include <assert.h>

namespace ws
{
	namespace utils
	{
		/************************************************************************/
		/* 读取字节流方法                                                        */
		/************************************************************************/
#define READ_TYPE(TypeName, pos, totalSize, ptr)\
		{ \
			size_t typeSize = sizeof(TypeName); \
			if (pos + typeSize <= totalSize)\
			{\
				TypeName* pType = (TypeName*)((intptr_t)ptr + pos);\
				pos += typeSize;\
				return *pType;\
			}\
			pos = totalSize;\
			return 0;\
		}

		ByteArray::ByteArray() :capacity(DEFAULT_SIZE), contentSize(0), position(0), isAttached(false)
		{
			pBytes = malloc(DEFAULT_SIZE);
			memset(pBytes, 0, DEFAULT_SIZE);
		}

		ByteArray::ByteArray(size_t length) :capacity(length), contentSize(0), position(0), isAttached(false)
		{
			if (length > 0)
			{
				pBytes = malloc(length);
				memset(pBytes, 0, length);
			}
		}

		ByteArray::ByteArray(void* bytes, size_t length, bool copy /*= false*/) :capacity(length), contentSize(length), position(0), isAttached(!copy)
		{
			if (copy)
			{
				pBytes = malloc(length);
				memcpy(pBytes, bytes, length);
			}
			else
			{
				pBytes = bytes;
			}
		}

		ByteArray::~ByteArray()
		{
			if (!isAttached)
			{
				free(pBytes);
			}
			pBytes = NULL;
		}

		char ByteArray::readByte()
		{
			READ_TYPE(char, position, contentSize, pBytes)
		}

		unsigned char ByteArray::readUnsignedByte()
		{
			READ_TYPE(unsigned char, position, contentSize, pBytes)
		}

		short ByteArray::readShort()
		{
			READ_TYPE(short, position, contentSize, pBytes)
		}

		unsigned short ByteArray::readUnsignedShort()
		{
			READ_TYPE(unsigned short, position, contentSize, pBytes)
		}

		int ByteArray::readInt()
		{
			READ_TYPE(int, position, contentSize, pBytes)
		}

		unsigned int ByteArray::readUnsignedInt()
		{
			READ_TYPE(unsigned int, position, contentSize, pBytes)
		}

		long long ByteArray::readInt64()
		{
			READ_TYPE(long long, position, contentSize, pBytes)
		}

		unsigned long long ByteArray::readUnsignedInt64()
		{
			READ_TYPE(unsigned long long, position, contentSize, pBytes)
		}

		float ByteArray::readFloat()
		{
			READ_TYPE(float, position, contentSize, pBytes)
		}

		double ByteArray::readDouble()
		{
			READ_TYPE(double, position, contentSize, pBytes)
		}

		size_t ByteArray::readBytes(ByteArray& outBytes, unsigned int offset /*= 0*/, size_t length /*= 0*/)
		{
			if (length == 0 || length > contentSize - position)	//读取数据量限制
			{
				length = contentSize - position;
			}
			if (length > 0)
			{
				outBytes.resize(length);
				if (offset + length > outBytes.contentSize)
				{
					outBytes.contentSize = offset + length;
				}
				void* pDst = (void*)((intptr_t)outBytes.pBytes + offset);
				void* pSrc = (void*)((intptr_t)pBytes + position);
				memcpy(pDst, pSrc, length);
				position += length;
			}
			return length;
		}

		size_t ByteArray::readObject(void* outBuff, size_t length)
		{
			if (outBuff == NULL)
			{
				return 0;
			}
			if (length == 0 || length > contentSize - position)
			{
				length = contentSize - position;
			}
			if (length > 0)
			{
				memcpy(outBuff, (void*)((intptr_t)pBytes + position), length);
				position += length;
			}
			return length;
		}

		std::string ByteArray::readString(size_t length)
		{
			if (length == 0 || length > contentSize - position)
			{
				length = contentSize - position;
			}
			if (length > 0)
			{
				char* pStr = (char*)((intptr_t)pBytes + position);
				return std::string(pStr, length);
			}
			return std::string();
		}

		/************************************************************************/
		/* 写入字节流方法                                                        */
		/************************************************************************/
		void ByteArray::resize(size_t size)
		{
			size_t newCap(capacity);
			while (position + size > newCap)
			{
				newCap += STEP_SIZE;
			}
			if (newCap != capacity)
			{
				capacity = newCap;
				pBytes = realloc(pBytes, newCap);
				assert(pBytes != NULL);
			}
		}

		void ByteArray::writeByte(const char& b)
		{
			writeType(b);
		}

		void ByteArray::writeUnsignedByte(const unsigned char& b)
		{
			writeType(b);
		}

		void ByteArray::writeShort(const short& s)
		{
			writeType(s);
		}

		void ByteArray::writeUnsignedShort(const unsigned short& s)
		{
			writeType(s);
		}

		void ByteArray::writeInt(const int& i)
		{
			writeType(i);
		}

		void ByteArray::writeUnsignedInt(const unsigned int& i)
		{
			writeType(i);
		}

		void ByteArray::writeInt64(const long long& ll)
		{
			writeType(ll);
		}

		void ByteArray::writeUnsignedInt64(const unsigned long long& ull)
		{
			writeType(ull);
		}

		void ByteArray::writeFloat(const float& f)
		{
			writeType(f);
		}

		void ByteArray::writeDouble(const double& d)
		{
			writeType(d);
		}

		void ByteArray::writeBytes(const ByteArray& inBytes, size_t offset /*= 0*/, size_t length /*= 0*/)
		{
			if (offset >= inBytes.contentSize)
			{
				return;
			}
			if (length == 0 || offset + length > inBytes.contentSize)
			{
				length = inBytes.contentSize - offset;
			}
			if (length > 0)
			{
				resize(length);
				void* pDst = (void*)((intptr_t)pBytes + position);
				void* pSrc = (void*)((intptr_t)inBytes.pBytes + offset);
				memcpy(pDst, pSrc, length);
				position += length;
				if (contentSize < position)
				{
					contentSize = position;
				}
			}
		}

		void ByteArray::writeObject(const void* obj, size_t length)
		{
			if (length > 0)
			{
				resize(length);
				memcpy((void*)((intptr_t)pBytes + position), obj, length);
				position += length;
				if (contentSize < position)
				{
					contentSize = position;
				}
			}
		}

		void ByteArray::truncate()
		{
			memset(pBytes, 0, capacity);
			position = 0;
			contentSize = 0;
			/*if (isAttached)
			{
				memset(pBytes, 0, contentSize);
				position = 0;
			}
			else
			{
				//free(pBytes);
				//pBytes = malloc(DEFAULT_SIZE);
				//capacity = DEFAULT_SIZE;
			}*/
		}

		void ByteArray::cutHead(size_t length, char* pOut /*= nullptr*/)
		{
			if (length > contentSize)
			{
				length = contentSize;
			}
			if (length > 0)
			{
				if (pOut)
				{
					memcpy(pOut, pBytes, length);
				}
				if (length < contentSize)
				{
					contentSize -= length;
					memmove(pBytes, (void*)((intptr_t)pBytes + length), contentSize);
				}
				else
				{
					contentSize = 0;
				}
				if (length < position)
				{
					position -= length;
				}
				else
				{
					position = 0;
				}
			}
		}

		void ByteArray::cutHead(size_t length, ByteArray& ba)
		{
			if (length > contentSize)
			{
				length = contentSize;
			}
			if (length > 0)
			{
				ba.writeBytes(*this, 0, length);
				if (length < contentSize)
				{
					contentSize -= length;
					memmove(pBytes, (void*)((intptr_t)pBytes + length), contentSize);
				}
				else
				{
					contentSize = 0;
				}
				if (length < position)
				{
					position -= length;
				}
				else
				{
					position = 0;
				}
			}
		}

		void ByteArray::cutTail(size_t length, char* pOut /*= nullptr*/)
		{
			if (length > contentSize)
			{
				length = contentSize;
			}
			if (length > 0)
			{
				if (pOut)
				{
					memcpy(pOut, (void*)((intptr_t)pBytes + contentSize - length), length);
				}
				if (length < contentSize)
				{
					contentSize -= length;
				}
				else
				{
					contentSize = 0;
				}
				if (length < position)
				{
					position -= length;
				}
				else
				{
					position = 0;
				}
			}
		}

		void ByteArray::cutTail(size_t length, ByteArray& ba /*= nullptr*/)
		{
			if (length > contentSize)
			{
				length = contentSize;
			}
			if (length > 0)
			{
				ba.writeBytes(*this, contentSize - length, length);
				if (length < contentSize)
				{
					contentSize -= length;
				}
				else
				{
					contentSize = 0;
				}
				if (length < position)
				{
					position -= length;
				}
				else
				{
					position = 0;
				}
			}
		}

		void ByteArray::toHexString(char* dest, size_t length, bool toUpperCase /*= false*/)
		{
			unsigned char* bytes = (unsigned char*)pBytes;
			const char* format(nullptr);
			if (toUpperCase)
			{
				format = "%02X";
			}
			else
			{
				format = "%02x";
			}
			for (size_t i = 0; i < contentSize; ++i)
			{
				sprintf(dest, format, bytes[i]);
				dest += 2;
			}
		}
	}
}
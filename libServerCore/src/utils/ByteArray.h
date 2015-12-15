#ifndef __WS_UTILS_BYTEARRAY_H__
#define __WS_UTILS_BYTEARRAY_H__

#include <string>
#include <mutex>
namespace ws
{
	namespace utils
	{
#pragma pack(push)
#pragma pack(1)
		class ByteArray
		{
		public:
			ByteArray(void);
			ByteArray(unsigned int length);
			ByteArray(void* bytes, unsigned int length);
			virtual ~ByteArray(void);

			size_t				position;
			inline size_t		getSize() { return contentSize; }
			inline size_t		available() { return contentSize - position; };
			void				truncate();
			void				cutHead(size_t length, char* pOut = nullptr);
			void				cutHead(size_t length, ByteArray* ba);
			void				cutTail(size_t length, char* pOut = nullptr);
			void				cutTail(size_t length, ByteArray* ba);
			inline void			lock() { mtx.lock(); };
			inline void			unlock(){ mtx.unlock(); };
			inline const void*	getBytes(){ return pBytes; };

			bool				readBoolean();
			char				readByte();
			unsigned char		readUnsignedByte();
			short				readShort();
			unsigned short		readUnsignedShort();
			int					readInt();
			unsigned int		readUnsignedInt();
			float				readFloat();
			double				readDouble();
			/************************************************************************/
			/* 将数据读到另一个ByteArray中                                           */
			/* ba		要读入的目标ByteArray                                             */
			/* offset	从ba的offset位置开始写入                                      */
			/* length	要读取的内容大小                                              */
			/************************************************************************/
			size_t				readBytes(ByteArray& outBytes, unsigned int offset = 0, size_t length = 0);
			/************************************************************************/
			/* 将数据读到一个内存块中                                                 */
			/* outBuff	要读到的内存块                                               */
			/* size		要读取的内容大小                                             */
			/************************************************************************/
			size_t				readObject(void* outBuff, size_t size = 0);
			std::string			readString(size_t length);

			void				writeBoolean(const bool& b);
			void				writeByte(const char& b);
			void				writeUnsignedByte(const unsigned char& b);
			void				writeShort(const short& s);
			void				writeUnsignedShort(const unsigned short& s);
			void				writeInt(const int& i);
			void				writeUnsignedInt(const unsigned int& i);
			void				writeFloat(const float& f);
			void				writeDouble(const double& d);
			void				writeBytes(const ByteArray& inBytes, size_t offset = 0, size_t length = 0);
			void				writeObject(const void* inBuff, size_t length);
			template <typename T> void writeType(const T& val);

		private:
			static const unsigned int DEFAULT_SIZE = 100;
			static const unsigned int STEP_SIZE = 100;
			void*				pBytes;			//数据内存块
			size_t				capacity;		//pBytes内存块大小
			size_t				contentSize;	//实际内容大小
			void				resize(size_t typeSize);	//根据typeSize重新分配pBytes的内存
			std::mutex			mtx;
		};
#pragma pack(pop)
	}
}

#endif	//__BYTEARRAY_H__
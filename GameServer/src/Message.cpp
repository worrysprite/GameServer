#include "Message.h"

void Message::readUTF8(ByteArray& input, std::string& output)
{
	unsigned short length(input.readUnsignedShort());
	char* str = new char[length];
	input.readObject(str, length);
	output = std::string(str, length);
	delete[] str;
}

void Message::writeUTF8(ByteArray& output, std::string& input)
{
	unsigned short length((unsigned short)input.size());
	output.writeUnsignedShort(length);
	output.writeObject(input.c_str(), length);
}

void MessageHead::unpack(ByteArray& input)
{
	input.lock();
	command = input.readUnsignedShort();
	packSize = input.readUnsignedShort();
	input.unlock();
}

void MessageHead::pack(ByteArray& output)
{
	output.lock();
	output.writeUnsignedShort(command);
	output.writeUnsignedShort(packSize);
	output.unlock();
}

void ActivationMessage::unpack(ByteArray& input)
{
	input.lock();
	readUTF8(input, code);
	input.unlock();
}

void ActivationMessage::pack(ByteArray& output)
{
	output.lock();
	output.writeUnsignedByte(status);
	writeUTF8(output, code);
	writeBlock(output, reward, plane4)
	output.unlock();
}

void UIConfigMessage::unpack(ByteArray& input)
{
	input.lock();
	version = input.readUnsignedByte();
	platform = input.readUnsignedByte();
	input.unlock();
}

void UIConfigMessage::pack(ByteArray& output)
{
	output.lock();
	output.writeUnsignedShort(numConfig);
	output.writeObject(configList, sizeof(UIConfig) * numConfig);
	output.unlock();
}

void SDKConfigMessage::unpack(ByteArray& input)
{
	input.lock();
	version = input.readUnsignedByte();
	platform = input.readUnsignedByte();
	input.unlock();
}

void SDKConfigMessage::pack(ByteArray& output)
{
	output.lock();
	output.writeUnsignedInt(opensdk);
	output.unlock();
}

void ConsoleLoginMessage::unpack(ByteArray& input)
{
	input.lock();
	readUTF8(input, username);
	readUTF8(input, password);
	input.unlock();
}

void ConsoleLoginMessage::pack(ByteArray& output)
{
	output.lock();
	output.writeUnsignedInt(id);
	output.unlock();
}

void ConsoleSubscribeMessage::unpack(ByteArray& input)
{

}

void ConsoleSubscribeMessage::pack(ByteArray& output)
{

}

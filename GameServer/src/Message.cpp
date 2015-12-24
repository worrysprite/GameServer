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
	command = input.readUnsignedShort();
	packSize = input.readUnsignedShort();
}

void MessageHead::pack(ByteArray& output)
{
	output.writeUnsignedShort(command);
	output.writeUnsignedShort(packSize);
}

void ActivationMessage::unpack(ByteArray& input)
{
	readUTF8(input, code);
}

void ActivationMessage::pack(ByteArray& output)
{
	output.writeUnsignedByte(status);
	writeUTF8(output, code);
	writeBlock(output, reward, plane4)
}

void UIConfigMessage::unpack(ByteArray& input)
{
	version = input.readUnsignedByte();
	platform = input.readUnsignedByte();
}

void UIConfigMessage::pack(ByteArray& output)
{
	output.writeUnsignedShort(numConfig);
	output.writeObject(configList, sizeof(UIConfig) * numConfig);
}

void SDKConfigMessage::unpack(ByteArray& input)
{
	version = input.readUnsignedByte();
	platform = input.readUnsignedByte();
}

void SDKConfigMessage::pack(ByteArray& output)
{
	output.writeUnsignedInt(opensdk);
}

void ConsoleLoginMessage::unpack(ByteArray& input)
{
	readUTF8(input, username);
	readUTF8(input, password);
}

void ConsoleLoginMessage::pack(ByteArray& output)
{
	output.writeUnsignedInt(id);
}

void ConsoleSubscribeMessage::unpack(ByteArray& input)
{
	subscribed = input.readUnsignedByte();
}

void ConsoleSubscribeMessage::pack(ByteArray& output)
{
	writeBlock(output, subscribed, memoryPeak)
}

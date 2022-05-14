/**
 * @file        hanami_messages.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2020 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include <libKitsunemimiHanamiMessaging/hanami_messages.h>

namespace Kitsunemimi
{
namespace Hanami
{

//==================================================================================================

/**
 * @brief constructor
 */
HanamiMessage::HanamiMessage() {}

/**
 * @brief destructor
 */
HanamiMessage::~HanamiMessage() {}

/**
 * @brief initialize converting content into bytes
 *
 * @param result buffer for the resulting message
 * @param totalMsgSize total number of bytes for the complete message
 */
void
HanamiMessage::initBlob(DataBuffer &result, const uint64_t totalMsgSize)
{
    allocateBlocks_DataBuffer(result, calcBytesToBlocks(totalMsgSize));
    result.usedBufferSize = totalMsgSize;
    MessageHeader* header = static_cast<MessageHeader*>(result.data);

    header->type = m_type;
    header->messageSize = totalMsgSize;

    m_pos = sizeof(MessageHeader);
}

/**
 * @brief append a string to the message
 *
 * @param result data-buffer, which holds the resulting bytes
 * @param val string to convert
 */
void
HanamiMessage::appendString(DataBuffer &result, const std::string &val)
{
    uint8_t* u8Data = static_cast<uint8_t*>(result.data);

    Entry* tempEntry = reinterpret_cast<Entry*>(&u8Data[m_pos]);
    tempEntry->type = EntryType::STRING_ENTRY_TYPE;
    tempEntry->valSize = val.size();
    m_pos += sizeof(Entry);

    if(tempEntry->valSize > 0)
    {
        memcpy(&u8Data[m_pos], val.c_str(), tempEntry->valSize);
        m_pos += tempEntry->valSize;
    }
}

/**
 * @brief append bytes to the message
 *
 * @param result data-buffer, which holds the resulting bytes
 * @param val buffer with bytes to add
 */
void
HanamiMessage::appendData(DataBuffer &result, const DataBuffer &val)
{
    uint8_t* u8Data = static_cast<uint8_t*>(result.data);

    Entry* tempEntry = reinterpret_cast<Entry*>(&u8Data[m_pos]);
    tempEntry->type = EntryType::BYTE_ENTRY_TYPE;
    tempEntry->valSize = val.usedBufferSize;
    m_pos += sizeof(Entry);

    if(tempEntry->valSize > 0)
    {
        memcpy(&u8Data[m_pos], val.data, val.usedBufferSize);
        m_pos += tempEntry->valSize;
    }
}

/**
 * @brief initialize the reading of a message
 *
 * @param data bytes to read
 * @param dataSize number of bytes to read
 *
 * @return false, if message is invalid, else true
 */
bool
HanamiMessage::initRead(const void* data, const uint64_t dataSize)
{
    // check if even enough data for the header exist
    if(dataSize < sizeof(MessageHeader)) {
        return false;
    }

    // check message size with given data-amount
    const MessageHeader* header = static_cast<const MessageHeader*>(data);
    if(header->messageSize != dataSize) {
        return false;
    }

    m_pos = sizeof(MessageHeader);

    return true;
}

/**
 * @brief read string from bytes
 *
 * @param data bytes to read
 * @param output reference for the output-string
 *
 * @return false, if message is invalid, else true
 */
bool
HanamiMessage::readString(const void* data, std::string& output)
{
    const uint8_t* u8Data = static_cast<const uint8_t*>(data);
    const Entry* tempEntry = nullptr;

    tempEntry = reinterpret_cast<const Entry*>(&u8Data[m_pos]);
    if(tempEntry->type != EntryType::STRING_ENTRY_TYPE) {
        return false;
    }
    m_pos += sizeof(Entry);

    if(tempEntry->valSize > 0)
    {
        output = std::string(reinterpret_cast<const char*>(&u8Data[m_pos]), tempEntry->valSize);
        m_pos += tempEntry->valSize;
    }

    return true;
}

/**
 * @brief read bytes from bytes
 *
 * @param data bytes to read
 * @param output buffer for the bytes to read
 *
 * @return false, if message is invalid, else true
 */
bool
HanamiMessage::readBinary(const void* data, DataBuffer &output)
{
    const uint8_t* u8Data = static_cast<const uint8_t*>(data);
    const Entry* tempEntry = nullptr;

    tempEntry = reinterpret_cast<const Entry*>(&u8Data[m_pos]);
    if(tempEntry->type != EntryType::BYTE_ENTRY_TYPE) {
        return false;
    }
    m_pos += sizeof(Entry);

    if(tempEntry->valSize > 0)
    {
        allocateBlocks_DataBuffer(output, calcBytesToBlocks(tempEntry->valSize));
        memcpy(output.data, data, tempEntry->valSize);
        m_pos += tempEntry->valSize;
    }

    return true;
}

//==================================================================================================

/**
 * @brief constructor
 */
ErrorLog_Message::ErrorLog_Message()
{
    m_type = 255;
}

/**
 * @brief destructor
 */
ErrorLog_Message::~ErrorLog_Message() {}

/**
 * @brief read message from bytes
 *
 * @param data data-pointer to read
 * @param dataSize number of bytes to read
 *
 * @return false, if message is broken, else true
 */
bool
ErrorLog_Message::read(const void* data, const uint64_t dataSize)
{
    if(initRead(data, dataSize) == false) {
        return false;
    }

    if(readString(data, userUuid) == false) {
        return false;
    }
    if(readString(data, component) == false) {
        return false;
    }
    if(readString(data, errorMsg) == false) {
        return false;
    }
    if(readString(data, context) == false) {
        return false;
    }
    if(readString(data, values) == false) {
        return false;
    }

    return true;
}

/**
 * @brief convert message content into binary to send
 *
 * @param result data-buffer for the resulting binary
 */
void
ErrorLog_Message::createBlob(DataBuffer &result)
{
    const uint64_t totalMsgSize = sizeof(MessageHeader)
                                  + 5 * sizeof(Entry)
                                  + userUuid.size()
                                  + component.size()
                                  + errorMsg.size()
                                  + context.size()
                                  + values.size();

    initBlob(result, totalMsgSize);
    appendString(result, userUuid);
    appendString(result, component);
    appendString(result, errorMsg);
    appendString(result, context);
    appendString(result, values);
}

//==================================================================================================

}  // namespace Hanami
}  // namespace Kitsunemimi

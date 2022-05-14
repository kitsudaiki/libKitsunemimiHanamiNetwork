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
 * @brief HanamiMessage::HanamiMessage
 */
HanamiMessage::HanamiMessage() {}

/**
 * @brief HanamiMessage::~HanamiMessage
 */
HanamiMessage::~HanamiMessage() {}

/**
 * @brief HanamiMessage::initBlob
 * @param result
 * @param totalMsgSize
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
 * @brief HanamiMessage::appendString
 * @param result
 * @param val
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
 * @brief HanamiMessage::appendData
 * @param result
 * @param val
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
 * @brief HanamiMessage::initRead
 * @param data
 * @param dataSize
 * @return
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
 * @brief HanamiMessage::readString
 * @param data
 * @param output
 * @return
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
 * @brief HanamiMessage::readBinary
 * @param data
 * @param output
 * @return
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
 * @brief ErrorLog_Message::ErrorLog_Message
 */
ErrorLog_Message::ErrorLog_Message()
{
    m_type = 255;
}

/**
 * @brief ErrorLog_Message::~ErrorLog_Message
 */
ErrorLog_Message::~ErrorLog_Message() {}

/**
 * @brief ErrorLog_Message::read
 * @param data
 * @param dataSize
 * @return
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
 * @brief ErrorLog_Message::createBlob
 * @param result
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

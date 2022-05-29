/**
 * @file        hanami_messages.h
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

#ifndef HANAMI_MESSAGES_H
#define HANAMI_MESSAGES_H

#include <stdint.h>
#include <string>

#include <libKitsunemimiCommon/buffer/data_buffer.h>

namespace Kitsunemimi
{
namespace Hanami
{

class HanamiMessage
{

public:
    enum EntryType
    {
        UNDEFINED_ENTRY_TYPE = 0,
        INT64_ENTRY_TYPE = 1,
        UINT64_ENTRY_TYPE = 2,
        FLOAT64_ENTRY_TYPE = 3,
        BOOL_ENTRY_TYPE = 4,
        STRING_ENTRY_TYPE = 5,
        BYTE_ENTRY_TYPE = 6,
    };

    struct Entry
    {
        uint8_t type = UNDEFINED_ENTRY_TYPE;
        uint8_t padding[7];
        uint64_t valSize = 0;
    }; // size: 16

    struct MessageHeader
    {
        uint8_t type = 0;
        uint8_t padding[7];
        uint64_t messageSize = 0;
    }; // size: 16

    HanamiMessage();
    virtual ~HanamiMessage();

    virtual bool read(void* data, const uint64_t dataSize) = 0;
    virtual void createBlob(DataBuffer &result) = 0;

protected:
    uint64_t m_pos = 0;
    uint8_t m_type = 0;

    void initBlob(DataBuffer &result, const uint64_t totalMsgSize);
    void appendString(DataBuffer &result, const std::string &val);
    void appendData(DataBuffer &result, const void* data, const uint64_t &dataSize);

    bool initRead(const void* data, const uint64_t dataSize);
    bool readString(const void* data, std::string& output);
    bool readBinary(void* data, void** resultData, uint64_t &resultDataSize);
};

//==================================================================================================

class ErrorLog_Message
        : public HanamiMessage
{
public:
    ErrorLog_Message();
    ~ErrorLog_Message();

    std::string userUuid = "";
    std::string component = "";
    std::string errorMsg = "";
    std::string context = "";
    std::string values = "";

    bool read(void* data, const uint64_t dataSize);
    void createBlob(DataBuffer &result);
};

//==================================================================================================

}  // namespace Hanami
}  // namespace Kitsunemimi

#endif // HANAMI_MESSAGES_H

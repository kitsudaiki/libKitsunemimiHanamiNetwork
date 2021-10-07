/**
 * @file        messaging_client.cpp
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

#include <messaging_client.h>

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiJson/json_item.h>

#include <message_handling/message_definitions.h>

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief constructor
 */
MessagingClient::MessagingClient() {}

/**
 * @brief destructor
 */
MessagingClient::~MessagingClient()
{
    if(m_session != nullptr)
    {
        m_session->closeSession(false);
        delete m_session;
    }
}

/**
 * @brief close session
 *
 * @return true, if successful, else false
 */
bool
MessagingClient::closeSession()
{
    if(m_session != nullptr) {
        return m_session->closeSession(false);
    }

    return false;
}

/**
 * @brief trigger sakura-file remotely
 *
 * @param result resulting data-items coming from the triggered tree
 * @param id tree-id to trigger
 * @param inputValues input-values as string
 * @param errorMessage reference for error-output
 *
 * @return true, if successful, else false
 */
bool
MessagingClient::triggerSakuraFile(DataMap &result,
                                   const std::string &id,
                                   const std::string &inputValues,
                                   std::string &errorMessage)
{
    // create buffer
    const uint64_t totalSize = sizeof(SakuraTriggerMessage) + id.size() + inputValues.size();
    uint8_t* buffer = new uint8_t[totalSize];

    // prepare header
    SakuraTriggerMessage header;
    header.idSize = static_cast<uint32_t>(id.size());
    header.inputValuesSize = static_cast<uint32_t>(inputValues.size());

    uint32_t positionCounter = 0;

    // copy header
    memcpy(buffer, &header, sizeof(SakuraTriggerMessage));
    positionCounter += sizeof(SakuraTriggerMessage);

    // copy id
    memcpy(buffer + positionCounter, id.c_str(), id.size());
    positionCounter += id.size();

    // copy input-values
    memcpy(buffer + positionCounter, inputValues.c_str(), inputValues.size());
    positionCounter += inputValues.size();

    // send
    // TODO: make timeout-time configurable
    DataBuffer* response = m_session->sendRequest(buffer, totalSize, 0);
    if(response == nullptr)
    {
        errorMessage = "timeout while triggering sakura-file with id: " + id;
        return false;
    }

    const bool ret = processResponse(result, response, errorMessage);

    delete response;

    return ret;
}

/**
 * @brief process response-message
 *
 * @param result reference for resulting data-items, which are withing the response
 * @param response data-buffer with the plain response message
 * @param errorMessage reference for error-output
 *
 * @return false, if message is invalid or process was not successful, else true
 */
bool
MessagingClient::processResponse(DataMap &result,
                                 DataBuffer* response,
                                 std::string &errorMessage)
{
    // precheck
    if(response->bufferPosition == 0
            || response->data == nullptr)
    {
        // TODO: create error-message
        return false;
    }

    // transform incoming message
    const ResponseMessage* header = static_cast<const ResponseMessage*>(response->data);
    const char* message = static_cast<const char*>(response->data);
    const uint32_t pos = sizeof (ResponseMessage);
    const std::string messageContent(&message[pos], header->messageSize);

    // handle result
    if(header->success)
    {
        Kitsunemimi::Json::JsonItem newItem;
        assert(newItem.parse(messageContent, errorMessage));
        assert(newItem.getItemContent()->isMap());
        result = *newItem.getItemContent()->toMap();
    }
    else
    {
        errorMessage = messageContent;
    }

    return header->success;
}

}
}

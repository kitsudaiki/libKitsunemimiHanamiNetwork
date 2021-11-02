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

#ifndef MESSAGE_IO_H
#define MESSAGE_IO_H

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiJson/json_item.h>

#include <message_handling/message_definitions.h>

#include <libKitsunemimiHanamiCommon/structs.h>

namespace Kitsunemimi
{
namespace Hanami
{

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
processResponse(ResponseMessage& response,
                const DataBuffer* responseData,
                std::string &errorMessage)
{
    // precheck
    if(responseData->usedBufferSize == 0
            || responseData->data == nullptr)
    {
        // TODO: create error-message
        return false;
    }

    // transform incoming message
    const ResponseHeader* header = static_cast<const ResponseHeader*>(responseData->data);
    const char* message = static_cast<const char*>(responseData->data);
    const uint32_t pos = sizeof (ResponseHeader);
    const std::string messageContent(&message[pos], header->messageSize);
    response.type = header->responseType;
    if(response.type != 0)
    {
        response.respnseContent = new DataValue(messageContent);
        return true;
    }

    // handle result
    Kitsunemimi::Json::JsonItem newItem;
    if(newItem.parse(messageContent, errorMessage) == false) {
        return false;
    }

    response.respnseContent = newItem.getItemContent()->copy();

    return true;
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
createRequest(Kitsunemimi::Sakura::Session* session,
              ResponseMessage& response,
              const RequestMessage &request,
              std::string &errorMessage)
{
    // create buffer
    const uint64_t totalSize = sizeof(SakuraTriggerHeader)
                               + request.id.size()
                               + request.inputValues.size();
    uint8_t* buffer = new uint8_t[totalSize];
    uint32_t positionCounter = 0;

    // prepare header
    SakuraTriggerHeader header;
    header.idSize = static_cast<uint32_t>(request.id.size());
    header.requestType = request.httpType;
    header.inputValuesSize = static_cast<uint32_t>(request.inputValues.size());

    // copy header
    memcpy(buffer, &header, sizeof(SakuraTriggerHeader));
    positionCounter += sizeof(SakuraTriggerHeader);

    // copy id
    memcpy(buffer + positionCounter, request.id.c_str(), request.id.size());
    positionCounter += request.id.size();

    // copy input-values
    memcpy(buffer + positionCounter, request.inputValues.c_str(), request.inputValues.size());

    // send
    // TODO: make timeout-time configurable
    DataBuffer* responseData = session->sendRequest(buffer, totalSize, 0);
    if(responseData == nullptr)
    {
        errorMessage = "timeout while triggering sakura-file with id: " + request.id;
        return false;
    }

    const bool ret = processResponse(response, responseData, errorMessage);

    delete responseData;

    return ret;
}

}
}

#endif // MESSAGE_IO_H

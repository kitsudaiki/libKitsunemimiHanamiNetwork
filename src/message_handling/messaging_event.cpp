/**
 * @file        messaging_event.cpp
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

#include "messaging_event.h"

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiJson/json_item.h>

#include <message_handling/message_definitions.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief constructor
 *
 * @param treeId id of the tree to trigger by the event
 * @param inputValues input-values as string
 * @param session pointer to session to send the response back
 * @param blockerId blocker-id for the response
 */
MessagingEvent::MessagingEvent(const HttpRequestType httpType,
                               const std::string &treeId,
                               const std::string &inputValues,
                               Kitsunemimi::Sakura::Session* session,
                               const uint64_t blockerId)
{
    m_httpType = httpType;
    m_treeId = treeId;
    m_inputValues = inputValues;
    m_session = session;
    m_blockerId = blockerId;
}

/**
 * @brief destructor
 */
MessagingEvent::~MessagingEvent() {}

/**
 * @brief send reponse message with the results of the event
 *
 * @param success success-result of the event
 * @param message message to send over the response
 * @param session pointer to session to send the response back
 * @param blockerId blocker-id for the response
 */
void
MessagingEvent::sendResponseMessage(const HttpResponseTypes responseType,
                                    const std::string &message,
                                    Kitsunemimi::Sakura::Session* session,
                                    const uint64_t blockerId)
{
    // allocate memory to fill with the response-message
    const uint32_t responseMessageSize = sizeof(ResponseHeader)
                                         + static_cast<uint32_t>(message.size());
    uint8_t* buffer = new uint8_t[responseMessageSize];

    // prepare response-header
    ResponseHeader responseHeader;
    responseHeader.responseType = responseType;
    responseHeader.messageSize =  static_cast<uint32_t>(message.size());

    uint32_t positionCounter = 0;

    // copy header and id
    memcpy(buffer, &responseHeader, sizeof(ResponseHeader));
    positionCounter += sizeof(ResponseHeader);
    memcpy(buffer + positionCounter, message.c_str(), message.size());

    // send reponse over the session
    session->sendResponse(buffer, responseMessageSize, blockerId);

    delete[] buffer;
}

/**
 * @brief process messageing-event
 *
 * @return true, if event was successful, else false
 */
bool
MessagingEvent::processEvent()
{
    // parse input-values
    DataMap resultingItems;
    std::string errorMessage = "";
    Kitsunemimi::Json::JsonItem newItem;

    // parse json-formated input values
    if(newItem.parse(m_inputValues, errorMessage) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.errorMessage = errorMessage;
        LOG_ERROR(error);
        sendResponseMessage(BAD_REQUEST_RTYPE,
                            error.errorMessage,
                            m_session,
                            m_blockerId);
        return false;
    }

    // get global instances
    Endpoint* endpoints = Endpoint::getInstance();
    SakuraLangInterface* langInterface = SakuraLangInterface::getInstance();

    EndpointEntry entry;
    bool ret = endpoints->mapEndpoint(entry, m_treeId, m_httpType);
    if(ret == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.errorMessage = "endpoint not found for id "
                             + m_treeId
                             + " and type "
                             + std::to_string(m_httpType);
        LOG_ERROR(error);
        sendResponseMessage(NOT_IMPLEMENTED_RTYPE,
                            error.errorMessage,
                            m_session,
                            m_blockerId);
        return false;
    }

    uint64_t status = 0;
    if(entry.type == TREE_TYPE)
    {
        ret = langInterface->triggerTree(resultingItems,
                                         entry.path,
                                         *newItem.getItemContent()->toMap(),
                                         status,
                                         errorMessage);
    }
    else
    {
        ret = langInterface->triggerBlossom(resultingItems,
                                            entry.path,
                                            "special",
                                            *newItem.getItemContent()->toMap(),
                                            status,
                                            errorMessage);
    }

    // creating and send reposonse with the result of the event
    if(ret)
    {
        sendResponseMessage(static_cast<HttpResponseTypes>(status),
                            resultingItems.toString(),
                            m_session,
                            m_blockerId);
    }
    else
    {
        sendResponseMessage(static_cast<HttpResponseTypes>(status),
                            errorMessage,
                            m_session,
                            m_blockerId);
    }

    return true;
}


}
}

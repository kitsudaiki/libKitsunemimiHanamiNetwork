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
#include "permission.h"

#include <message_handling/message_definitions.h>

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <libKitsunemimiHanamiCommon/component_support.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiJson/json_item.h>


using Kitsunemimi::Sakura::SakuraLangInterface;

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief constructor
 *
 * @param targetId id of target to trigger
 * @param inputValues input-values as string
 * @param session pointer to session to send the response back
 * @param blockerId blocker-id for the response
 */
MessagingEvent::MessagingEvent(const HttpRequestType httpType,
                               const std::string &targetId,
                               const std::string &inputValues,
                               Kitsunemimi::Sakura::Session* session,
                               const uint64_t blockerId)
{
    m_httpType = httpType;
    m_targetId = targetId;
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
MessagingEvent::sendResponseMessage(const bool success,
                                    const HttpResponseTypes responseType,
                                    const std::string &message,
                                    Kitsunemimi::Sakura::Session* session,
                                    const uint64_t blockerId,
                                    ErrorContainer &error)
{
    // allocate memory to fill with the response-message
    const uint32_t responseMessageSize = sizeof(ResponseHeader)
                                         + static_cast<uint32_t>(message.size());
    uint8_t* buffer = new uint8_t[responseMessageSize];

    // prepare response-header
    ResponseHeader responseHeader;
    responseHeader.success = success;
    responseHeader.responseType = responseType;
    responseHeader.messageSize =  static_cast<uint32_t>(message.size());

    uint32_t positionCounter = 0;

    // copy header and id
    memcpy(buffer, &responseHeader, sizeof(ResponseHeader));
    positionCounter += sizeof(ResponseHeader);
    memcpy(buffer + positionCounter, message.c_str(), message.size());

    // send reponse over the session
    session->sendResponse(buffer, responseMessageSize, blockerId, error);

    delete[] buffer;
}

/**
 * @brief MessagingEvent::trigger
 * @param newItem
 * @param status
 * @param entry
 * @return
 */
bool
MessagingEvent::trigger(DataMap &resultingItems,
                        Json::JsonItem &newItem,
                        Sakura::BlossomStatus &status,
                        const EndpointEntry &entry)
{
    SakuraLangInterface* langInterface = SakuraLangInterface::getInstance();
    ErrorContainer error;
    DataMap context;

    const std::string token = newItem["token"].getString();
    // token is moved into the context object, so to not break the check of the input-fileds of the
    // blossoms, we have to remove this here again
    // TODO: handle context in a separate field in the messaging
    if(m_targetId != "auth") {
        newItem.remove("token");
    }

    const bool skipPermission = m_session->m_sessionIdentifier != "torii";

    if(m_targetId == "auth"
            || m_targetId == "token"
            || m_targetId == "token/internal"
            || checkPermission(context, token, status, skipPermission, error))
    {
        if(entry.type == TREE_TYPE)
        {
            return langInterface->triggerTree(resultingItems,
                                             entry.name,
                                             context,
                                             *newItem.getItemContent()->toMap(),
                                             status,
                                             error);
        }
        else
        {
            return langInterface->triggerBlossom(resultingItems,
                                                entry.name,
                                                entry.group,
                                                context,
                                                *newItem.getItemContent()->toMap(),
                                                status,
                                                error);
        }
    }

    status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;

    return false;
}

/**
 * @brief process messageing-event
 *
 * @return true, if event was successful, else false
 */
bool
MessagingEvent::processEvent()
{
    ErrorContainer error;

    // parse json-formated input values
    Kitsunemimi::Json::JsonItem newItem;
    if(newItem.parse(m_inputValues, error) == false)
    {
        LOG_ERROR(error);
        sendResponseMessage(false,
                            BAD_REQUEST_RTYPE,
                            error.toString(),
                            m_session,
                            m_blockerId,
                            error);
        return false;
    }

    // get global instances
    Endpoint* endpoints = Endpoint::getInstance();

    // get real endpoint
    EndpointEntry entry;
    bool ret = endpoints->mapEndpoint(entry, m_targetId, m_httpType);
    if(ret == false)
    {
        error.addMeesage("endpoint not found for id "
                         + m_targetId
                         + " and type "
                         + std::to_string(m_httpType));
        LOG_ERROR(error);
        sendResponseMessage(false,
                            NOT_IMPLEMENTED_RTYPE,
                            error.toString(),
                            m_session,
                            m_blockerId,
                            error);
        return false;
    }

    // execute trigger
    Sakura::BlossomStatus status;
    DataMap resultingItems;
    ret = trigger(resultingItems, newItem, status, entry);

    // creating and send reposonse with the result of the event
    const HttpResponseTypes type = static_cast<HttpResponseTypes>(status.statusCode);
    if(ret)
    {
        sendResponseMessage(true,
                            type,
                            resultingItems.toString(),
                            m_session,
                            m_blockerId,
                            error);
    }
    else
    {
        LOG_ERROR(error);
        sendResponseMessage(false,
                            type,
                            status.errorMessage,
                            m_session,
                            m_blockerId,
                            error);
    }

    return true;
}

}  // namespace Hanami
}  // namespace Kitsunemimi

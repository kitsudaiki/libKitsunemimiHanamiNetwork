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

#include <message_handling/message_definitions.h>

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <libKitsunemimiHanamiCommon/component_support.h>

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraLang/blossom.h>
#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCrypto/common.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

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
 * @brief process messageing-event
 *
 * @return true, if event was successful, else false
 */
bool
MessagingEvent::processEvent()
{
    // parse input-values
    DataMap resultingItems;
    ErrorContainer error;
    Kitsunemimi::Json::JsonItem newItem;

    // parse json-formated input values
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
    SakuraLangInterface* langInterface = SakuraLangInterface::getInstance();

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

    DataMap context;
    Sakura::BlossomStatus status;
    ret = false;
    const std::string token = newItem["token"].getString();
    // token is moved into the context object, so to not break the check of the input-fileds of the
    // blossoms, we have to remove this here again
    // TODO: handle context in a separate field in the messaging
    if(m_targetId != "auth") {
        newItem.remove("token");
    }

    if(checkPermission(context, token, status, error))
    {
        if(entry.type == TREE_TYPE)
        {
            ret = langInterface->triggerTree(resultingItems,
                                             entry.name,
                                             context,
                                             *newItem.getItemContent()->toMap(),
                                             status,
                                             error);
        }
        else
        {
            ret = langInterface->triggerBlossom(resultingItems,
                                                entry.name,
                                                entry.group,
                                                context,
                                                *newItem.getItemContent()->toMap(),
                                                status,
                                                error);
        }
    }

    // creating and send reposonse with the result of the event
    const HttpResponseTypes type = static_cast<HttpResponseTypes>(status.statusCode);
    if(ret)
    {
        sendResponseMessage(true, type,
                            resultingItems.toString(),
                            m_session,
                            m_blockerId,
                            error);
    }
    else
    {
        LOG_ERROR(error);
        sendResponseMessage(false, type,
                            status.errorMessage,
                            m_session,
                            m_blockerId,
                            error);
    }

    return true;
}

/**
 * @brief MessagingEvent::checkPermission
 * @param parsedResult
 * @param token
 * @param status
 * @param error
 * @return
 */
bool
MessagingEvent::checkPermission(DataMap &context,
                                const std::string &token,
                                Sakura::BlossomStatus &status,
                                Kitsunemimi::ErrorContainer &error)
{
    // filter actions, which do not need a token in its context
    if(m_targetId == "auth"
            || m_targetId == "token")
    {
        return true;
    }

    // precheck
    if(token == "")
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Token is missing in request");
        return false;
    }

    Kitsunemimi::Json::JsonItem parsedResult;

    // only get token content without validation, if misaka is not supported
    if(supportedComponents.support[MISAKA] == false)
    {
        if(getJwtTokenPayload(parsedResult, token, error) == false) {
            return false;
        }
    }
    else
    {
        if(getPermission(parsedResult, token, status, error) == false) {
            return false;
        }
    }

    context = *parsedResult.getItemContent()->toMap();
    context.insert("token", new DataValue(token));

    return true;
}

/**
 * @brief HanamiMessaging::getJwtTokenPayload
 * @param resultPayload
 * @param token
 * @param error
 * @return
 */
bool
MessagingEvent::getJwtTokenPayload(Json::JsonItem &parsedResult,
                                   const std::string &token,
                                   ErrorContainer &error)
{
    std::vector<std::string> tokenParts;
    Kitsunemimi::splitStringByDelimiter(tokenParts, token, '.');
    if(tokenParts.size() != 3)
    {
        error.addMeesage("Token is broken");
        LOG_ERROR(error);
        return false;
    }

    std::string payloadString = tokenParts.at(1);
    Kitsunemimi::Crypto::base64UrlToBase64(payloadString);
    Kitsunemimi::Crypto::decodeBase64(payloadString, payloadString);
    if(parsedResult.parse(payloadString, error) == false)
    {
        error.addMeesage("Token-payload is broken");
        LOG_ERROR(error);
        return false;
    }

    return true;
}

/**
 * @brief MessagingEvent::getPermission
 * @param parsedResult
 * @param token
 * @param status
 * @param error
 * @return
 */
bool
MessagingEvent::getPermission(Json::JsonItem &parsedResult,
                              const std::string &token,
                              Sakura::BlossomStatus &status,
                              ErrorContainer &error)
{
    Kitsunemimi::Hanami::RequestMessage requestMsg;
    Kitsunemimi::Hanami::ResponseMessage responseMsg;
    Hanami::HanamiMessaging* messaging = Hanami::HanamiMessaging::getInstance();

    requestMsg.id = "auth";
    requestMsg.httpType = HttpRequestType::GET_TYPE;
    requestMsg.inputValues = "{\"token\":\"" + token + "\"}";

    // send request to misaka
    if(messaging->triggerSakuraFile("misaka", responseMsg, requestMsg, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Unable to validate token");
        return false;
    }

    // handle failed authentication
    if(responseMsg.type == Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE
            || responseMsg.success == false)
    {
        status.statusCode = responseMsg.type;
        status.errorMessage = responseMsg.responseContent;
        error.addMeesage(responseMsg.responseContent);
        return false;
    }

    if(parsedResult.parse(responseMsg.responseContent, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Unable to parse auth-reponse.");
        return false;
    }

    return true;
}

}
}

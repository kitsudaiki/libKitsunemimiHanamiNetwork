/**
 * @file        permission.cpp
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

#include "permission.h"

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraLang/blossom.h>
#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiJwt/jwt.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_items/data_items.h>

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief MessagingEvent::checkPermission
 * @param parsedResult
 * @param token
 * @param status
 * @param error
 * @return
 */
bool
checkPermission(DataMap &context,
                const std::string &token,
                Sakura::BlossomStatus &status,
                const bool skipPermission,
                Kitsunemimi::ErrorContainer &error)
{
    // precheck
    if(token == "")
    {
        status.statusCode = Kitsunemimi::Hanami::BAD_REQUEST_RTYPE;
        status.errorMessage = "Token is required but missing in the request.";
        error.addMeesage("Token is missing in request");
        return false;
    }

    Kitsunemimi::Json::JsonItem parsedResult;

    // only get token content without validation, if misaka is not supported
    if(skipPermission
            || SupportedComponents::getInstance()->support[MISAKA] == false)
    {
        if(Kitsunemimi::Jwt::getJwtTokenPayload(parsedResult, token, error) == false)
        {
            status.statusCode = Kitsunemimi::Hanami::BAD_REQUEST_RTYPE;
            return false;
        }
    }
    else
    {
        if(getPermission(parsedResult, token, status, error) == false)
        {
            status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
            return false;
        }
    }

    // fill context-object
    context = *parsedResult.getItemContent()->toMap();
    context.insert("token", new DataValue(token));

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
getPermission(Json::JsonItem &parsedResult,
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
        error.addMeesage("Unable send validation for token-request.");
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

}  // namespace Hanami
}  // namespace Kitsunemimi

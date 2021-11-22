/**
 * @file        hanami_blossom.cpp
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

#include <libKitsunemimiHanamiMessaging/hanami_blossom.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiJson/json_item.h>

namespace Kitsunemimi
{
namespace Hanami
{

HanamiBlossom::HanamiBlossom(const bool requireToken)
{
    registerInputField("token", requireToken);
}

/**
 * @brief HanamiBlossom::checkPermission
 * @param parsedResult
 * @param token
 * @param status
 * @param error
 * @return
 */
bool
HanamiBlossom::checkPermission(Kitsunemimi::Json::JsonItem &parsedResult,
                               const std::string &token,
                               Sakura::BlossomStatus &status,
                               Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::Hanami::RequestMessage requestMsg;
    Kitsunemimi::Hanami::ResponseMessage responseMsg;
    Hanami::HanamiMessaging* messaging = Hanami::HanamiMessaging::getInstance();

    requestMsg.id = "auth";
    requestMsg.httpType = HttpRequestType::GET_TYPE;
    requestMsg.inputValues = "{\"token\":\"" + token + "\"}";

    if(messaging->triggerSakuraFile("Misaka", responseMsg, requestMsg, error) == false)
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

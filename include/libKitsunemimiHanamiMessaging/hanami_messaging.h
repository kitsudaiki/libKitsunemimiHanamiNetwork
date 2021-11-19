/**
 * @file        messaging_controller.h
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

#ifndef KITSUNEMIMI_SAKURA_MESSAGING_MESSAGING_CONTROLLER_H
#define KITSUNEMIMI_SAKURA_MESSAGING_MESSAGING_CONTROLLER_H

#include <iostream>
#include <map>
#include <vector>
#include <regex>

#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
struct DataBuffer;
class DataMap;
namespace Sakura {
class Blossom;
class Session;
class SessionController;
}
namespace Hanami
{

class HanamiMessaging
{

public:
    static HanamiMessaging* getInstance();

    ~HanamiMessaging();

    bool initialize(const std::string &identifier,
                    const std::vector<std::string> &configGroups,
                    ErrorContainer &error,
                    const bool createServer = true);

    bool triggerSakuraFile(const std::string &target,
                           ResponseMessage &response,
                           const RequestMessage &request,
                           ErrorContainer &error);

    bool closeClient(const std::string &remoteIdentifier,
                     ErrorContainer &error);

private:
    HanamiMessaging();

    Kitsunemimi::Sakura::SessionController* m_sessionController = nullptr;
    std::map<std::string, Sakura::Session*> m_outgoingClients;

    std::string m_localIdentifier = "";
    bool m_isInit = false;

    static HanamiMessaging* m_messagingController;

    bool createClient(const std::string &clientName,
                      const std::string &address,
                      ErrorContainer &error,
                      const uint16_t port = 0);
};

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_MESSAGING_CONTROLLER_H

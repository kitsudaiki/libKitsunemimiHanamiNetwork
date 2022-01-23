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

#ifndef KITSUNEMIMI_HANAMI_MESSAGING_MESSAGING_CONTROLLER_H
#define KITSUNEMIMI_HANAMI_MESSAGING_MESSAGING_CONTROLLER_H

#include <iostream>
#include <map>
#include <vector>
#include <regex>

#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
struct DataBuffer;
class DataMap;
struct StackBuffer;
namespace Sakura {
class Blossom;
class Session;
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
                    void* receiver,
                    void (*processStream)(void*, Sakura::Session*, const void*, const uint64_t),
                    ErrorContainer &error,
                    const bool createServer = true,
                    const std::string &predefinedEndpoints = "");
    bool addServer(const std::string &serverAddress,
                   ErrorContainer &error,
                   const uint16_t port = 0,
                   const std::string &certFilePath = "",
                   const std::string &keyFilePath = "");

    bool sendStreamMessage(const std::string &target,
                           StackBuffer &data,
                           ErrorContainer &error);
    bool sendStreamMessage(const std::string &target,
                           const void* data,
                           const uint64_t dataSize,
                           const bool replyExpected,
                           ErrorContainer &error);

    bool triggerSakuraFile(const std::string &target,
                           ResponseMessage &response,
                           const RequestMessage &request,
                           ErrorContainer &error);

    bool closeClient(const std::string &remoteIdentifier,
                     ErrorContainer &error);

    Sakura::Session* getOutgoingSession(const std::string identifier);
    Sakura::Session* getIncomingSession(const std::string identifier);

private:
    HanamiMessaging();

    bool m_isInit = false;

    void fillSupportOverview();
    bool initClients(const std::vector<std::string> &configGroups);
    bool initEndpoints(ErrorContainer &error,
                       const std::string &predefinedEndpoints);

    static HanamiMessaging* m_messagingController;
    void createBlossomDocu(Sakura::Blossom* blossom, std::string &docu);
};

}  // namespace Hanami
}  // namespace Kitsunemimi

#endif // KITSUNEMIMI_HANAMI_MESSAGING_MESSAGING_CONTROLLER_H

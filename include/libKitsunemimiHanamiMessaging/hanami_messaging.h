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
class SessionController;
}
namespace Hanami
{
class HanamiMessagingClient;

class HanamiMessaging
{

public:
    static HanamiMessaging* getInstance();

    ~HanamiMessaging();

    bool initialize(const std::string &identifier,
                    const std::vector<std::string> &configGroups,
                    void* receiver,
                    void (*processStream)(void*,
                                          Sakura::Session*,
                                          const void*,
                                          const uint64_t),
                    void (*processGenericRequest)(Sakura::Session*,
                                                  const Kitsunemimi::Json::JsonItem&,
                                                  const uint64_t),
                    ErrorContainer &error,
                    const bool createServer = true,
                    const std::string &predefinedEndpoints = "");
    bool addServer(const std::string &serverAddress,
                   ErrorContainer &error,
                   const uint16_t port = 0,
                   const std::string &certFilePath = "",
                   const std::string &keyFilePath = "");

    bool closeClient(const std::string &remoteIdentifier,
                     ErrorContainer &error);

    HanamiMessagingClient* getOutgoingClient(const std::string &identifier);
    HanamiMessagingClient* getIncomingClient(const std::string &identifier);

    void sendGenericErrorMessage(const std::string &errorMessage);

    static Kitsunemimi::Sakura::SessionController* m_sessionController;

    HanamiMessagingClient* misakaClient = nullptr;
    HanamiMessagingClient* sagiriClient = nullptr;
    HanamiMessagingClient* kyoukoClient = nullptr;
    HanamiMessagingClient* azukiClient  = nullptr;
    HanamiMessagingClient* nozomiClient = nullptr;
    HanamiMessagingClient* izumiClient  = nullptr;
    HanamiMessagingClient* toriiClient  = nullptr;

    //=====================================================================
    // ALL BELOW IS INTERNAL AND SHOULD NEVER BE USED BY EXTERNAL METHODS!
    //=====================================================================
    bool addInternalClient(const std::string &identifier,
                           Sakura::Session* newSession);
    bool removeInternalClient(const std::string &identifier);

    void* streamReceiver = nullptr;
    void (*processStreamData)(void*,
                              Sakura::Session*,
                              const void*,
                              const uint64_t);
    void (*processGenericRequest)(Sakura::Session*,
                                  const Kitsunemimi::Json::JsonItem&,
                                  const uint64_t);


private:
    HanamiMessaging();

    bool m_isInit = false;
    bool m_whileSendError = false;

    // session-handling
    std::map<std::string, HanamiMessagingClient*> m_clients;
    std::map<std::string, HanamiMessagingClient*> m_incomingClients;
    std::string m_localIdentifier = "";
    std::mutex m_incominglock;

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

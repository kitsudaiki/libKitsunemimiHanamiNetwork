/**
 * @file        client_handler.h
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

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <map>
#include <string>
#include <vector>
#include <mutex>

#include <libKitsunemimiCommon/threading/thread.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Json {
class JsonItem;
}
namespace Sakura {
class Session;
class SessionController;
}
namespace Hanami
{
class MessagingClient;

class ClientHandler
        : public Kitsunemimi::Thread
{

public:
    struct ClientInformation
    {
        std::string remoteIdentifier = "";
        std::string address = "";
        uint16_t port = 0;
        Sakura::Session* session = nullptr;
    };

    ClientHandler(const std::string &localIdentifier);
    static ClientHandler* m_instance;
    static Kitsunemimi::Sakura::SessionController* m_sessionController;

    bool addOutgoingClient(const std::string &remoteIdentifier,
                           const std::string &address,
                           const uint16_t port);
    Sakura::Session* getOutgoingSession(const std::string &identifier);
    Sakura::Session* getIncomingSession(const std::string &identifier);
    bool closeClient(const std::string &identifier,
                     ErrorContainer &error,
                     const bool earseFromList);

    bool waitForAllConnected(const uint32_t timeout);

    bool addInternalClient(const std::string &identifier,
                           Sakura::Session* newClient);

    bool removeInternalClient(const std::string &identifier);

    void* streamReceiver = nullptr;
    void (*processStreamData)(void*,
                              Sakura::Session*,
                              const void*,
                              const uint64_t);
    void (*processGenericRequest)(Sakura::Session*,
                                  const Kitsunemimi::Json::JsonItem&,
                                  const uint64_t);

protected:
    void run();

private:
    bool connectClient(ClientInformation &info, ErrorContainer &error);

    std::map<std::string, ClientInformation> m_outgoingClients;
    std::map<std::string, Sakura::Session*> m_incomingClients;

    std::vector<Sakura::Session*> m_forDeletion;

    std::string m_localIdentifier = "";
    std::mutex m_outgoinglock;
    std::mutex m_incominglock;
    std::mutex m_deletionMutex;
};

}  // namespace Hanami
}  // namespace Kitsunemimi

#endif // CLIENTHANDLER_H

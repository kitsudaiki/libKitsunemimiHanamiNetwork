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

namespace Kitsunemimi
{
struct DataBuffer;
namespace Sakura {
class Blossom;
class Session;
class SessionController;
}
namespace Hanami
{
class MessagingClient;

class MessagingController
{

public:
    static bool initializeMessagingController(const std::string& identifier,
                                              const std::vector<std::string> &configGroups,
                                              void (*processCreateSession)(MessagingClient*,
                                                                             const std::string),
                                              void (*processCloseSession)(const std::string),
                                              const bool createServer = true);
    static MessagingController* getInstance();

    ~MessagingController();

    MessagingClient* createClient(const std::string &clientName,
                                  const std::string &identifier,
                                  const std::string &address,
                                  const uint16_t port = 0);


    // internally
    MessagingClient* createClient(const std::string &identifier,
                                  Kitsunemimi::Sakura::Session* session);

    void closeClient(const std::string &remoteIdentifier);

private:
    MessagingController();

    Kitsunemimi::Sakura::SessionController* m_controller = nullptr;
    std::string m_localIdentifier = "";

    void (*m_processCreationSession)(MessagingClient*, const std::string);
    void (*m_processClosingSession)(const std::string);

    static MessagingController* m_instance;
};

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_MESSAGING_CONTROLLER_H

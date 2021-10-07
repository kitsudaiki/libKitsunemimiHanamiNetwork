/**
 * @file        internal_client_handler.cpp
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

#include "internal_client_handler.h"

#include <messaging_client.h>

namespace Kitsunemimi
{
namespace Hanami
{

InternalClientHandler* InternalClientHandler::m_instance = new InternalClientHandler();

InternalClientHandler::InternalClientHandler() {}

/**
 * @brief InternalClientHandler::addClient
 * @param identifier
 * @param newClient
 * @return
 */
bool
InternalClientHandler::addClient(const std::string &identifier,
                                 MessagingClient* newClient)
{
    std::map<std::string, MessagingClient*>::const_iterator it;
    it = m_incomingClients.find(identifier);

    if(it != m_incomingClients.end()) {
        return false;
    }

    m_incomingClients.insert(std::make_pair(identifier, newClient));

    return true;
}

/**
 * @brief InternalClientHandler::removeClient
 * @param identifier
 * @return
 */
bool
InternalClientHandler::removeClient(const std::string &identifier)
{
    std::map<std::string, MessagingClient*>::const_iterator it;
    it = m_incomingClients.find(identifier);

    if(it != m_incomingClients.end())
    {
        if(it->second != nullptr) {
            delete it->second;
        }

        m_incomingClients.erase(it);

        return true;
    }

    return false;
}

}
}

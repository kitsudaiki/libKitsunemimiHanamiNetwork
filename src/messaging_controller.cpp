/**
 * @file        messaging_controller.cpp
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

#include <libKitsunemimiHanamiMessaging/messaging_controller.h>
#include <libKitsunemimiHanamiMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

#include <libKitsunemimiCommon/logger.h>

#include <callbacks.h>
#include <config.h>

using Kitsunemimi::Sakura::SessionController;

namespace Kitsunemimi
{
namespace Hanami
{

Kitsunemimi::Hanami::MessagingController* MessagingController::m_instance = nullptr;

/**
 * @brief constructor
 */
MessagingController::MessagingController()
{
    m_controller = new SessionController(&sessionCreateCallback,
                                         &sessionCloseCallback,
                                         &errorCallback);
}

/**
 * @brief destructor
 */
MessagingController::~MessagingController()
{
    delete m_controller;
}

/**
 * @brief create and initialize new messaging-controller
 *
 * @param localIdentifier identifier for outgoing sessions to identify against the servers
 * @param configGroups config-groups for automatic creation of server and clients
 * @param processCreateSession callback for creating new session
 * @param processCloseSession callback for closing a session
 * @param createServer true, if the instance should also create a server
 *
 * @return true, if successful, else false
 */
bool
MessagingController::initializeMessagingController(const std::string &localIdentifier,
                                                   const std::vector<std::string> &configGroups,
                                                   void (*processCreateSession)(MessagingClient*,
                                                                                const std::string),
                                                   void (*processCloseSession)(const std::string),
                                                   const bool createServer)
{
    // precheck to avoid double-initializing
    if(m_instance != nullptr) {
        return false;
    }

    // set global values
    m_instance = new MessagingController();
    m_instance->m_localIdentifier = localIdentifier;
    m_instance->m_processCreationSession = processCreateSession;
    m_instance->m_processClosingSession = processCloseSession;

    // check if config-file already initialized
    if(Kitsunemimi::Config::ConfigHandler::m_config == nullptr)
    {
        LOG_ERROR("config-file not initilized");
        return false;
    }

    // init config-options
    registerConfigs(configGroups);
    if(checkConfigs(configGroups, createServer) == false) {
        return false;
    }

    bool success = false;
    const std::regex ipv4Regex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}"
                               "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    // init server if requested
    if(createServer)
    {
        const std::string serverAddress = GET_STRING_CONFIG("DEFAULT", "address", success);

        // init server based on the type of the address in the config
        if(regex_match(serverAddress, ipv4Regex))
        {
            // create tcp-server
            const int port = GET_INT_CONFIG("DEFAULT", "port", success);
            const uint16_t serverPort = static_cast<uint16_t>(port);
            if(m_instance->m_controller->addTcpServer(serverPort) == 0)
            {
                LOG_ERROR("can't initialize tcp-server on port " + std::to_string(serverPort));
                return false;
            }
        }
        else
        {
            // create uds-server
            if(m_instance->m_controller->addUnixDomainServer(serverAddress) == 0)
            {
                LOG_ERROR("can't initialize uds-server on file " + serverAddress);
                return false;
            }
        }
    }

    // init client-connections
    for(const std::string& groupName : configGroups)
    {
        const std::string address = GET_STRING_CONFIG(groupName, "address", success);
        const uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG(groupName, "port", success));
        MessagingClient* newClient = m_instance->createClient(groupName,
                                                              localIdentifier,
                                                              address,
                                                              port);
        if(newClient == nullptr) {
            return false;
        }
    }

    return true;
}

/**
 * @brief get instance, which must be already initialized
 *
 * @return instance-object
 */
MessagingController*
MessagingController::getInstance()
{
    return m_instance;
}

/**
 * @brief create new client
 *
 * @param remoteIdentifier identifier to link the session with a target-name
 * @param localIdentifier identifier to indentify agains the server
 * @param address ipv4-address of the tcp-server or path to the uds-server
 * @param port port where the server is listening
 *
 * @return pointer tho the new created client, nullptr if creation failed
 */
MessagingClient*
MessagingController::createClient(const std::string &remoteIdentifier,
                                  const std::string &localIdentifier,
                                  const std::string &address,
                                  const uint16_t port)
{
    const std::regex ipv4Regex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}"
                               "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    Kitsunemimi::Sakura::Session* newSession = nullptr;
    if(regex_match(address, ipv4Regex)) {
        newSession = m_instance->m_controller->startTcpSession(address, port, localIdentifier);
    } else {
        newSession = m_instance->m_controller->startUnixDomainSession(address, localIdentifier);
    }

    if(newSession == nullptr) {
        return nullptr;
    }

    return createClient(remoteIdentifier, newSession);
}

/**
* @brief create and forward new client
*
* @param remoteIdentifier name of the client for later identification
* @param session session for the new client
*
* @return new client, or nullptr, if client-name already exist
*/
MessagingClient*
MessagingController::createClient(const std::string &remoteIdentifier,
                                  Kitsunemimi::Sakura::Session* session)
{

    MessagingClient* client = new MessagingClient();
    client->m_session = session;

    m_instance->m_processCreationSession(client, remoteIdentifier);

    return client;
}

/**
 * @brief forward callback to close session
 *
* @param remoteIdentifier name of the client for later identification
 */
void
MessagingController::closeClient(const std::string &remoteIdentifier)
{
    m_instance->m_processClosingSession(remoteIdentifier);
}

}
}

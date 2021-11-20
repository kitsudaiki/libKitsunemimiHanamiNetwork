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

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <message_io.h>
#include <client_handler.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

#include <libKitsunemimiHanamiCommon/config.h>
#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiConfig/config_handler.h>


using Kitsunemimi::Sakura::SessionController;

namespace Kitsunemimi
{
namespace Hanami
{

HanamiMessaging* HanamiMessaging::m_messagingController = nullptr;

/**
 * @brief constructor
 */
HanamiMessaging::HanamiMessaging()
{

}

/**
 * @brief destructor
 */
HanamiMessaging::~HanamiMessaging()
{
}

/**
 * @brief create and initialize new messaging-controller
 *
 * @param localIdentifier identifier for outgoing sessions to identify against the servers
 * @param configGroups config-groups for automatic creation of server and clients
 * @param createServer true, if the instance should also create a server
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::initialize(const std::string &localIdentifier,
                            const std::vector<std::string> &configGroups,
                            ErrorContainer &error,
                            const bool createServer,
                            const std::string &predefinedEndpoints)
{
    // precheck to avoid double-initializing
    if(m_isInit) {
        return false;
    }

    // set global values
    ClientHandler::m_instance = new ClientHandler(localIdentifier);
    ClientHandler::m_instance->startThread();
    ;

    // check if config-file already initialized
    if(Kitsunemimi::Config::ConfigHandler::m_config == nullptr)
    {
        error.addMeesage("config-file not initilized");
        LOG_ERROR(error);
        return false;
    }

    // init config-options
    registerBasicConnectionConfigs(configGroups, createServer, error);
    if(Kitsunemimi::Config::ConfigHandler::m_config->isConfigValid() == false) {
        return false;
    }

    bool success = false;
    const std::regex ipv4Regex("\\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}\\b");

    // init server if requested
    if(createServer)
    {
        const std::string serverAddress = GET_STRING_CONFIG("DEFAULT", "address", success);
        Endpoint* endpoints = Endpoint::getInstance();

        std::string endpointContent = predefinedEndpoints;
        if(endpointContent == "")
        {
            const std::string endpointPath = GET_STRING_CONFIG("DEFAULT", "endpoints", success);
            if(Kitsunemimi::readFile(endpointContent, endpointPath, error) == false)
            {
                LOG_ERROR(error);
                return false;
            }
        }

        if(endpoints->parse(endpointContent, error) == false)
        {
            LOG_ERROR(error);
            return false;
        }

        // init server based on the type of the address in the config
        if(regex_match(serverAddress, ipv4Regex))
        {
            // create tcp-server
            const int port = GET_INT_CONFIG("DEFAULT", "port", success);
            const uint16_t serverPort = static_cast<uint16_t>(port);
            if(ClientHandler::m_sessionController->addTcpServer(serverPort, error) == 0)
            {
                error.addMeesage("can't initialize tcp-server on port "
                                 + std::to_string(serverPort));
                LOG_ERROR(error);
                return false;
            }
        }
        else
        {
            // create uds-server
            if(ClientHandler::m_sessionController->addUnixDomainServer(serverAddress, error) == 0)
            {
                error.addMeesage("can't initialize uds-server on file " + serverAddress);
                LOG_ERROR(error);
                return false;
            }
        }
    }

    // init client-connections
    for(const std::string& groupName : configGroups)
    {
        const std::string address = GET_STRING_CONFIG(groupName, "address", success);
        const uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG(groupName, "port", success));
        if(ClientHandler::m_instance->addOutgoingClient(groupName, address, port) == false) {
            return false;
        }
    }

    // wait for 1 secont at maximum
    ClientHandler::m_instance->waitForAllConnected(1);

    m_isInit = true;

    return true;
}

/**
 * @brief HanamiMessaging::triggerSakuraFile
 * @param target
 * @param response
 * @param request
 * @param error
 * @return
 */
bool
HanamiMessaging::triggerSakuraFile(const std::string &target,
                                   ResponseMessage& response,
                                   const RequestMessage &request,
                                   ErrorContainer &error)
{
    LOG_DEBUG("trigger sakura-file \'" + request.id + "\' on target \'" + target + "\'");

    Sakura::Session* client = ClientHandler::m_instance->getSession(target);
    if(client != nullptr)
    {
        if(createRequest(client, response, request, error) == false)
        {
            // in case of an error, try to wait for a reconnect and try again
            ClientHandler::m_instance->waitForAllConnected(1);
            return createRequest(client, response, request, error);
        }
        return true;
    }

    response.success = false;
    response.type = NOT_FOUND_RTYPE;
    response.responseContent = "target \'" + target + "\' not found";

    return false;
}

/**
 * @brief HanamiMessaging::closeClient
 * @param remoteIdentifier
 * @param error
 * @return
 */
bool
HanamiMessaging::closeClient(const std::string &remoteIdentifier,
                             ErrorContainer &error)
{
    return ClientHandler::m_instance->closeClient(remoteIdentifier, error, true);
}

/**
 * @brief get instance, which must be already initialized
 *
 * @return instance-object
 */
HanamiMessaging*
HanamiMessaging::getInstance()
{
    if(m_messagingController == nullptr) {
        m_messagingController = new HanamiMessaging();
    }
    return m_messagingController;
}

}
}

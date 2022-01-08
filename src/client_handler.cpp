/**
 * @file        client_handler.cpp
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

#include "client_handler.h"
#include <callbacks.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

namespace Kitsunemimi
{
namespace Hanami
{
ClientHandler* ClientHandler::m_instance = nullptr;
Kitsunemimi::Sakura::SessionController* ClientHandler::m_sessionController = nullptr;

/**
 * @brief constructor
 *
 * @param localIdentifier name for the lokal component for identification against other components
 */
ClientHandler::ClientHandler(const std::string &localIdentifier)
    : Kitsunemimi::Thread("HanamiClientHandler")
{
    m_localIdentifier = localIdentifier;
    m_sessionController = new Sakura::SessionController(&sessionCreateCallback,
                                                        &sessionCloseCallback,
                                                        &errorCallback);
}

/**
 * @brief create a new connection to a client
 *
 * @param info object with all information for the connection
 * @param error reference for error-ourput
 *
 * @return
 */
bool
ClientHandler::connectClient(ClientInformation &info,
                             ErrorContainer &error)
{
    LOG_DEBUG("create client with remote-identifier \""
              + info.remoteIdentifier
              + "\" and address\""
              + info.address
              + "\"");

    Kitsunemimi::Sakura::Session* newSession = nullptr;

    // connect based on the address-type
    const std::regex ipv4Regex("\\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}\\b");
    if(regex_match(info.address, ipv4Regex))
    {
        newSession = m_sessionController->startTcpSession(info.address,
                                                          info.port,
                                                          m_localIdentifier,
                                                          "HanamiClient",
                                                          error);
    }
    else
    {
        newSession = m_sessionController->startUnixDomainSession(info.address,
                                                                 m_localIdentifier,
                                                                 "HanamiClient",
                                                                 error);
    }

    // check if connection was successful
    if(newSession == nullptr) {
        return false;
    }

    // handle result
    newSession->m_sessionIdentifier = info.remoteIdentifier;
    info.session = newSession;

    return true;
}


/**
 * @brief forward callback to close session
 *
 * @param identifier name of the client for later identification
 * @param earseFromList set to true to avoid reconnects
 */
bool
ClientHandler::closeClient(const std::string &identifier,
                           ErrorContainer &,
                           const bool earseFromList)
{
    LOG_DEBUG("close client with remote-identifier \"" + identifier + "\"");

    std::lock_guard<std::mutex> guard(m_outgoinglock);

    std::map<std::string, ClientInformation>::iterator it;
    it = m_outgoingClients.find(identifier);
    if(it != m_outgoingClients.end())
    {
        if(it->second.session != nullptr)
        {
            ErrorContainer error;
            if(it->second.session->closeSession(error) == false) {
                LOG_ERROR(error);
            }

            LOG_DEBUG("schedule client with remote-identifier \""
                      + identifier
                      + "\" for deletion");
            m_deletionMutex.lock();
            m_forDeletion.push_back(it->second.session);
            m_deletionMutex.unlock();
            it->second.session = nullptr;
        }

        if(earseFromList) {
            m_outgoingClients.erase(it);
        }

        return true;
    }

    return false;
}

/**
 * @brief wait until all outging connections are connected
 *
 * @param timeout number of seconds to wait for a timeout
 *
 * @return true, if all are connected, else false
 */
bool
ClientHandler::waitForAllConnected(const uint32_t timeout)
{
    const uint64_t microTimeout = timeout * 1000000;
    uint64_t actualTimeout = 0;

    while(m_abort == false
            && actualTimeout < microTimeout)
    {
        bool allConnected = true;

        m_outgoinglock.lock();
        std::map<std::string, ClientInformation>::iterator it;
        for(it = m_outgoingClients.begin();
            it != m_outgoingClients.end();
            it++)
        {
            if(it->second.session == nullptr)
            {
                allConnected = false;
                break;
            }
        }
        m_outgoinglock.unlock();

        if(allConnected) {
            return true;
        }

        sleepThread(10000);
        actualTimeout += 10000;
    }

    return false;
}

/**
 * @brief register an incoming connection
 *
 * @param identifier identifier for the new incoming connection
 * @param newClient pointer to the session
 *
 * @return true, if successful, else false
 */
bool
ClientHandler::addInternalClient(const std::string &identifier,
                                 Sakura::Session* newClient)
{
    m_incominglock.lock();

    std::map<std::string, Sakura::Session*>::const_iterator it;
    it = m_incomingClients.find(identifier);

    if(it != m_incomingClients.end())
    {
        m_incominglock.unlock();

        ErrorContainer error;
        newClient->closeSession(error);
        LOG_ERROR(error);
        m_deletionMutex.lock();
        m_forDeletion.push_back(newClient);
        m_deletionMutex.unlock();
        return false;
    }

    m_incomingClients.insert(std::make_pair(identifier, newClient));

    m_incominglock.unlock();

    return true;
}

/**
 * @brief remove the client of an incoming connection
 *
 * @param identifier identifier for the internal client
 *
 * @return true, if successful, else false
 */
bool
ClientHandler::removeInternalClient(const std::string &identifier)
{
    m_incominglock.lock();

    std::map<std::string, Sakura::Session*>::iterator it;
    it = m_incomingClients.find(identifier);
    if(it != m_incomingClients.end())
    {
        Sakura::Session* tempSession = it->second;
        m_incomingClients.erase(it);

        m_incominglock.unlock();

        if(tempSession != nullptr)
        {
            ErrorContainer error;
            if(tempSession->closeSession(error) == false) {
                LOG_ERROR(error);
            }

            m_deletionMutex.lock();
            m_forDeletion.push_back(tempSession);
            m_deletionMutex.unlock();
        }

        return true;
    }

    m_incominglock.unlock();

    return false;
}

/**
 * @brief register a outgoing connection
 *
 * @param remoteIdentifier indentifier for the connection
 * @param address address of the target
 * @param port port in case of a tcp-connection
 *
 * @return true, if successful, else false
 */
bool
ClientHandler::addOutgoingClient(const std::string &remoteIdentifier,
                                 const std::string &address,
                                 const uint16_t port)
{
    std::lock_guard<std::mutex> guard(m_outgoinglock);

    std::map<std::string, ClientInformation>::const_iterator it;
    it = m_outgoingClients.find(remoteIdentifier);
    if(it != m_outgoingClients.end()) {
        return false;
    }

    ClientInformation newClient;
    newClient.remoteIdentifier = remoteIdentifier;
    newClient.address = address;
    newClient.port = port;

    m_outgoingClients.insert(std::make_pair(remoteIdentifier, newClient));

    return true;
}

/**
 * @brief request the session of an outgoing connection
 *
 * @param identifier identifier of the connection
 *
 * @return nullptr, if session for the identifier was not found, else pointer to the found session
 */
Sakura::Session*
ClientHandler::getSession(const std::string &identifier)
{
    std::lock_guard<std::mutex> guard(m_outgoinglock);

    std::map<std::string, ClientInformation>::const_iterator it;
    it = m_outgoingClients.find(identifier);
    if(it != m_outgoingClients.end()) {
        return it->second.session;
    }

    return nullptr;
}

/**
 * @brief thread to close and delete connections, which are marked by a close-call or timeout,
 *        and to reconnect outgoing connections
 */
void
ClientHandler::run()
{
    while(m_abort == false)
    {
        m_outgoinglock.lock();
        m_deletionMutex.lock();

        // close marked sessions
        for(Sakura::Session* session : m_forDeletion)
        {
            LOG_DEBUG("delete session '" + session->m_sessionIdentifier + "'");
            delete session;
        }
        m_forDeletion.clear();

        m_deletionMutex.unlock();

        // reconnect outgoing connections
        std::map<std::string, ClientInformation>::iterator it;
        for(it = m_outgoingClients.begin();
            it != m_outgoingClients.end();
            it++)
        {
            if(it->second.session == nullptr)
            {
                ErrorContainer error;
                if(connectClient(it->second, error) == false)
                {
                    error.addMeesage("create connection to '" + it->first + "' failed");
                    error.addSolution("check if component '" + it->first + "' is up and running.");
                    LOG_ERROR(error);
                }
            }
        }

        m_outgoinglock.unlock();

        // wait for 100ms
        sleepThread(100000);
    }
}

}  // namespace Hanami
}  // namespace Kitsunemimi

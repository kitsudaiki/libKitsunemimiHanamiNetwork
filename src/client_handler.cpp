#include "client_handler.h"
#include <callbacks.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

namespace Kitsunemimi
{
namespace Hanami
{
ClientHandler* ClientHandler::m_instance = nullptr;
Kitsunemimi::Sakura::SessionController* ClientHandler::m_sessionController = nullptr;

ClientHandler::ClientHandler(const std::string &localIdentifier)
    : Kitsunemimi::Thread("HanamiClientHandler")
{
    m_localIdentifier = localIdentifier;
    m_sessionController = new Sakura::SessionController(&sessionCreateCallback,
                                                        &sessionCloseCallback,
                                                        &errorCallback);
}

/**
 * @brief ClientHandler::connectClient
 * @param info
 * @param error
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

    const std::regex ipv4Regex("\\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}\\b");

    Kitsunemimi::Sakura::Session* newSession = nullptr;
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

    if(newSession == nullptr) {
        return false;
    }

    newSession->m_sessionIdentifier = info.remoteIdentifier;
    info.session = newSession;

    return true;
}


/**
 * @brief forward callback to close session
 *
* @param remoteIdentifier name of the client for later identification
 */
bool
ClientHandler::closeClient(const std::string &remoteIdentifier,
                           ErrorContainer &,
                           const bool earseFromList)
{
    LOG_DEBUG("close client with remote-identifier \"" + remoteIdentifier + "\"");

    std::lock_guard<std::mutex> guard(m_lock);

    std::map<std::string, ClientInformation>::iterator it;
    it = m_outgoingClients.find(remoteIdentifier);
    if(it != m_outgoingClients.end())
    {
        if(it->second.session != nullptr)
        {
            LOG_DEBUG("schedule client with remote-identifier \""
                      + remoteIdentifier
                      + "\" for deletion");
            ErrorContainer error;
            it->second.session->closeSession(error);
            LOG_ERROR(error);
            m_forDeletion.push_back(it->second.session);
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
 * @brief ClientHandler::waitForAllConnected
 * @param timeout
 * @return
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

        m_lock.lock();
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
        m_lock.unlock();

        if(allConnected) {
            return true;
        }

        sleepThread(10000);
        actualTimeout += 10000;
    }

    return false;
}

/**
 * @brief ClientHandler::addInternalClient
 * @param identifier
 * @param newClient
 * @return
 */
bool
ClientHandler::addInternalClient(const std::string &identifier,
                                 Sakura::Session* newClient)
{
    std::map<std::string, Sakura::Session*>::const_iterator it;
    it = m_incomingClients.find(identifier);

    if(it != m_incomingClients.end())
    {
        ErrorContainer error;
        newClient->closeSession(error);
        LOG_ERROR(error);
        m_forDeletion.push_back(newClient);
        return false;
    }

    m_incomingClients.insert(std::make_pair(identifier, newClient));

    return true;
}

/**
 * @brief ClientHandler::removeInternalClient
 * @param identifier
 * @return
 */
bool
ClientHandler::removeInternalClient(const std::string &identifier)
{
    std::map<std::string, Sakura::Session*>::iterator it;
    it = m_incomingClients.find(identifier);
    if(it != m_incomingClients.end())
    {
        Sakura::Session* tempSession = it->second;
        m_incomingClients.erase(it);

        if(tempSession != nullptr)
        {
            ErrorContainer error;
            tempSession->closeSession(error);
            LOG_ERROR(error);
            m_forDeletion.push_back(tempSession);
        }

        return true;
    }

    return false;
}

/**
 * @brief ClientHandler::addOutgoingClient
 * @param remoteIdentifier
 * @param address
 * @param port
 * @return
 */
bool
ClientHandler::addOutgoingClient(const std::string &remoteIdentifier,
                                 const std::string &address,
                                 const uint16_t port)
{
    std::lock_guard<std::mutex> guard(m_lock);

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
 * @brief ClientHandler::getSession
 * @param target
 * @return
 */
Sakura::Session*
ClientHandler::getSession(const std::string &target)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::map<std::string, ClientInformation>::const_iterator it;
    it = m_outgoingClients.find(target);
    if(it != m_outgoingClients.end()) {
        return it->second.session;
    }

    return nullptr;
}

/**
 * @brief ClientHandler::run
 */
void
ClientHandler::run()
{
    ErrorContainer error;
    while(m_abort == false)
    {
        m_lock.lock();
        for(Sakura::Session* session : m_forDeletion)
        {
            LOG_DEBUG("delete session '" + session->m_sessionIdentifier + "'");
            delete session;
        }
        m_forDeletion.clear();

        std::map<std::string, ClientInformation>::iterator it;
        for(it = m_outgoingClients.begin();
            it != m_outgoingClients.end();
            it++)
        {
            if(it->second.session == nullptr)
            {
                if(connectClient(it->second, error) == false)
                {
                    error.addMeesage("create connection to '" + it->first + "' failed");
                    error.addSolution("check if component '" + it->first + "' is up and running.");
                    LOG_ERROR(error);
                    error._errorMessages.clear();
                    error._possibleSolution.clear();
                }
            }
        }

        m_lock.unlock();

        sleepThread(100000);
    }
}

}  // namespace Hanami
}  // namespace Kitsunemimi

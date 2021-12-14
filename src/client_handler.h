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
    Sakura::Session* getSession(const std::string &target);
    bool closeClient(const std::string &remoteIdentifier,
                     ErrorContainer &error,
                     const bool earseFromList);

    bool waitForAllConnected(const uint32_t timeout);

    bool addInternalClient(const std::string &identifier,
                           Sakura::Session* newClient);

    bool removeInternalClient(const std::string &identifier);

    void* streamReceiver = nullptr;
    void (*processStreamData)(void*, Sakura::Session*, const void*, const uint64_t);

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

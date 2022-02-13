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

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>
#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraLang/blossom.h>

#include <libKitsunemimiHanamiCommon/config.h>
#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/buffer/stack_buffer.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCrypto/common.h>

using Kitsunemimi::Sakura::SessionController;

namespace Kitsunemimi
{
namespace Hanami
{

HanamiMessaging* HanamiMessaging::m_messagingController = nullptr;

/**
 * @brief constructor
 */
HanamiMessaging::HanamiMessaging() {}

/**
 * @brief destructor
 */
HanamiMessaging::~HanamiMessaging() {}

/**
 * @brief callback, which is triggered by error-logs
 *
 * @param errorMessage error-message to send to sagiri
 */
void
handleErrorCallback(const std::string &errorMessage)
{
    HanamiMessaging::getInstance()->sendGenericErrorMessage(errorMessage);
}

/**
 * @brief fill overview with all configured components
 */
void
HanamiMessaging::fillSupportOverview()
{
    bool success = false;

    SupportedComponents* supportedComponents = SupportedComponents::getInstance();

    if(GET_STRING_CONFIG("kyouko", "address", success) != "") {
        supportedComponents->support[KYOUKO] = true;
    }
    if(GET_STRING_CONFIG("misaka", "address", success) != "") {
        supportedComponents->support[MISAKA] = true;
    }
    if(GET_STRING_CONFIG("azuki", "address", success) != "") {
        supportedComponents->support[AZUKI] = true;
    }
    if(GET_STRING_CONFIG("sagiri", "address", success) != "") {
        supportedComponents->support[SAGIRI] = true;
    }
    if(GET_STRING_CONFIG("nagato", "address", success) != "") {
        supportedComponents->support[NAGATO] = true;
    }
    if(GET_STRING_CONFIG("izuna", "address", success) != "") {
        supportedComponents->support[IZUNA] = true;
    }
}

/**
 * @brief initalize endpoints
 *
 * @param error reference for error-output
 * @param predefinedEndpoints optional string with a predefined endpoint-file for testing
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::initEndpoints(ErrorContainer &error,
                               const std::string &predefinedEndpoints)
{
    bool success = false;
    Endpoint* endpoints = Endpoint::getInstance();

    // read endpoints
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

    // parse endpoints
    if(endpoints->parse(endpointContent, error) == false)
    {
        LOG_ERROR(error);
        return false;
    }

    return true;
}

/**
 * @brief add new server
 *
 * @param serverAddress address of the new server
 * @param error reference for error-output
 * @param port of the tcp-server
 * @param certFilePath path to the certificate-file
 * @param keyFilePath path to the key-file
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::addServer(const std::string &serverAddress,
                           ErrorContainer &error,
                           const uint16_t port,
                           const std::string &certFilePath,
                           const std::string &keyFilePath)
{
    // init server based on the type of the address in the config
    const std::regex ipv4Regex("\\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}\\b");
    if(regex_match(serverAddress, ipv4Regex))
    {
        // create tcp-server
        if(ClientHandler::m_sessionController->addTcpServer(port,

                                                               error) == 0)
        {
            error.addMeesage("can't initialize tcp-server on port "
                             + std::to_string(port));
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

    return true;
}

/**
 * @brief initalize client-connections
 *
 * @param list of groups in config-file
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::initClients(const std::vector<std::string> &configGroups)
{
    bool success = false;

    // init client-connections
    for(const std::string& groupName : configGroups)
    {
        const std::string address = GET_STRING_CONFIG(groupName, "address", success);
        if(address != "")
        {
            const uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG(groupName, "port", success));
            if(ClientHandler::m_instance->addOutgoingClient(groupName, address, port) == false) {
                return false;
            }
        }
    }

    // wait for 1 secont at maximum
    ClientHandler::m_instance->waitForAllConnected(1);

    return true;
}

/**
 * @brief create and initialize new messaging-controller
 *
 * @param localIdentifier identifier for outgoing sessions to identify against the servers
 * @param configGroups config-groups for automatic creation of server and clients
 * @param receiver receiver for handling of intneral steam-messages within the callback-function
 * @param processStream callback for stream-messages
 * @param processGenericRequest callback for data-request-messages
 * @param error callbacks for incoming stream-messages
 * @param createServer true, if the instance should also create a server
 * @param predefinedEndpoints
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::initialize(const std::string &localIdentifier,
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
                            const bool createServer,
                            const std::string &predefinedEndpoints)
{
    // precheck to avoid double-initializing
    if(m_isInit) {
        return false;
    }

    // set callback to send error-messages to sagiri for logging
    setErrorLogCallback(&handleErrorCallback);

    // init client-handler
    ClientHandler::m_instance = new ClientHandler(localIdentifier);    
    ClientHandler::m_instance->streamReceiver = receiver;
    ClientHandler::m_instance->processStreamData = processStream;
    ClientHandler::m_instance->processGenericRequest = processGenericRequest;
    ClientHandler::m_instance->startThread();

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
    fillSupportOverview();
    SupportedComponents* support = SupportedComponents::getInstance();
    support->localComponent = localIdentifier;

    // init server if requested
    if(createServer)
    {
        // get server-address from config
        bool success = false;
        const std::string serverAddress = GET_STRING_CONFIG("DEFAULT", "address", success);
        if(success == false) {
            return false;
        }

        // init endpoints
        if(initEndpoints(error, predefinedEndpoints) == false) {
            return false;
        }

        // get port from config
        const int port = GET_INT_CONFIG("DEFAULT", "port", success);
        const uint16_t serverPort = static_cast<uint16_t>(port);

        // create server
        if(addServer(serverAddress, error, serverPort) == false) {
            return false;
        }
    }

    // init clients
    if(initClients(configGroups) == false) {
        return false;
    }

    m_isInit = true;

    return true;
}

/**
 * @brief send stream-message over a client
 *
 * @param target name of the client to trigger
 * @param data stack-buffer to send
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::sendStreamMessage(const std::string &target,
                                   StackBuffer &data,
                                   ErrorContainer &error)
{
    // get target-client
    Sakura::Session* client = ClientHandler::m_instance->getOutgoingSession(target);
    if(client == nullptr) {
        return false;
    }

    // send all data from the stackbuffer
    for(uint32_t i = 0; i < data.blocks.size(); i++)
    {
        DataBuffer* buf = getFirstElement_StackBuffer(data);

        // only the last buffer should have an expected reply
        const bool expReply = i == data.blocks.size() - 1;
        if(client->sendStreamData(buf->data, buf->usedBufferSize, error, expReply) == false) {
            return false;
        }
        removeFirst_StackBuffer(data);
    }

    return true;
}

/**
 * @brief send stream-message over a client
 *
 * @param target name of the client to trigger
 * @param data pointer to the data to send
 * @param dataSize size of data in bytes to send
 * @param replyExpected true to expect a reply-message
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::sendStreamMessage(const std::string &target,
                                   const void* data,
                                   const uint64_t dataSize,
                                   const bool replyExpected,
                                   ErrorContainer &error)
{
    // get target-client
    Sakura::Session* client = ClientHandler::m_instance->getOutgoingSession(target);
    if(client == nullptr) {
        return false;
    }

    // send stream-data
    if(client->sendStreamData(data, dataSize, error, replyExpected) == false) {
        return false;
    }

    return true;
}

/**
 * @brief send a generic message over the internal messaging
 *
 * @param target name of the client to trigger
 * @param data pointer to data to send
 * @param dataSize size of data to send
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::sendGenericMessage(const std::string &target,
                                    const void* data,
                                    const uint64_t dataSize,
                                    ErrorContainer &error)
{
    LOG_DEBUG("send generic message to target \'" + target + "\'");

    // get client
    Sakura::Session* client = ClientHandler::m_instance->getOutgoingSession(target);
    if(client == nullptr)
    {
        error.addMeesage("target '" + target + "' for send a generic message not found.");
        LOG_ERROR(error);
        return false;
    }

    // create header
    SakuraGenericHeader header;
    header.size = dataSize;

    // create message
    const uint64_t bufferSize = sizeof(SakuraGenericHeader) + dataSize;
    uint8_t* buffer = new uint8_t[bufferSize];
    memcpy(&buffer[0], &header, sizeof(SakuraGenericHeader));
    memcpy(&buffer[sizeof(SakuraGenericHeader)], data, dataSize);

    // send
    return client->sendNormalMessage(buffer, bufferSize, error);
}

/**
 * @brief send a generic message over the internal messaging
 *
 * @param target name of the client to trigger
 * @param data pointer to data to send
 * @param dataSize size of data to send
 * @param error reference for error-output
 *
 * @return pointer to data-buffer with response, if successful, else nullptr
 */
DataBuffer*
HanamiMessaging::sendGenericRequest(const std::string &target,
                                    const void* data,
                                    const uint64_t dataSize,
                                    ErrorContainer &error)
{
    LOG_DEBUG("send generic request to target \'" + target + "\'");

    // get client
    Sakura::Session* client = ClientHandler::m_instance->getOutgoingSession(target);
    if(client == nullptr)
    {
        error.addMeesage("target '" + target + "' for send a generic request not found.");
        LOG_ERROR(error);
        return nullptr;
    }

    // create header
    SakuraGenericHeader header;
    header.size = dataSize;

    // create message
    const uint64_t bufferSize = sizeof(SakuraGenericHeader) + dataSize;
    uint8_t* buffer = new uint8_t[bufferSize];
    memcpy(&buffer[0], &header, sizeof(SakuraGenericHeader));
    memcpy(&buffer[sizeof(SakuraGenericHeader)], data, dataSize);

    // send
    DataBuffer* result = client->sendRequest(buffer, bufferSize, 10, error);

    // clear buffer to avoid memory-leak
    delete[] buffer;

    return result;
}

/**
 * @brief trigger remote action
 *
 * @param target name of the client to trigger
 * @param response reference for the response
 * @param request request-information to identify the target-action on the remote host
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::triggerSakuraFile(const std::string &target,
                                   ResponseMessage& response,
                                   const RequestMessage &request,
                                   ErrorContainer &error)
{
    LOG_DEBUG("trigger sakura-file \'" + request.id + "\' on target \'" + target + "\'");

    // get client
    Sakura::Session* client = ClientHandler::m_instance->getOutgoingSession(target);
    if(client == nullptr)
    {
        response.success = false;
        response.type = NOT_FOUND_RTYPE;
        response.responseContent = "target \'" + target + "\' not found";

        return false;
    }

    // try to send request to target
    if(createRequest(client, response, request, error) == false)
    {
        response.success = false;
        response.type = INTERNAL_SERVER_ERROR_RTYPE;

        return false;
    }

    return true;
}

/**
 * @brief close a client
 *
 * @param remoteIdentifier identifier for the client to close
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
HanamiMessaging::closeClient(const std::string &remoteIdentifier,
                             ErrorContainer &error)
{
    return ClientHandler::m_instance->closeClient(remoteIdentifier, error, true);
}

/**
 * @brief get pointer of a specific outgoing session
 *
 * @param identifier name of the requested session
 *
 * @return nullptr, if session not found, else pointer to session
 */
Sakura::Session*
HanamiMessaging::getOutgoingSession(const std::string identifier)
{
    return ClientHandler::m_instance->getOutgoingSession(identifier);
}

/**
 * @brief get pointer of a specific incoming session
 *
 * @param identifier name of the requested session
 *
 * @return nullptr, if session not found, else pointer to session
 */
Sakura::Session*
HanamiMessaging::getIncomingSession(const std::string identifier)
{
    return ClientHandler::m_instance->getIncomingSession(identifier);
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

/**
 * @brief get the current datetime of the system
 *
 * @return datetime as string
 */
const std::string
getDatetime()
{
    const time_t now = time(nullptr);
    tm *ltm = localtime(&now);

    const std::string datatime =
            std::to_string(1900 + ltm->tm_year)
            + "-"
            + std::to_string(1 + ltm->tm_mon)
            + "-"
            + std::to_string(ltm->tm_mday)
            + " "
            + std::to_string(ltm->tm_hour)
            + ":"
            + std::to_string(ltm->tm_min)
            + ":"
            + std::to_string(ltm->tm_sec);

    return datatime;
}

/**
 * @brief send error-message to sagiri
 *
 * @param errorMessage error-message to send to sagiri
 */
void
HanamiMessaging::sendGenericErrorMessage(const std::string &errorMessage)
{
    // handle local sagiri
    if(SupportedComponents::getInstance()->localComponent == "sagiri")
    {
        bool success = false;
        Kitsunemimi::ErrorContainer error;

        // TODO: handle result
        const std::string resultLocation = GET_STRING_CONFIG("sagiri", "error_location", success);
        const std::string filePath = resultLocation + "/generic";

        // create an empty file, if no exist
        if(std::filesystem::exists(filePath) == false)
        {
            // create new file and write content
            std::ofstream outputFile;
            outputFile.open(filePath);
            outputFile.close();
        }

        // init table
        Kitsunemimi::TableItem tableOutput;
        tableOutput.addColumn("key");
        tableOutput.addColumn("value");

        // fill table
        tableOutput.addRow(std::vector<std::string>{"timestamp", getDatetime()});
        tableOutput.addRow(std::vector<std::string>{"component", "sagiri"});
        tableOutput.addRow(std::vector<std::string>{"error", errorMessage});

        // write to file
        const std::string finalMessage = tableOutput.toString(200, true) + "\n\n\n";
        if(appendText(filePath, finalMessage, error) == false) {
            LOG_ERROR(error);
        }

        return;
    }

    // check if sagiri is supported
    if(SupportedComponents::getInstance()->support[SAGIRI] == false) {
        return;
    }

    // this function is triggered by every error-message in this logger. if an error is in the
    // following code which sending to sagiri, it would result in an infinity-look of this function
    // and a stackoverflow. So this variable should ensure, that error in this function doesn't
    // tigger the function in itself again.
    if(m_whileSendError == true) {
        return;
    }
    m_whileSendError = true;

    // convert message
    std::string base64Error;
    Kitsunemimi::Crypto::encodeBase64(base64Error, errorMessage.c_str(), errorMessage.size());

    // create message
    Kitsunemimi::Hanami::HanamiMessaging* msg = Kitsunemimi::Hanami::HanamiMessaging::getInstance();
    const std::string message = "{\"message_type\":\"error_log\","
                                "\"message\":\"" + base64Error + "\"}";

    // send
    Kitsunemimi::ErrorContainer error;
    msg->sendGenericMessage("sagiri", message.c_str(), message.size(), error);
    m_whileSendError = false;
}

}  // namespace Hanami
}  // namespace Kitsunemimi

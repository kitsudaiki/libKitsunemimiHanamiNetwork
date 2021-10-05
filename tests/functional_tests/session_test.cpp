/**
 * @file       session_test.cpp
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
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

#include "session_test.h"

#include <test_blossom.h>

#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraLang/blossom.h>

#include <libKitsunemimiHanamiMessaging/messaging_controller.h>
#include <libKitsunemimiHanamiMessaging/messaging_client.h>

#include <libKitsunemimiCommon/files/text_file.h>

namespace Kitsunemimi
{
namespace Hanami
{

Kitsunemimi::Hanami::Session_Test* Session_Test::m_instance = nullptr;

/**
 * @brief streamDataCallback
 * @param data
 * @param dataSize
 */
void
streamDataCallback(void*,
                   Kitsunemimi::Sakura::Session*,
                   const void* data,
                   const uint64_t dataSize)
{
    std::string receivedMessage(static_cast<const char*>(data), dataSize);

    std::cout<<"receive message"<<std::endl;
    bool ret = false;

    if(dataSize == Session_Test::m_instance->m_message.size())
    {
        ret = true;
        Session_Test::m_instance->compare(receivedMessage, Session_Test::m_instance->m_message);
    }

    Session_Test::m_instance->compare(ret,  true);
}

/**
 * @brief sessionCreateCallback
 * @param client
 */
void
sessionCreateCallback(Kitsunemimi::Hanami::MessagingClient* client,
                      const std::string identifier)
{
    Session_Test::m_instance->compare(true,  true);
    if(identifier == "self") {
        Session_Test::m_instance->compare(identifier,  std::string("self"));
    } else {
        Session_Test::m_instance->compare(identifier,  std::string("contr1"));
    }

    client->setStreamMessageCallback(Session_Test::m_instance, &streamDataCallback);
    Session_Test::m_instance->m_client = client;
}

/**
 * @brief sessionCloseCallback
 * @param identifier
 */
void
sessionCloseCallback(const std::string identifier)
{
    Session_Test::m_instance->compare(true,  true);
    Session_Test::m_instance->compare(identifier,  std::string("contr1"));
}

/**
 * @brief Session_Test::Session_Test
 */
Session_Test::Session_Test(const std::string &address) :
    Kitsunemimi::CompareTestHelper("Session_Test")
{
    m_address = address;
    Session_Test::m_instance = this;

    initTestCase();
    runTest();
}

/**
 * @brief initTestCase
 */
void
Session_Test::initTestCase()
{
    m_message = "------------------------------------------------------------------------"
                "-------------------------------------#----------------------------------"
                "------------------------------------------------------------------------"
                "---#--------------------------------------------------------------------"
                "-----------------------------------------#------------------------------"
                "------------------------------------------------------------------------"
                "-------#----------------------------------------------------------------"
                "---------------------------------------------#--------------------------"
                "------------------------------------------------------------------------"
                "-----------#------------------------------------------------------------"
                "-------------------------------------------------#----------------------"
                "-----#";

    TestBlossom* newBlossom = new TestBlossom(this);
    Kitsunemimi::Sakura::SakuraLangInterface::getInstance()->addBlossom("test1",
                                                                        "test2",
                                                                        newBlossom);
    std::string errorMessage = "";
    Kitsunemimi::Sakura::SakuraLangInterface::getInstance()->addTree("test-tree",
                                                                     getTestTree(),
                                                                     errorMessage);
    Kitsunemimi::writeFile("/tmp/test-config.conf",
                           getTestConfig(),
                           errorMessage,
                           true);
}

/**
 * @brief runTest
 */
void
Session_Test::runTest()
{
    Config::initConfig("/tmp/test-config.conf");
    std::vector<std::string> groupNames = {"self"};
    m_numberOfTests++;
    TEST_EQUAL(MessagingController::getInstance()->initialize("contr1",
                                                               groupNames,
                                                               sessionCreateCallback,
                                                               sessionCloseCallback), true);
    m_numberOfTests++;
    TEST_EQUAL(MessagingController::getInstance()->initialize("contr1",
                                                              groupNames,
                                                              sessionCreateCallback,
                                                              sessionCloseCallback), false);

    const bool isNullptr = m_client == nullptr;
    m_numberOfTests++;
    TEST_EQUAL(isNullptr, false);

    std::string errorMessage = "";
    DataMap inputValues;
    inputValues.insert("input", new DataValue(42));
    inputValues.insert("test_output", new DataValue(""));

    DataMap result;
    m_numberOfTests++;
    TEST_EQUAL(m_client->triggerSakuraFile(result,
                                         "test-tree",
                                         inputValues.toString(),
                                         errorMessage),
               true);
    m_numberOfTests++;
    TEST_EQUAL(m_client->triggerSakuraFile(result,
                                         "fail",
                                         inputValues.toString(),
                                         errorMessage),
               false);

    m_numberOfTests++;
    TEST_EQUAL(result.get("test_output")->toValue()->getInt(), 42);

    m_numberOfTests++;
    TEST_EQUAL(m_client->sendStreamData(m_message.c_str(), m_message.size()), true);
    sleep(1);

    m_numberOfTests++;
    TEST_EQUAL(m_client->closeSession(), true);
    sleep(1);


    TEST_EQUAL(m_numberOfTests, 19);
    delete m_client;

    std::cout<<"finish"<<std::endl;
}

/**
 * @brief Session_Test::getTestTree
 * @return
 */
const std::string
Session_Test::getTestTree()
{
    const std::string tree = "[\"test\"]\n"
                             "- input = \"{{}}\"\n"
                             "- test_output = \"\"\n"
                             "\n"
                             "test1(\"this is a test\")\n"
                             "->test2:\n"
                             "   - input = input\n"
                             "   - output >> test_output\n";
    return tree;
}

/**
 * @brief Session_Test::getTestConfig
 * @return
 */
const std::string
Session_Test::getTestConfig()
{
    const std::string config = "[DEFAULT]\n"
                               "address = \"" + m_address + "\"\n"
                               "port = 12345\n"
                               "\n"
                               "\n"
                               "[self]\n"
                               "port = 12345\n"
                               "address = \"" + m_address + "\"\n";
    return config;
}

} // namespace Hanami
} // namespace Kitsunemimi

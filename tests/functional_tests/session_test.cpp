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

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCommon/files/text_file.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

namespace Kitsunemimi
{
namespace Hanami
{

Kitsunemimi::Hanami::Session_Test* Session_Test::m_instance = nullptr;

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

    std::string errorMessage = "";
    TestBlossom* newBlossom = new TestBlossom(this);
    SakuraLangInterface::getInstance()->addBlossom("test1", "test2", newBlossom);
    SakuraLangInterface::getInstance()->addTree("test-tree", getTestTree(), errorMessage);
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
    std::vector<std::string> groupNames = {"target"};
    m_numberOfTests++;
    TEST_EQUAL(HanamiMessaging::getInstance()->initialize("client", groupNames), true);
    m_numberOfTests++;
    TEST_EQUAL(HanamiMessaging::getInstance()->initialize("client", groupNames), false);

    std::string errorMessage = "";
    DataMap inputValues;
    inputValues.insert("input", new DataValue(42));
    inputValues.insert("test_output", new DataValue(""));

    DataMap result;
    m_numberOfTests++;
    TEST_EQUAL(HanamiMessaging::getInstance()->triggerSakuraFile("target",
                                                                 result,
                                                                 "test-tree",
                                                                 inputValues.toString(),
                                                                 errorMessage),
               true);
    m_numberOfTests++;
    TEST_EQUAL(HanamiMessaging::getInstance()->triggerSakuraFile("target",
                                                                 result,
                                                                 "fail",
                                                                 inputValues.toString(),
                                                                 errorMessage),
               false);

    m_numberOfTests++;
    TEST_EQUAL(result.get("test_output")->toValue()->getInt(), 42);

    m_numberOfTests++;
    TEST_EQUAL(HanamiMessaging::getInstance()->closeClient("target"), true);
    sleep(1);


    TEST_EQUAL(m_numberOfTests, 7);

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
                               "[target]\n"
                               "port = 12345\n"
                               "address = \"" + m_address + "\"\n";
    return config;
}

} // namespace Hanami
} // namespace Kitsunemimi

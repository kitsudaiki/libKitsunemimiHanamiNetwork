/**
 * @file        test_blossom.cpp
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

#include "test_blossom.h"

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <session_test.h>


namespace Kitsunemimi
{
namespace Hanami
{

TestBlossom::TestBlossom(Session_Test* sessionTest)
    : Kitsunemimi::Sakura::Blossom("this is a test-blossom")
{
    m_sessionTest = sessionTest;
    registerInputField("input", Kitsunemimi::Sakura::SAKURA_INT_TYPE, true, "test-intput");
    registerOutputField("output", Kitsunemimi::Sakura::SAKURA_INT_TYPE, "test-output");
}

bool
TestBlossom::runTask(Sakura::BlossomLeaf &blossomLeaf,
                     const DataMap &,
                     Sakura::BlossomStatus &status,
                     ErrorContainer &)
{
    LOG_DEBUG("TestBlossom");
    const int value = blossomLeaf.input.get("input").getInt();
    m_sessionTest->compare(value, 42);
    blossomLeaf.output.insert("output", 42);

    status.statusCode = OK_RTYPE;
    return true;
}

}  // namespace Hanami
}  // namespace Kitsunemimi

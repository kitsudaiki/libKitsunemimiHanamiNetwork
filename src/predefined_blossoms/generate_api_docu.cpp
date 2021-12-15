/**
 * @file        generate_api_docu.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2021 Tobias Anker
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

#include "generate_api_docu.h"

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <libKitsunemimiHanamiCommon/component_support.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;

namespace Kitsunemimi
{
namespace Hanami
{

GenerateApiDocu::GenerateApiDocu()
    : Kitsunemimi::Sakura::Blossom("Generate a user-specific documentation for the API "
                                   "of the current component.")
{
    registerOutputField("documentation",
                        SAKURA_STRING_TYPE,
                        "API-documentation as base64 converted string.");
}

/**
 * @brief runTask
 */
bool
GenerateApiDocu::runTask(BlossomLeaf &blossomLeaf,
                         const Kitsunemimi::DataMap &,
                         BlossomStatus &,
                         Kitsunemimi::ErrorContainer &)
{
    const std::string localComponent = SupportedComponents::getInstance()->localComponent;

    std::string documentsion = "";
    documentsion.append(localComponent);
    toUpperCase(documentsion);
    documentsion.append("\n");
    documentsion.append(localComponent.size(), '=');
    documentsion.append("\n");

    HanamiMessaging::getInstance()->generateDocu(documentsion);
    std::string base64Docu;
    Crypto::encodeBase64(base64Docu, documentsion.c_str(), documentsion.size());

    blossomLeaf.output.insert("documentation", base64Docu);

    return true;
}

}  // namespace Hanami
}  // namespace Kitsunemimi

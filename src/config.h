/**
 * @file        config.h
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

#ifndef KITSUNEMIMI_SAKURA_MESSAGING_CONFIG_H
#define KITSUNEMIMI_SAKURA_MESSAGING_CONFIG_H

#include <assert.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief register configs
 * s
 * @param configGroups list of groups to register network-connections
 */
void
registerConfigs(const std::vector<std::string> &configGroups)
{
    REGISTER_INT_CONFIG("DEFAULT", "port", 0);
    REGISTER_STRING_CONFIG("DEFAULT", "address", "");

    for(const std::string& groupName : configGroups)
    {
        REGISTER_INT_CONFIG(groupName, "port", 0);
        REGISTER_STRING_CONFIG(groupName, "address", "");
    }
}

/**
 * @brief validate config for a specific group
 *
 * @param groupName group-name to validate
 *
 * @return true, if config is valid, else false
 */
bool
checkConfigs(const std::string &groupName)
{
    bool success = false;

    // check address
    const std::string address = GET_STRING_CONFIG(groupName, "address", success);
    assert(success);
    if(address == "")
    {
        LOG_ERROR("address in group " + groupName + " was not set in config-file");
        return false;
    }
    // TODO: check if address is valid ip

    return true;
}

/**
 * @brief validate config
 *
 * @param configGroups list of config-groups the check
 * @param createServer if true, check also the DEFAULT-section
 *
 * @return true, if config is valid, else false
 */
bool
checkConfigs(const std::vector<std::string> &configGroups,
             const bool createServer)
{
    if(createServer)
    {
        if(checkConfigs("DEFAULT") == false) {
            return false;
        }
    }

    // check client-configs
    for(const std::string& groupName : configGroups)
    {
        if(checkConfigs(groupName) == false) {
            return false;
        }
    }

    return true;
}

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_CONFIG_H

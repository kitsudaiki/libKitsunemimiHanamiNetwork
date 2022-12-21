/**
 * @file        item_methods.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
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

#include "item_methods.h"

#include <libKitsunemimiHanamiNetwork/blossom.h>

#include <libKitsunemimiCommon/items/data_items.h>
#include <libKitsunemimiCommon/items/table_item.h>

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief override data of a data-map with new incoming information
 *
 * @param original data-map with the original key-values, which should be updates with the
 *                 information of the override-map
 * @param override map with the new incoming information
 * @param type type of override
 */
void
overrideItems(DataMap &original,
              const DataMap &override,
              OverrideType type)
{
    if(type == ONLY_EXISTING)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt != original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second->copy(), true);
            }
        }
    }
    if(type == ONLY_NON_EXISTING)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt == original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second->copy(), true);
            }
        }
    }
    else if(type == ALL)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            original.insert(overrideIt->first, overrideIt->second->copy(), true);
        }
    }
}

/**
 * @brief create an error-output
 *
 * @param errorLocation location where the error appeared
 * @param error reference for error-output
 * @param possibleSolution message with a possible solution to solve the problem
 * @param blossomType type of the blossom, where the error appeared
 * @param blossomGroup type of the blossom-group, where the error appeared
 * @param blossomName name of the blossom in the script to specify the location
 * @param blossomFilePath file-path, where the error had appeared
 */
void
createError(const std::string &errorLocation,
            ErrorContainer &error,
            const std::string &possibleSolution,
            const std::string &blossomType,
            const std::string &blossomGroupType,
            const std::string &blossomName,
            const std::string &blossomFilePath)
{
    Kitsunemimi::TableItem errorOutput;
    // initialize error-output
    errorOutput.addColumn("Field");
    errorOutput.addColumn("Value");

    if(errorLocation.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"location", errorLocation});
    }

    if(possibleSolution.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"possible solution", possibleSolution});
    }
    if(blossomType.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-type", blossomType});
    }
    if(blossomGroupType.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-group-type", blossomGroupType});
    }
    if(blossomName.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-name", blossomName});
    }
    if(blossomFilePath.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-file-path", blossomFilePath});
    }

    error.addMeesage("Error in location: \n" + errorOutput.toString(200, true));
}

/**
 * @brief create an error-output
 *
 * @param blossomItem blossom-item with information of the error-location
 * @param blossomPath file-path, which contains the blossom
 * @param errorLocation location where the error appeared
 * @param error reference for error-output
 * @param possibleSolution message with a possible solution to solve the problem
 */
void
createError(const BlossomItem &blossomItem,
            const std::string &blossomPath,
            const std::string &errorLocation,
            ErrorContainer &error,
            const std::string &possibleSolution)
{
    return createError(errorLocation,
                       error,
                       possibleSolution,
                       blossomItem.blossomType,
                       blossomItem.blossomGroupType,
                       blossomItem.blossomName,
                       blossomPath);
}

/**
 * @brief create an error-output
 *
 * @param blossomIO blossom-item with information of the error-location
 * @param errorLocation location where the error appeared
 * @param error reference for error-output
 * @param possibleSolution message with a possible solution to solve the problem
 */
void
createError(const BlossomIO &blossomIO,
            const std::string &errorLocation,
            ErrorContainer &error,
            const std::string &possibleSolution)
{
    return createError(errorLocation,
                       error,
                       possibleSolution,
                       blossomIO.blossomType,
                       blossomIO.blossomGroupType,
                       blossomIO.blossomName,
                       blossomIO.blossomPath);
}

} // namespace Sakura
} // namespace Kitsunemimi

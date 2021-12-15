/**
 * @file        special_blossoms.cpp
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

#include "special_blossoms.h"

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/process_execution.h>
#include <libKitsunemimiCommon/common_items/table_item.h>

using namespace Kitsunemimi::Sakura;

//==================================================================================================
// PrintBlossom
//==================================================================================================
AssertBlossom::AssertBlossom()
    : Blossom("Assert a list of values to validate a state.")
{
    allowUnmatched = true;
}

/**
 * runTask
 */
bool
AssertBlossom::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &,
                       BlossomStatus &,
                       Kitsunemimi::ErrorContainer &error)
{
    std::map<std::string, Kitsunemimi::DataItem*>::iterator it;
    for(it = blossomLeaf.input.getItemContent()->toMap()->map.begin();
        it != blossomLeaf.input.getItemContent()->toMap()->map.end();
        it++)
    {
        const std::string isValue = blossomLeaf.parentValues->get(it->first)->toString();
        const std::string shouldValue = it->second->toString();

        if(isValue != shouldValue)
        {
            error.addMeesage("the variable \""
                             + it->first
                             + "\" has the value \""
                             + isValue
                             + "\", but it should have the value \""
                             + shouldValue
                             + "\"");
            return false;
        }
    }

    return true;
}

//==================================================================================================
// PrintBlossom
//==================================================================================================
ItemUpdateBlossom::ItemUpdateBlossom()
    : Blossom("Change a value of an item.")
{
    allowUnmatched = true;
}

/**
 * runTask
 */
bool
ItemUpdateBlossom::runTask(BlossomLeaf &blossomLeaf,
                           const Kitsunemimi::DataMap &,
                           BlossomStatus &,
                           Kitsunemimi::ErrorContainer &)
{
    std::map<std::string, Kitsunemimi::DataItem*>::iterator it;
    for(it = blossomLeaf.input.getItemContent()->toMap()->map.begin();
        it != blossomLeaf.input.getItemContent()->toMap()->map.end();
        it++)
    {
        std::map<std::string, Kitsunemimi::DataItem*>::iterator originalIt;
        originalIt = blossomLeaf.parentValues->map.find(it->first);

        if(originalIt != blossomLeaf.parentValues->map.end())
        {
            blossomLeaf.parentValues->insert(it->first,
                                             it->second->copy(),
                                             true);
        }
    }

    return true;
}

//==================================================================================================
// PrintBlossom
//==================================================================================================
PrintBlossom::PrintBlossom()
    : Blossom("Print values")
{
    allowUnmatched = true;
}

/**
 * runTask
 */
bool
PrintBlossom::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &,
                      BlossomStatus &,
                      Kitsunemimi::ErrorContainer &)
{
    std::string output = "";
    Kitsunemimi::TableItem tableItem;
    tableItem.addColumn("key", "Item-Name");
    tableItem.addColumn("value", "Value");

    std::map<std::string, Kitsunemimi::DataItem*>::iterator it;
    for(it = blossomLeaf.input.getItemContent()->toMap()->map.begin();
        it != blossomLeaf.input.getItemContent()->toMap()->map.end();
        it++)
    {
        tableItem.addRow(std::vector<std::string>{it->first, it->second->toString(true)});
    }

    blossomLeaf.terminalOutput = tableItem.toString(150);

    return true;
}

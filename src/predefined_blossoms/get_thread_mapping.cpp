/**
 * @file        get_thread_mapping.cpp
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

#include "get_thread_mapping.h"

using namespace Kitsunemimi::Sakura;

namespace Kitsunemimi
{
namespace Hanami
{

GetThreadMapping::GetThreadMapping()
    : Kitsunemimi::Sakura::Blossom("Collect all thread-names with its acutal mapped core-id's")
{
    registerOutputField("thread_map",
                        SAKURA_STRING_TYPE,
                        "Map with all thread-names and its core-id as json-string.");
}

/**
 * @brief runTask
 */
bool
GetThreadMapping::runTask(BlossomLeaf &blossomLeaf,
                          const Kitsunemimi::DataMap &context,
                          BlossomStatus &status,
                          Kitsunemimi::ErrorContainer &error)
{

}

}  // namespace Hanami
}  // namespace Kitsunemimi

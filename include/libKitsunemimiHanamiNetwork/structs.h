/**
 * @file        structs.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_STRUCTS_H
#define KITSUNEMIMI_SAKURA_LANG_STRUCTS_H

#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiHanamiCommon/enums.h>

namespace Kitsunemimi
{
namespace Hanami
{

//--------------------------------------------------------------------------------------------------

struct BlossomIO
{
    std::string blossomType = "";
    std::string blossomName = "";
    std::string blossomPath = "";
    std::string blossomGroupType = "";
    std::vector<std::string> nameHirarchie;

    Json::JsonItem output;
    Json::JsonItem input;

    DataMap* parentValues = nullptr;

    std::string terminalOutput = "";

    BlossomIO()
    {
        std::map<std::string, Json::JsonItem> temp;
        output = Json::JsonItem(temp);
        input = Json::JsonItem(temp);
    }
};

//--------------------------------------------------------------------------------------------------

struct BlossomStatus
{
    uint64_t statusCode = 0;
    std::string errorMessage = "";
};

//--------------------------------------------------------------------------------------------------

enum FieldType
{
    SAKURA_UNDEFINED_TYPE = 0,
    SAKURA_INT_TYPE = 1,
    SAKURA_FLOAT_TYPE = 2,
    SAKURA_BOOL_TYPE = 3,
    SAKURA_STRING_TYPE = 4,
    SAKURA_ARRAY_TYPE = 5,
    SAKURA_MAP_TYPE = 6
};

//--------------------------------------------------------------------------------------------------

struct FieldDef
{
    enum IO_ValueType
    {
        UNDEFINED_VALUE_TYPE = 0,
        INPUT_TYPE = 1,
        OUTPUT_TYPE = 2,
    };

    const IO_ValueType ioType;
    const FieldType fieldType;
    const bool isRequired;
    const std::string comment;
    DataItem* match = nullptr;
    DataItem* defaultVal = nullptr;
    std::string regex = "";
    long lowerBorder = 0;
    long upperBorder = 0;

    FieldDef(const IO_ValueType ioType,
             const FieldType fieldType,
             const bool isRequired,
             const std::string &comment)
        : ioType(ioType),
          fieldType(fieldType),
          isRequired(isRequired),
          comment(comment) { }
};

//--------------------------------------------------------------------------------------------------

struct EndpointEntry
{
    SakuraObjectType type = BLOSSOM_TYPE;
    std::string group = "-";
    std::string name = "";
};

//--------------------------------------------------------------------------------------------------

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_STRUCTS_H

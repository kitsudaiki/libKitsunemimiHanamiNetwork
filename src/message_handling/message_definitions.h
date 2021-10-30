/**
 * @file        message_definitions.h
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

#ifndef KITSUNEMIMI_SAKURA_MESSAGING_MESSAGE_DEFINITIONS_H
#define KITSUNEMIMI_SAKURA_MESSAGING_MESSAGE_DEFINITIONS_H

#include <stdint.h>
#include <libKitsunemimiHanamiCommon/enums.h>

namespace Kitsunemimi
{
namespace Hanami
{

enum MessageTypes
{
    SAKURA_TRIGGER_MESSAGE = 0,
    SAKURA_FILE_MESSAGE = 1,
    NORMAL_FILE_MESSAGE = 2,
    TEMPLATE_FILE_MESSAGE = 3,
    RESPONSE_MESSAGE = 4,
};

struct SakuraTriggerMessage
{
    uint8_t type = SAKURA_TRIGGER_MESSAGE;
    HttpType httpType = GET_TYPE;
    uint32_t idSize = 0;
    uint32_t inputValuesSize = 0;
};

struct SakuraFileMessage
{
    uint8_t type = SAKURA_FILE_MESSAGE;
    uint32_t idSize = 0;
    uint32_t sakuraStringSize = 0;
};

struct NormalFileMessage
{
    uint8_t type = NORMAL_FILE_MESSAGE;
    uint32_t idSize = 0;
    uint64_t fileSize = 0;
};

struct TemplateFileMessage
{
    uint8_t type = TEMPLATE_FILE_MESSAGE;
    uint32_t idSize = 0;
    uint32_t templateStringSize = 0;
};

struct ResponseMessage
{
    uint8_t type = RESPONSE_MESSAGE;
    uint8_t success = 0;
    uint32_t messageSize = 0;
};

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_MESSAGE_DEFINITIONS_H

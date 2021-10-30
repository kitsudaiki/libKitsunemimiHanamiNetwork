/**
 * @file        messaging_client.h
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

#ifndef KITSUNEMIMI_SAKURA_MESSAGING_DATA_CLIENT_H
#define KITSUNEMIMI_SAKURA_MESSAGING_DATA_CLIENT_H

#include <iostream>
#include <libKitsunemimiHanamiCommon/enums.h>

namespace Kitsunemimi
{
class DataMap;
struct DataBuffer;
namespace Sakura {
class Session;
}
namespace Hanami
{
class HanamiMessaging;

class MessagingClient
{

public:
    MessagingClient();
    ~MessagingClient();

    bool closeSession();

    bool triggerSakuraFile(DataMap &result,
                           HttpType httpType,
                           const std::string &id,
                           const std::string &inputValues,
                           std::string &errorMessage);

    Kitsunemimi::Sakura::Session* m_session = nullptr;

private:
    bool m_sessionActive = false;

    bool processResponse(DataMap &result,
                         DataBuffer* response,
                         std::string &errorMessage);
};

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_DATA_CLIENT_H

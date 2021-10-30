/**
 * @file        callbacks.h
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

#ifndef KITSUNEMIMI_SAKURA_MESSAGING_CALLBACKS_H
#define KITSUNEMIMI_SAKURA_MESSAGING_CALLBACKS_H

#include <iostream>

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <messaging_client.h>
#include <message_handling/messaging_event.h>
#include <internal_client_handler.h>

#include <message_handling/message_definitions.h>
#include <message_handling/messaging_event_queue.h>

#include <libKitsunemimiSakuraNetwork/session.h>
#include <libKitsunemimiSakuraNetwork/session_controller.h>

#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief process incoming standalone-message
 *
 * @param session pointer to the session, where the message belongs to
 * @param blockerId blocker-id for the response
 * @param data data-buffer with plain message
 */
void
standaloneDataCallback(void*,
                       Kitsunemimi::Sakura::Session* session,
                       const uint64_t blockerId,
                       Kitsunemimi::DataBuffer* data)
{
    LOG_DEBUG("receive sakura-message");
    if(data->usedBufferSize == 0)
    {
        LOG_WARNING("received empty message");
        delete data;
        return;
    }

    const uint8_t type = static_cast<const uint8_t*>(data->data)[0];

    //==============================================================================================
    if(type == SAKURA_TRIGGER_MESSAGE)
    {
        // prepare message
        const SakuraTriggerMessage* header = static_cast<const SakuraTriggerMessage*>(data->data);
        const char* message = static_cast<const char*>(data->data);

        // get id
        uint32_t pos = sizeof (SakuraTriggerMessage);
        const std::string id(&message[pos], header->idSize);

        // get input-values
        pos += header->idSize;
        const std::string inputValues(&message[pos], header->inputValuesSize);

        LOG_DEBUG("receive sakura-trigger-message for id: " + id
                  + "\n  and input-values: " + inputValues);

        // create new event and place it within the event-queue
        MessagingEvent* event = new MessagingEvent(id,
                                                   inputValues,
                                                   session,
                                                   blockerId);

        MessagingEventQueue::getInstance()->addEventToQueue(event);
    }
    //==============================================================================================

    // TODO: error when unknown

    delete data;
}

/**
 * @brief error-callback
 */
void
errorCallback(Kitsunemimi::Sakura::Session* session,
              const uint8_t,
              const std::string message)
{
    Kitsunemimi::ErrorContainer error;
    error.errorMessage = message;
    LOG_ERROR(error);

    // end session
    const bool ret = session->closeSession();

    // check if close session was successful
    if(ret == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.errorMessage = "failed to close session after connection-error";
        LOG_ERROR(error);
    }
    else
    {
        delete session;
    }
}

/**
 * @brief callback for new sessions
 *
 * @param session pointer to session
 * @param identifier identifier of the incoming session
 */
void
sessionCreateCallback(Kitsunemimi::Sakura::Session* session,
                      const std::string identifier)
{
    // set callback for incoming standalone-messages for trigger sakura-files
    session->setStandaloneMessageCallback(nullptr, &standaloneDataCallback);

    // callback was triggered on server-side, place new session into central list
    if(session->isClientSide() == false)
    {
        MessagingClient* newClient = new MessagingClient();
        newClient->m_session = session;
        InternalClientHandler::getInstance()->addClient(identifier, newClient);
    }
}

/**
 * @brief callback for closing sessions
 *
 * @param identifier identifier of the incoming session
 */
void
sessionCloseCallback(Kitsunemimi::Sakura::Session* session,
                      const std::string identifier)
{
    if(session->isClientSide() == false) {
        InternalClientHandler::getInstance()->removeClient(identifier);
    }
}

}
}

#endif // KITSUNEMIMI_SAKURA_MESSAGING_CALLBACKS_H

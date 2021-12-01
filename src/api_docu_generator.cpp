/**
 * @file        api_docu_generator.cpp
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

#include "api_docu_generator.h"

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraLang/blossom.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

namespace Kitsunemimi
{
namespace Hanami
{

/**
 * @brief ApiDocuGenerator::addFieldDocu
 * @param docu
 * @param isInput
 * @param blossom
 */
void
addFieldDocu(std::string &docu,
                               const bool isInput,
                               Sakura::Blossom* blossom)
{
    std::map<std::string, Sakura::Blossom::BlossomValidDef> validMap = blossom->validationMap;

    const std::string bossomComment = blossom->comment;
    std::map<std::string, Sakura::Blossom::BlossomValidDef>::iterator it;
    for(it = validMap.begin();
        it != validMap.end();
        it++)
    {
        const Sakura::Blossom::IO_ValueType ioType = it->second.ioType;
        if(isInput
                && ioType == Sakura::Blossom::OUTPUT_TYPE)
        {
            continue;
        }

        if(isInput == false
                && ioType == Sakura::Blossom::INPUT_TYPE)
        {
            continue;
        }


        const std::string field = it->first;
        const Sakura::FieldType fieldType = it->second.fieldType;
        const std::string comment = it->second.comment;
        const bool isRequired = it->second.isRequired;

        docu.append("\n");
        docu.append("``" + field + "``\n");

        // type
        docu.append("    **Type:**\n");
        if(fieldType == Sakura::SAKURA_MAP_TYPE) {
            docu.append("        ``Map``\n");
        } else if(fieldType == Sakura::SAKURA_ARRAY_TYPE) {
            docu.append("        ``Array``\n");
        } else if(fieldType == Sakura::SAKURA_BOOL_TYPE) {
            docu.append("        ``Bool``\n");
        } else if(fieldType == Sakura::SAKURA_INT_TYPE) {
            docu.append("        ``Int``\n");
        } else if(fieldType == Sakura::SAKURA_FLOAT_TYPE) {
            docu.append("        ``Float``\n");
        } else if(fieldType == Sakura::SAKURA_STRING_TYPE) {
            docu.append("        ``String``\n");
        }

        // required
        docu.append("    **Required:**\n");
        if(isRequired) {
            docu.append("        ``True``\n");
        } else {
            docu.append("        ``False``\n");
        }

        // comment
        if(comment != "")
        {
            docu.append("    **Description:**\n");
            docu.append("        ``" + comment + "``\n");
        }
    }
}

/**
 * @brief ApiDocuGenerator::createBlossomDocu
 * @param docu
 * @param blossom
 */
void
createBlossomDocu(std::string &docu,
                                    Sakura::Blossom* blossom)
{
    // add comment/describtion
    docu.append(blossom->comment + "\n");

    // add input-fields
    docu.append("\n");
    docu.append("Request-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, true, blossom);

    // add output-fields
    docu.append("\n");
    docu.append("Response-Parameter\n");
    docu.append("~~~~~~~~~~~~~~~~~~\n");
    addFieldDocu(docu, false, blossom);
}

/**
 * @brief HanamiMessaging::createEndpointDocu
 * @param docu
 */
void
generateEndpointDocu(std::string &docu)
{
    Endpoint* endpoints = Endpoint::getInstance();
    Sakura::SakuraLangInterface* langInterface = Sakura::SakuraLangInterface::getInstance();

    std::map<std::string, std::map<HttpRequestType, EndpointEntry>>::iterator it;
    for(it = endpoints->endpointRules.begin();
        it != endpoints->endpointRules.end();
        it++)
    {
        const std::string endpoint = it->first;

        // add endpoint
        docu.append(endpoint);
        docu.append("\n");
        docu.append(std::string(endpoint.size(), '-'));
        docu.append("\n");

        std::map<HttpRequestType, EndpointEntry>::const_iterator ruleIt;
        for(ruleIt = it->second.begin();
            ruleIt != it->second.end();
            ruleIt++)
        {
            docu.append("\n");

            // add http-type
            if(ruleIt->first == GET_TYPE) {
                docu.append("GET\n^^^\n\n");
            } else if(ruleIt->first == POST_TYPE) {
                docu.append("POST\n^^^^\n\n");
            } else if(ruleIt->first == DELETE_TYPE) {
                docu.append("DELETE\n^^^^^^\n\n");
            } else if(ruleIt->first == PUT_TYPE) {
                docu.append("PUT\n^^^\n\n");
            }

            if(ruleIt->second.type == BLOSSOM_TYPE)
            {
                createBlossomDocu(docu, langInterface->getBlossom(ruleIt->second.group,
                                                                  ruleIt->second.name));
            }
            else
            {
                //TODO tree-type
            }
        }
    }
}

}  // namespace Hanami
}  // namespace Kitsunemimi

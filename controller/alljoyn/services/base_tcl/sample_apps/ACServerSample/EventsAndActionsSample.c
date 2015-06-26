/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h.
 * The corresponding flag dbgAJSVCAPP is defined in the containing sample app.
 */
#define AJ_MODULE AJSVCAPP
#include <aj_debug.h>

#include <alljoyn.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include "ControlPanelProvided.h"
#include <aj_link_timeout.h>
#include <alljoyn/services_common/PropertyStore.h>
#include "PropertyStoreOEMProvisioning.h"

#ifndef NDEBUG
extern AJ_EXPORT uint8_t dbgAJSVCAPP;
#endif

/**
 * Published Events and Actions Interfaces
 */
static const char EventsInterfaceName[] = "org.alljoyn.ACServerSample.Events";

static const char* EventsInterface[] =
{
    EventsInterfaceName,         /* The first entry is the interface name. */
    "@Version>q",                               /* The Version Property */
    "!&Temperature80FReached",
    "!&Temperature60FReached",
    "!&ModeSetToOff",
    "!&ModeSetToOn",
    NULL
};

static const char ActionsInterfaceName[] = "org.alljoyn.ACServerSample.Actions";

static const char* ActionsInterface[] =
{
    ActionsInterfaceName,         /* The first entry is the interface name. */
    "@Version>q",                               /* The Version Property */
    "?SetModeToAuto",
    "?SetModeToCool",
    "?SetModeToHeat",
    "?SetModeToFan",
    "?SetModeToOff",
    NULL
};

static const AJ_InterfaceDescription EventsAndActionsInterfaces[] =
{
    AJ_PropertiesIface,
    EventsInterface,
    ActionsInterface,
    NULL
};

/*
 * Message identifiers for the method calls this service implements
 */

#define EVENTSANDACTIONS_OBJECT_INDEX                           0

#define EVENTSANDACTIONS_GET_PROP                               AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 0, AJ_PROP_GET)
#define EVENTSANDACTIONS_SET_PROP                               AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 0, AJ_PROP_SET)

#define EVENTS_VERSION_PROP                                     AJ_ENCODE_PROPERTY_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 1, 0)
#define EVENTS_TEMPERATURE80FREACHED_SLS                        AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 1, 1)
#define EVENTS_TEMPERATURE60FREACHED_SLS                        AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 1, 2)
#define EVENTS_TURNEDOFF_SLS                                    AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 1, 3)
#define EVENTS_TURNEDON_SLS                                     AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 1, 4)

#define ACTIONS_VERSION_PROP                                    AJ_ENCODE_PROPERTY_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 0)
#define ACTIONS_SETMODETOAUTO                                   AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 1)
#define ACTIONS_SETMODETOCOOL                                   AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 2)
#define ACTIONS_SETMODETOHEAT                                   AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 3)
#define ACTIONS_SETMODETOFAN                                    AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 4)
#define ACTIONS_SETMODETOOFF                                    AJ_ENCODE_MESSAGE_ID(AJAPP_OBJECTS_LIST_INDEX, EVENTSANDACTIONS_OBJECT_INDEX, 2, 5)

#define EVENTSANDACTIONS_OBJECT_ID                              EVENTSANDACTIONS_OBJECT_INDEX

#define EVENTSANDACTIONS_OBJECT_DESC                            AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 0, 0, 0)

#define EVENTS_INTERFACE_DESC                                   AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 2, 0, 0)
#define EVENTS_TEMPERATURE80FREACHED_SLS_DESC                   AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 2, 2, 0)
#define EVENTS_TEMPERATURE60FREACHED_SLS_DESC                   AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 2, 3, 0)
#define EVENTS_TURNEDOFF_SLS_DESC                               AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 2, 4, 0)
#define EVENTS_TURNEDON_SLS_DESC                                AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 2, 5, 0)

#define ACTIONS_INTERFACE_DESC                                  AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 0, 0)
#define ACTIONS_SETMODETOAUTO_DESC                              AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 2, 0)
#define ACTIONS_SETMODETOCOOL_DESC                              AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 3, 0)
#define ACTIONS_SETMODETOHEAT_DESC                              AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 4, 0)
#define ACTIONS_SETMODETOFAN_DESC                               AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 5, 0)
#define ACTIONS_SETMODETOOFF_DESC                               AJ_DESCRIPTION_ID(EVENTSANDACTIONS_OBJECT_ID, 3, 6, 0)

#define DESCRIPTION_LENGTH 64
char description[DESCRIPTION_LENGTH] = "";

static const char* DescriptionLookup(uint32_t descId, const char* lang)
{
    const char* actualLanguage;
    int8_t langIndex = AJSVC_PropertyStore_GetLanguageIndex(lang);
    char deviceName[DEVICE_NAME_VALUE_LENGTH] = { '\0' };
    uint8_t i;
    const char* deviceNamePerLanguage;

    AJ_InfoPrintf(("Looking up description for o:%u i:%u m:%u a:%u", (descId >> 24) & 0xFF, (descId >> 16) & 0xFF, (descId >> 8) & 0xFF, (descId >> 0) & 0xFF));
    if (langIndex != AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX) {
        actualLanguage = AJSVC_PropertyStore_GetLanguageName(langIndex);
        AJ_InfoPrintf((" language=%s\n", actualLanguage));
        deviceNamePerLanguage = AJSVC_PropertyStore_GetValueForLang(AJSVC_PROPERTY_STORE_DEVICE_NAME, langIndex);
        if (deviceNamePerLanguage == NULL) {
            deviceNamePerLanguage = AJSVC_PropertyStore_GetValueForLang(AJSVC_PROPERTY_STORE_DEVICE_NAME, AJSVC_PropertyStore_GetCurrentDefaultLanguageIndex());
            if (deviceNamePerLanguage == NULL) {
                AJ_ErrPrintf(("DeviceName for language=%s does not exist!\n", actualLanguage));
            }
        }
        if (deviceNamePerLanguage != NULL) {
            strncpy(deviceName, deviceNamePerLanguage, DEVICE_NAME_VALUE_LENGTH);
        }
        for (i = 0; i < DEVICE_NAME_VALUE_LENGTH; i++) { // Replace any illegal/escaped XML characters with '_'
            if (deviceName[i] == '>' || deviceName[i] == '<' || deviceName[i] == '"' || deviceName[i] == '\'' || deviceName[i] == '&') {
                deviceName[i] = '_';
            }
        }
        switch (descId) {
        case (EVENTSANDACTIONS_OBJECT_DESC):
#ifdef _WIN32
            _snprintf(description, DESCRIPTION_LENGTH, "%s Events and Actions [%s]", deviceName, actualLanguage);
#else
            snprintf(description, DESCRIPTION_LENGTH, "%s Events and Actions [%s]", deviceName, actualLanguage);
#endif
            return description;

        case (EVENTS_INTERFACE_DESC):
#ifdef _WIN32
            _snprintf(description, DESCRIPTION_LENGTH, "%s Events [%s]", deviceName, actualLanguage);
#else
            snprintf(description, DESCRIPTION_LENGTH, "%s Events [%s]", deviceName, actualLanguage);
#endif
            return description;

        case (EVENTS_TEMPERATURE80FREACHED_SLS_DESC):
            return "Triggerred when extreme temperature of 80F is reached while heating";

        case (EVENTS_TEMPERATURE60FREACHED_SLS_DESC):
            return "Triggerred when extreme temperature of 60F is reached while cooling";

        case (EVENTS_TURNEDOFF_SLS_DESC):
            return "Triggerred when the AC is turned OFF";

        case (EVENTS_TURNEDON_SLS_DESC):
            return "Triggerred when the AC is turned ON";

        case (ACTIONS_INTERFACE_DESC):
#ifdef _WIN32
            _snprintf(description, DESCRIPTION_LENGTH, "%s Actions [%s]", deviceName, actualLanguage);
#else
            snprintf(description, DESCRIPTION_LENGTH, "%s Actions [%s]", deviceName, actualLanguage);
#endif
            return description;

        case (ACTIONS_SETMODETOAUTO_DESC):
            return "Set AC mode to Auto";

        case (ACTIONS_SETMODETOCOOL_DESC):
            return "Set AC mode to Cool";

        case (ACTIONS_SETMODETOHEAT_DESC):
            return "Set AC mode to Heat";

        case (ACTIONS_SETMODETOFAN_DESC):
            return "Set AC mode to Fan";

        case (ACTIONS_SETMODETOOFF_DESC):
            return "Set AC mode to Off";

        default:
            return NULL;
        }
    }
    AJ_WarnPrintf(("\nError: Unsupported language=%s\n", lang == NULL ? "NULL" : lang));
    return NULL;
}

#define EVENTSANDACTIONS_OBJECT {  "/MyDeviceEventsAndActions", EventsAndActionsInterfaces, AJ_OBJ_FLAG_HIDDEN | AJ_OBJ_FLAG_DISABLED | AJ_OBJ_FLAG_ANNOUNCED, NULL  }, \

AJ_Object eventsAndActionsObjectList[] = {
    EVENTSANDACTIONS_OBJECT
    { NULL }
};

static AJ_Status RegisterObjectList()
{
    eventsAndActionsObjectList[EVENTSANDACTIONS_OBJECT_INDEX].flags &= ~(AJ_OBJ_FLAG_HIDDEN | AJ_OBJ_FLAG_DISABLED);
    eventsAndActionsObjectList[EVENTSANDACTIONS_OBJECT_INDEX].flags |= AJ_OBJ_FLAG_DESCRIBED;
    return AJ_RegisterObjectListWithDescriptions(eventsAndActionsObjectList, AJAPP_OBJECTS_LIST_INDEX, DescriptionLookup);
}

AJ_Status EventsAndActions_Init(const char* const* descriptionLanguages)
{
    AJ_Status status;

    AJ_RegisterDescriptionLanguages(descriptionLanguages);
    status = RegisterObjectList();
    if (status == AJ_OK) {
        AJ_PrintXMLWithDescriptions(eventsAndActionsObjectList, "");
    }

    return status;
}

#define EVENTANDACTIONS_VERSION 1
static AJ_Status EventsAndActionsPropGetHandler(AJ_Message* replyMsg, uint32_t propId, void* context)
{
    if (propId == EVENTS_VERSION_PROP) {
        return AJ_MarshalArgs(replyMsg, "q", EVENTANDACTIONS_VERSION);
    } else if (propId == ACTIONS_VERSION_PROP) {
        return AJ_MarshalArgs(replyMsg, "q", EVENTANDACTIONS_VERSION);
    } else {
        return AJ_ERR_UNEXPECTED;
    }
}

static AJ_Status EventsAndActionsPropSetHandler(AJ_Message* replyMsg, uint32_t propId, void* context)
{
    return AJ_ERR_UNEXPECTED;
}

AJSVC_ServiceStatus EventsAndActionsMessageProcessor(AJ_BusAttachment* busAttachment, AJ_Message* msg, AJ_Status* msgStatus)
{
    AJ_Message reply;
    AJSVC_ServiceStatus serviceStatus = AJSVC_SERVICE_STATUS_HANDLED;

    switch (msg->msgId) {
    case EVENTSANDACTIONS_GET_PROP:
        *msgStatus = AJ_BusPropGet(msg, EventsAndActionsPropGetHandler, NULL);
        break;

    case EVENTSANDACTIONS_SET_PROP:
        *msgStatus = AJ_BusPropSet(msg, EventsAndActionsPropSetHandler, NULL);
        break;

    case ACTIONS_SETMODETOAUTO:
    case ACTIONS_SETMODETOCOOL:
    case ACTIONS_SETMODETOHEAT:
    case ACTIONS_SETMODETOFAN:
    case ACTIONS_SETMODETOOFF:
        AJ_MarshalReplyMsg(msg, &reply);
        setCurrentMode((msg->msgId & 0xFF) - (ACTIONS_SETMODETOAUTO & 0xFF));
        *msgStatus = AJ_DeliverMsg(&reply);
        break;

    default:
        serviceStatus = AJSVC_SERVICE_STATUS_NOT_HANDLED;
        break;
    }

    return serviceStatus;
}

static AJ_Status SendEvent(AJ_BusAttachment* busAttachment, uint32_t eventId)
{
    AJ_Status status = AJ_OK;
    AJ_Message msg;

    status = AJ_MarshalSignal(busAttachment, &msg, eventId, NULL, 0, ALLJOYN_FLAG_SESSIONLESS, 0);
    if (status != AJ_OK) {
        goto ErrorExit;
    }
    status = AJ_DeliverMsg(&msg);
    if (status != AJ_OK) {
        goto ErrorExit;
    }
    status = AJ_CloseMsg(&msg);
    if (status != AJ_OK) {
        goto ErrorExit;
    }
    AJ_AlwaysPrintf(("Event sent successfully\n"));
    return status;

ErrorExit:

    AJ_AlwaysPrintf(("Event sending failed with status=%s\n", AJ_StatusText(status)));
    return status;
}

void EventsAndActions_DoWork(AJ_BusAttachment* busAttachment)
{
    uint8_t sendEvents = checkForEventsToSend();
    if (sendEvents > 0) {

        // 0x01 == need to send event 80F reached whilst heating
        // 0x02 == need to send event 60F reached whilst cooling
        // 0x04 == need to send event mode turned off
        // 0x08 == need to send event mode turned off

        if ((sendEvents & (1 << 0)) != 0) {
            SendEvent(busAttachment, EVENTS_TEMPERATURE80FREACHED_SLS);
        }
        if ((sendEvents & (1 << 1)) != 0) {
            SendEvent(busAttachment, EVENTS_TEMPERATURE60FREACHED_SLS);
        }
        if ((sendEvents & (1 << 2)) != 0) {
            SendEvent(busAttachment, EVENTS_TURNEDOFF_SLS);
        }
        if ((sendEvents & (1 << 3)) != 0) {
            SendEvent(busAttachment, EVENTS_TURNEDON_SLS);
        }
    }
    return;
}

AJ_Status EventsAndActions_Finish(AJ_BusAttachment* busAttachment)
{
    return AJ_OK;
}

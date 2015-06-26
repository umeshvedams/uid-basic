/******************************************************************************
 * Copyright (c) 2013 - 2014, AllSeen Alliance. All rights reserved.
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
#include <aj_crypto.h>

#include <NotificationProducerSample.h>
#include <alljoyn/notification/NotificationCommon.h>
#include <alljoyn/notification/NotificationProducer.h>

#ifdef __linux
#include <NotificationProducerSampleUtil.h>
#else
#define Producer_GetNotificationFromUser(...) do { } while (0)
#define Producer_SetupEnv(...) do { } while (0)
#define Producer_GetShouldDeleteNotificationFromUser(...) do { } while (0)
#define Producer_FreeNotification(...) do { } while (0)
#endif

#ifndef NDEBUG
extern AJ_EXPORT uint8_t dbgAJSVCAPP;
#endif

/**
 * Static consts - sample application specific
 */
const static char* lang1  = "en";
const static char* lang2 = "de-AT";
const static char* hello1 = "Hello AJL World";
const static char* hello2 = "Hallo AJL Welt";
const static char* onKey = "On";
const static char* offKey = "Off";
const static char* HelloVal = "Hello";
const static char* GoodbyeVal = "Goodbye";
const static char* Audio1URL = "http://www.getAudio1.org";
const static char* Audio2URL = "http://www.getAudio2.org";
const static char* Icon1URL = "http://www.getIcon1.org";
const static char* controlPanelServiceObjectPath = "/ControlPanel/MyDevice/areYouSure";
const static char* richIconObjectPath = "/icon/MyDevice";
const static char* richAudioObjectPath = "/audio/MyDevice";

#define NUM_TEXTS   2
static AJNS_DictionaryEntry textToSend[NUM_TEXTS];

#define NUM_CUSTOMS 2
static AJNS_DictionaryEntry customAttributesToSend[NUM_CUSTOMS];

#define NUM_RICH_AUDIO 2
static AJNS_DictionaryEntry richAudioUrls[NUM_RICH_AUDIO];

static uint8_t inputMode = 0;
static AJ_Time isMessageTime;

#ifndef MESSAGES_INTERVAL
#define MESSAGES_INTERVAL 60000
#endif
static uint32_t nextMessageTime = MESSAGES_INTERVAL;

typedef enum _PriorityType {
    PRIORITY_TYPE_FIXED = 0,
    PRIORITY_TYPE_RANDOM = 1,
} PriorityType;
#ifndef MESSAGES_PRIORITY_TYPE
#define MESSAGES_PRIORITY_TYPE PRIORITY_TYPE_FIXED
#endif
static PriorityType priorityType = MESSAGES_PRIORITY_TYPE;
static const char* const PRIORITY_TYPES[] = { "Fixed", "Random" };

typedef enum _IntervalType {
    INTERVAL_TYPE_FIXED = 0,
    INTERVAL_TYPE_RANDOM = 1,
} IntervalType;

#ifndef MESSAGES_INTERVAL_TYPE
#define MESSAGES_INTERVAL_TYPE INTERVAL_TYPE_RANDOM
#endif
static IntervalType intervalType = MESSAGES_INTERVAL_TYPE;
static const char* const INTERVAL_TYPES[] = { "Fixed", "Random" };

#ifndef FIXED_MESSAGE_TYPE
#define FIXED_MESSAGE_TYPE AJNS_NOTIFICATION_MESSAGE_TYPE_INFO
#endif
static const char* const PRIORITIES[AJNS_NUM_MESSAGE_TYPES] = { "Emergency", "Warning", "Info" };

#ifndef FIXED_TTL
#define FIXED_TTL AJNS_NOTIFICATION_TTL_MIN // Note needs to be in the range AJNS_NOTIFICATION_TTL_MIN..AJNS_NOTIFICATION_TTL_MAX
#endif

/**
 * Initialize the Notifications that will be sent during this sample app
 */
static AJNS_NotificationContent notificationContent;
static void InitNotification()
{
    notificationContent.numCustomAttributes = NUM_CUSTOMS;
    customAttributesToSend[0].key   = onKey;
    customAttributesToSend[0].value = HelloVal;
    customAttributesToSend[1].key   = offKey;
    customAttributesToSend[1].value = GoodbyeVal;
    notificationContent.customAttributes = customAttributesToSend;

    notificationContent.numTexts = NUM_TEXTS;
    textToSend[0].key   = lang1;
    textToSend[0].value = hello1;
    textToSend[1].key   = lang2;
    textToSend[1].value = hello2;
    notificationContent.texts = textToSend;

    notificationContent.numAudioUrls = NUM_RICH_AUDIO;
    richAudioUrls[0].key   = lang1;
    richAudioUrls[0].value = Audio1URL;
    richAudioUrls[1].key   = lang2;
    richAudioUrls[1].value = Audio2URL;
    notificationContent.richAudioUrls = richAudioUrls;

    notificationContent.richIconUrl = Icon1URL;
    notificationContent.richIconObjectPath = richIconObjectPath;
    notificationContent.richAudioObjectPath = richAudioObjectPath;
    notificationContent.controlPanelServiceObjectPath = controlPanelServiceObjectPath;
}

/**
 * Initialize service
 */
AJ_Status NotificationProducer_Init()
{
    AJ_Status status = AJ_OK;
    uint32_t random;

    Producer_SetupEnv(&inputMode);
    InitNotification();
    status = AJNS_Producer_Start();

    AJ_AlwaysPrintf(("\n---------------------\nNotification Producer started!\n"));
    AJ_AlwaysPrintf(("Interval:     %u ms\n", MESSAGES_INTERVAL));
    AJ_AlwaysPrintf(("IntervalType: %s (%u)\n", INTERVAL_TYPES[intervalType], intervalType));
    AJ_AlwaysPrintf(("Priority      %s (%u)\n", PRIORITIES[FIXED_MESSAGE_TYPE], FIXED_MESSAGE_TYPE));
    AJ_AlwaysPrintf(("PriorityType: %s (%u)\n", PRIORITY_TYPES[priorityType], priorityType));
    AJ_AlwaysPrintf(("TTL:          %u s\n", FIXED_TTL));
    AJ_AlwaysPrintf(("---------------------\n\n"));
    AJ_InitTimer(&isMessageTime);
    if (!inputMode) {
        if (intervalType == INTERVAL_TYPE_RANDOM) { // Randomize next message time if interval type is RANDOM
            AJ_RandBytes((uint8_t*)&random, sizeof(random));
            nextMessageTime = random % MESSAGES_INTERVAL;
        }
        AJ_AlwaysPrintf(("Next message will be sent in %u ms\n", nextMessageTime));
    } else {
        isMessageTime.seconds -= nextMessageTime / 1000; // Expire next message timer
        isMessageTime.milliseconds -= nextMessageTime % 1000; // Expire next message timer
    }

    return status;
}

/**
 * Allow the user the possibility to delete sent Notifications when DoWork is called.
 * Give the user an option to delete a notification after one was sent.
 */
static void PossiblyDeleteNotification(AJ_BusAttachment* busAttachment)
{
    AJ_Status status;
    uint8_t delMsg = FALSE;
    uint16_t delMsgType = AJNS_NOTIFICATION_MESSAGE_TYPE_INFO;

    if (inputMode) {
        Producer_GetShouldDeleteNotificationFromUser(busAttachment, &delMsg, &delMsgType);
        if (delMsg) {
            status = AJNS_Producer_DeleteLastNotification(busAttachment, delMsgType);
            AJ_AlwaysPrintf(("Delete Last Message Type: %d returned: '%s'\n", delMsgType, AJ_StatusText(status)));
        }
    }
}

/**
 * Meant to simulate scenario where sometimes Notifications are sent when
 * DoWork is called and sometimes not and also toggle between a regular
 * notification and a notication with action.
 * Send the notification every MESSAGES_INTERVAL milliseconds.
 */
static void PossiblySendNotification(AJ_BusAttachment* busAttachment)
{
    static uint32_t offset = 0;
    AJ_Status status;
    uint16_t messageType = FIXED_MESSAGE_TYPE;
    uint32_t ttl = FIXED_TTL;
    uint32_t serialNum;
    uint32_t random;
    uint32_t elapsed = AJ_GetElapsedTime(&isMessageTime, TRUE);

    if (elapsed >= nextMessageTime) {
        if (!inputMode) {
            notificationContent.controlPanelServiceObjectPath = ((notificationContent.controlPanelServiceObjectPath == NULL) ? controlPanelServiceObjectPath : NULL); // Toggle notification with action ON/OFF
            if (priorityType == PRIORITY_TYPE_RANDOM) { // Randomize message type if priority type is RANDOM
                AJ_RandBytes((uint8_t*)&random, sizeof(random));
                messageType = (uint16_t)(random % AJNS_NUM_MESSAGE_TYPES);
            }
        } else {
            Producer_GetNotificationFromUser(&notificationContent, &messageType, &ttl, &nextMessageTime);
        }
        status = AJNS_Producer_SendNotification(busAttachment, &notificationContent, messageType, ttl, &serialNum);
        AJ_AlwaysPrintf(("Send Message Type: %u with TTL: %u secs returned: '%s'\n", messageType, ttl, AJ_StatusText(status)));
        if (inputMode) {
            Producer_FreeNotification(&notificationContent);
            PossiblyDeleteNotification(busAttachment);
        }
        if (!inputMode) {
            if (intervalType == INTERVAL_TYPE_RANDOM) { // Randomize next message time if interval type is RANDOM
                AJ_RandBytes((uint8_t*)&random, sizeof(random));
                if (elapsed < (MESSAGES_INTERVAL + offset)) {
                    offset += MESSAGES_INTERVAL - elapsed;
                } else {
                    offset = 0; // If we stalled too much simply randomize from now and cleared carried over offset
                }
                nextMessageTime = offset + (random % MESSAGES_INTERVAL); // randomize within the next time window of MESSAGES_INTERVAL
            }
        }
        AJ_AlwaysPrintf(("Next message will be sent in %u ms\n", nextMessageTime));
        AJ_InitTimer(&isMessageTime);
    }
}

void NotificationProducer_DoWork(AJ_BusAttachment* busAttachment)
{
    PossiblySendNotification(busAttachment);
}

AJ_Status NotificationProducer_Finish()
{
    return AJ_OK;
}

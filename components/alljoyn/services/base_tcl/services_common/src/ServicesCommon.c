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
 * The corresponding flag dbgAJSVC is defined in ServicesCommon.h and implemented
 * below.
 */
#define AJ_MODULE AJSVC
#include <aj_debug.h>

#include <alljoyn.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include <alljoyn/services_common/PropertyStore.h>
#ifdef ONBOARDING_SERVICE
#include <alljoyn/onboarding/OnboardingManager.h>
#endif
#include <aj_link_timeout.h>

/**
 * Turn on per-module debug printing by setting this variable to non-zero value
 * (usually in debugger).
 */
#ifndef NDEBUG
#ifndef ER_DEBUG_AJSVCALL
#define ER_DEBUG_AJSVCALL 0
#endif
#ifdef CONFIG_SERVICE
#ifndef ER_DEBUG_AJCFG
#define ER_DEBUG_AJCFG 0
#endif
AJ_EXPORT uint8_t dbgAJCFG = ER_DEBUG_AJCFG || ER_DEBUG_AJSVCALL;
#endif
#ifdef ONBOARDING_SERVICE
#ifndef ER_DEBUG_AJOBS
#define ER_DEBUG_AJOBS 0
#endif
AJ_EXPORT uint8_t dbgAJOBS = ER_DEBUG_AJOBS || ER_DEBUG_AJSVCALL;
#endif
#if defined(NOTIFICATION_SERVICE_CONSUMER) || defined(NOTIFICATION_SERVICE_PRODUCER)
#ifndef ER_DEBUG_AJNS
#define ER_DEBUG_AJNS 0
#endif
AJ_EXPORT uint8_t dbgAJNS = ER_DEBUG_AJNS || ER_DEBUG_AJSVCALL;
#endif
#ifdef CONTROLPANEL_SERVICE
#ifndef ER_DEBUG_AJCPS
#define ER_DEBUG_AJCPS 0
#endif
AJ_EXPORT uint8_t dbgAJCPS = ER_DEBUG_AJCPS || ER_DEBUG_AJSVCALL;
#endif
#ifndef ER_DEBUG_AJSVC
#define ER_DEBUG_AJSVC 0
#endif
AJ_EXPORT uint8_t dbgAJSVC = ER_DEBUG_AJSVC || ER_DEBUG_AJSVCALL;
#endif

uint8_t AJSVC_IsLanguageSupported(AJ_Message* msg, AJ_Message* reply, const char* language, int8_t* langIndex)
{
    uint8_t supported = TRUE;
    int8_t foundLangIndex = AJSVC_PropertyStore_GetLanguageIndex(language);
    if (foundLangIndex == AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX) {
        AJ_MarshalErrorMsg(msg, reply, AJSVC_ERROR_LANGUAGE_NOT_SUPPORTED);
        supported = FALSE;
    }
    if (langIndex != NULL) {
        *langIndex = foundLangIndex;
    }
    return supported;
}

AJ_Status AJSVC_MarshalAppIdAsVariant(AJ_Message* msg, const char* appId)
{
    AJ_Status status;
    uint8_t binAppId[UUID_LENGTH];
    uint32_t sz = strlen(appId);

    if (sz > UUID_LENGTH * 2) { // Crop application id that is too long
        sz = UUID_LENGTH * 2;
    }
    status = AJ_HexToRaw(appId, sz, binAppId, UUID_LENGTH);
    if (status != AJ_OK) {
        return status;
    }
    status = AJ_MarshalArgs(msg, "v", APP_ID_SIGNATURE, binAppId, sz / 2);

    return status;
}

AJ_Status AJSVC_MarshalAppId(AJ_Message* msg, const char* appId)
{
    AJ_Status status;
    uint8_t binAppId[UUID_LENGTH];
    uint32_t sz = strlen(appId);

    if (sz > UUID_LENGTH * 2) { // Crop application id that is too long
        sz = UUID_LENGTH * 2;
    }
    status = AJ_HexToRaw(appId, sz, binAppId, UUID_LENGTH);
    if (status != AJ_OK) {
        return status;
    }
    status = AJ_MarshalArgs(msg, APP_ID_SIGNATURE, binAppId, sz / 2);

    return status;
}

AJ_Status AJSVC_UnmarshalAppIdFromVariant(AJ_Message* msg, char* buf, size_t bufLen)
{
    AJ_Status status;
    uint8_t appId[UUID_LENGTH];
    size_t appIdLen;

    if (bufLen < (UUID_LENGTH * 2 + 1)) {
        AJ_ErrPrintf(("UnmarshalAppId: Insufficient buffer size! Should be at least %u but got %u\n", UUID_LENGTH * 2 + 1, (uint32_t)bufLen));
        return AJ_ERR_RESOURCES;
    }
    status = AJ_UnmarshalArgs(msg, "v", APP_ID_SIGNATURE, appId, &appIdLen);
    if (status != AJ_OK) {
        return status;
    }
    status = AJ_RawToHex((const uint8_t*)appId, appIdLen, buf, ((appIdLen > UUID_LENGTH) ? UUID_LENGTH : appIdLen) * 2 + 1, FALSE);

    return status;
}

AJ_Status AJSVC_UnmarshalAppId(AJ_Message* msg, char* buf, size_t bufLen)
{
    AJ_Status status;
    uint8_t appId[UUID_LENGTH];
    size_t appIdLen;

    if (bufLen < (UUID_LENGTH * 2 + 1)) {
        AJ_ErrPrintf(("UnmarshalAppId: Insufficient buffer size! Should be at least %u but got %u\n", UUID_LENGTH * 2 + 1, (uint32_t)bufLen));
        return AJ_ERR_RESOURCES;
    }
    status = AJ_UnmarshalArgs(msg, APP_ID_SIGNATURE, appId, &appIdLen);
    if (status != AJ_OK) {
        return status;
    }
    status = AJ_RawToHex((const uint8_t*)appId, appIdLen, buf, ((appIdLen > UUID_LENGTH) ? UUID_LENGTH : appIdLen) * 2 + 1, FALSE);

    return status;
}

AJ_Status AJSVC_RoutingNodeConnect(AJ_BusAttachment* busAttachment, const char* routingNodeName, uint32_t connectTimeout, uint32_t connectPause, uint32_t busLinkTimeout, uint8_t* isConnected)
{
    AJ_Status status = AJ_OK;
    const char* busUniqueName;

    while (TRUE) {
#ifdef ONBOARDING_SERVICE
        status = AJOBS_EstablishWiFi();
        if (status != AJ_OK) {
            AJ_AlwaysPrintf(("Failed to establish WiFi connectivity with status=%s\n", AJ_StatusText(status)));
            AJ_Sleep(connectPause);
            if (isConnected != NULL) {
                *isConnected = FALSE;
            }
            return status;
        }
#endif
        AJ_AlwaysPrintf(("Attempting to connect to bus '%s'\n", routingNodeName));
        status = AJ_FindBusAndConnect(busAttachment, routingNodeName, connectTimeout);
        if (status != AJ_OK) {
            AJ_AlwaysPrintf(("Failed attempt to connect to bus with status=%s, sleeping for %d seconds\n", AJ_StatusText(status), connectPause / 1000));
            AJ_Sleep(connectPause);
            continue;
        }
        busUniqueName = AJ_GetUniqueName(busAttachment);
        if (busUniqueName == NULL) {
            AJ_AlwaysPrintf(("Failed to GetUniqueName() from newly connected bus, retrying\n"));
            continue;
        }
        AJ_AlwaysPrintf(("Connected to Routing Node with BusUniqueName=%s\n", busUniqueName));
        break;
    }

    /* Configure timeout for the link to the Routing Node bus */
    AJ_SetBusLinkTimeout(busAttachment, busLinkTimeout);

    if (isConnected != NULL) {
        *isConnected = TRUE;
    }
    return status;
}

AJ_Status AJSVC_RoutingNodeDisconnect(AJ_BusAttachment* busAttachment, uint8_t disconnectWiFi, uint32_t preDisconnectPause, uint32_t postDisconnectPause, uint8_t* isConnected)
{
    AJ_Status status = AJ_OK;

    AJ_AlwaysPrintf(("AllJoyn disconnect\n"));
    AJ_Sleep(preDisconnectPause); // Sleep a little to let any pending requests to Routing Node to be sent
    AJ_Disconnect(busAttachment);
#ifdef ONBOARDING_SERVICE
    if (disconnectWiFi) {
        status = AJOBS_DisconnectWiFi();
    }
#endif
    AJ_Sleep(postDisconnectPause); // Sleep a little while before trying to reconnect

    if (isConnected != NULL) {
        *isConnected = FALSE;
    }
    return status;
}

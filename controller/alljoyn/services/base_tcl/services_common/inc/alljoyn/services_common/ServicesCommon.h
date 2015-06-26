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

#ifndef _SERVICES_COMMON_H_
#define _SERVICES_COMMON_H_

#include <alljoyn.h>

/**
 * Turn on per-module debug printing by setting this variable to non-zero value
 * (usually in debugger).
 */
#ifndef NDEBUG
#ifdef CONFIG_SERVICE
extern uint8_t dbgAJCFG;
#endif
#ifdef ONBOARDING_SERVICE
extern uint8_t dbgAJOBS;
#endif
#if defined(NOTIFICATION_SERVICE_CONSUMER) || defined(NOTIFICATION_SERVICE_PRODUCER)
extern uint8_t dbgAJNS;
#endif
#ifdef CONTROLPANEL_SERVICE
extern uint8_t dbgAJCPS;
#endif
extern uint8_t dbgAJSVC;
#endif

/**
 * Function prototype for return callback when a method call is completed.
 */
typedef void (*AJSVC_MethodCallCompleted)(AJ_Status status, void* context);

/**
 * Service Status is an enum that signals whether a call was handled
 * or not handled within an AJSVC_MessageProcessor function
 */
typedef enum _AJSVC_ServiceStatus {
    AJSVC_SERVICE_STATUS_HANDLED,       //!< SERVICE_STATUS_HANDLED
    AJSVC_SERVICE_STATUS_NOT_HANDLED,   //!< SERVICE_STATUS_NOT_HANDLED
} AJSVC_ServiceStatus;

/**
 * Function used to process request messages.
 * @param busAttachment
 * @param msg
 * @param msgStatus
 * @return serviceStatus
 */
typedef AJSVC_ServiceStatus (*AJSVC_MessageProcessor)(AJ_BusAttachment* busAttachment, AJ_Message* msg, AJ_Status* msgStatus);

/**
 * UpdateNotAllowed Error Message for services
 */
#define AJSVC_ERROR_UPDATE_NOT_ALLOWED     AJ_ErrUpdateNotAllowed

/**
 * InvalidValue Error Message for services
 */
#define AJSVC_ERROR_INVALID_VALUE          AJ_ErrInvalidValue

/**
 * FeatureNotAvailable Error Message for services
 */
#define AJSVC_ERROR_FEATURE_NOT_AVAILABLE  AJ_ErrFeatureNotAvailable

/**
 * MazSizeExceeded Error Message for services
 */
#define AJSVC_ERROR_MAX_SIZE_EXCEEDED      AJ_ErrMaxSizeExceeded

/**
 * LanguageNotSupported Error Message for services
 */
#define AJSVC_ERROR_LANGUAGE_NOT_SUPPORTED AJ_ErrLanguageNotSuppored

/**
 * returns the language index for the given language name possibly creating an error reply message if erred
 * @param msg
 * @param reply
 * @param language
 * @param langIndex
 * @return success
 */
uint8_t AJSVC_IsLanguageSupported(AJ_Message* msg, AJ_Message* reply, const char* language, int8_t* langIndex);

/**
 * Signature of the AppId field
 */
#define APP_ID_SIGNATURE "ay"

/**
 * Length of UUID that is used for the AppId field
 */
#define UUID_LENGTH 16

/**
 * Marshals the appId Hex string as a variant into the provided message.
 * @param msg       the message to marshal the appId into
 * @param appId     the application id to marshal
 * @return status
 */
AJ_Status AJSVC_MarshalAppIdAsVariant(AJ_Message* msg, const char* appId);

/**
 * Marshals the appId Hex string into the provided message.
 * @param msg       the message to marshal the appId into
 * @param appId     the application id to marshal
 * @return status
 */
AJ_Status AJSVC_MarshalAppId(AJ_Message* msg, const char* appId);

/**
 * Unmarshals the appId from a variant in the provided message.
 * @param msg       the message to unmarshal the appId from
 * @param buf       the buffer where the application id is unmarshalled into
 * @param bufLen    the size of the provided buffer. Should be UUID_LENGTH * 2 + 1.
 * @return status
 */
AJ_Status AJSVC_UnmarshalAppIdFromVariant(AJ_Message* msg, char* buf, size_t bufLen);

/**
 * Unmarshals the appId from the provided message.
 * @param msg       the message to unmarshal the appId from
 * @param buf       the buffer where the application id is unmarshalled into
 * @param bufLen    the size of the provided buffer. Should be UUID_LENGTH * 2 + 1.
 * @return status
 */
AJ_Status AJSVC_UnmarshalAppId(AJ_Message* msg, char* buf, size_t bufLen);

/**
 * Establish connection to named Routing Node
 * @param busAttachment
 * @param routingNodeName
 * @param connectTimeout
 * @param connectPause
 * @param busLinkTimeout
 * @param isConnected - state of connection to Routing Node after connect is performed
 * @return ajStatus - status of last request to Routing Node
 */
AJ_Status AJSVC_RoutingNodeConnect(AJ_BusAttachment* busAttachment, const char* routingNodeName, uint32_t connectTimeout, uint32_t connectPause, uint32_t busLinkTimeout, uint8_t* isConnected);

/**
 * Disconnect from Routing Node
 * @param busAttachment
 * @param disconnectWiFi
 * @param preDisconnectPause - a small pause before disconnect to allow for outgoing message to be dispatched
 * @param postDisconnectPause - a small pause after disconnect to allow for system to stablize
 * @param isConnected - state of connection to Rounting Node after disconnect is performed
 * @return ajStatus - status of last request to Routing Node
 */
AJ_Status AJSVC_RoutingNodeDisconnect(AJ_BusAttachment* busAttachment, uint8_t disconnectWiFi, uint32_t preDisconnectPause, uint32_t postDisconnectPause, uint8_t* isConnected);

// The following is the static registration of all services' bus objects

/*
 * For each service:
 * 1) Define pre objects - the amount of objects registered before the service
 * 2) If service is included:
 *    i)   include service header file(s)
 *    If service is NOT included:
 *    i)   define the default number of appobjects and number of objects
 *    ii)  define the default announce objects
 */
/*
 * ObjectsList definitions for ALL the services
 */
#ifdef CONFIG_SERVICE
/*
 * ObjectsList index for Config Service objects
 */
#define AJCFG_OBJECT_LIST_INDEX            3
#endif

#ifdef ONBOARDING_SERVICE
/*
 * ObjectsList index for Onboarding Service objects
 */
#define AJOBS_OBJECT_LIST_INDEX            4
#endif

#if defined(NOTIFICATION_SERVICE_PRODUCER) || defined(NOTIFICATION_SERVICE_CONSUMER)
/*
 * ObjectsList index for Notification Service objects
 */
#define AJNS_OBJECT_LIST_INDEX             5
#endif

#ifdef CONTROLPANEL_SERVICE
/*
 * ObjectsList index for ControlPanel Service objects
 */
#define AJCPS_OBJECT_LIST_INDEX            6
#endif

/*
 * ObjectsList index for Application objects
 */
#define AJAPP_OBJECTS_LIST_INDEX           7

/**
 * The NVRAM starting id for the PropertyStore
 */
#define AJ_PROPERTIES_NV_ID_BEGIN          (AJ_NVRAM_ID_CREDS_MAX + 1)
/**
 * The NVRAM maximum id for the PropertyStore
 */
#define AJ_PROPERTIES_NV_ID_MAX            (AJ_NVRAM_ID_CREDS_MAX + 1000)

#ifdef ONBOARDING_SERVICE
/**
 * The NVRAM starting id for the Onboarding Service
 */
#define AJ_OBS_NV_ID_BEGIN                 (AJ_PROPERTIES_NV_ID_MAX + 1)
#endif

#endif /* _SERVICES_COMMON_H_ */

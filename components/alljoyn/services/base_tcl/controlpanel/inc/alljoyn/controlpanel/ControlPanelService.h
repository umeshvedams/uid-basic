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

#ifndef CONTROLPANELRESPONSES_H_
#define CONTROLPANELRESPONSES_H_

#include <alljoyn/controlpanel/ControlPanelInterfaces.h>
#include <alljoyn/controlpanel/Definitions.h>

#ifdef CONTROLPANEL_SERVICE
#include <alljoyn/services_common/ServicesCommon.h>
#else
#include "ControlPanelClientGenerated.h"
#endif

/**
 * Send Signal if a property of a widget has changed.
 * @param busAttachment - busAttachment to use when sending the signal
 * @param propSignal - the msgId of the signal to send
 * @param sessionId - the sessionId to send it to
 * @return status - success/failure
 */
AJ_Status AJCPS_SendPropertyChangedSignal(AJ_BusAttachment* busAttachment, uint32_t propSignal, uint32_t sessionId);

/**
 * Send a signal to dismiss the displayed NotificationAction
 * @param busAttachment - busAttachment to use when sending the signal
 * @param propSignal - the msgId of the signal to send
 * @param sessionId - the sessionId to send it to
 * @return status - success/failure
 */
AJ_Status AJCPS_SendDismissSignal(AJ_BusAttachment* busAttachment, uint32_t propSignal, uint32_t sessionId);

/**
 * Function used to identify what kind of request we're dealing with. Defined in Generated code.
 * @param identifier - the msgId of the property/method
 * @param widgetType - if identified should be filled with the widget type
 * @param propType - if identified should be filled with the property requested
 * @param language - if identified should be filled with the language requested
 * @return widget - if identified should be filled with the pointer to the correct widget
 */
typedef void* (*AJCPS_IdentifyMsgOrPropId)(uint32_t identifier, uint16_t* widgetType, uint16_t* propType, uint16_t* language);

/**
 * Function used to identify what kind of signal we're sending. Defined in Generated code.
 * @param identifier - the msgId of the signal
 * @param isProperty - if identified should be filled with boolean to reflect if this is a property widget
 * @return widget - if identified should be filled with the pointer to the correct widget
 */
typedef void* (*AJCPS_IdentifyMsgOrPropIdForSignal)(uint32_t identifier, uint8_t* isProperty);

/**
 * Function used to identify what kind of request we're dealing with. Defined in Generated code.
 * @param identifier - the msgId of the property/method
 * @return true/false - true if identified/false if not found
 */
typedef uint8_t (*AJCPS_IdentifyRootMsgOrPropId)(uint32_t identifier);

/**
 * Start ControlPanel service framework passing callbacks from generated code
 * @param generatedObjectsList         - A NULL terminated array of generated objects which model the ControlPanel
 * @param generatedMessageProcessor    - A message processing callback for the generated objects' protocol
 * @param identifyMsgOrPropId - function pointer to call to identifyMsgOrPropId
 * @param identifyMsgOrPropIdForSignal - function pointer to call to identifyMsgOrPropIdForSignal
 * @param identifyRootMsgOrPropId - function pointer to call to identifyRootMsgOrPropId
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_Start(AJ_Object* generatedObjectsList, AJSVC_MessageProcessor generatedMessageProcessor, AJCPS_IdentifyMsgOrPropId identifyMsgOrPropId, AJCPS_IdentifyMsgOrPropIdForSignal identifyMsgOrPropIdForSignal, AJCPS_IdentifyRootMsgOrPropId identifyRootMsgOrPropId);

/**
 * Stop the control panel service if it has been previously started.
 * @return success/failure
 */
AJ_Status AJCPS_Stop();

/**
 * Return the current session's id.
 * @return sessionId
 */
uint32_t AJCPS_GetCurrentSessionId();

/**
 * Session request accept/reject function for service targeted session
 * @param port - port request came for
 * @param sessionId - sessionId for session
 * @param joiner - busName of joiner
 */
uint8_t AJCPS_CheckSessionAccepted(uint16_t port, uint32_t sessionId, char* joiner);

/**
 * Function called after busAttachment connects to Routing Node
 * @param busAttachment - busAttachment used
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_ConnectedHandler(AJ_BusAttachment* busAttachment);

/**
 * MessageProcessor function for the controlpanel service
 * @param busAttachment - busAttachment used
 * @param msg - message received
 * @param msgStatus - return status if necessary
 * @return serviceStatus - was message handled or not
 */
AJSVC_ServiceStatus AJCPS_MessageProcessor(AJ_BusAttachment* busAttachment, AJ_Message* msg, AJ_Status* msgStatus);

/**
 * Function called after busAttachment disconnects from Routing Node
 * @param busAttachment - busAttachment used
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_DisconnectHandler(AJ_BusAttachment* busAttachment);

/**
 * Send the root url of controlpanel model in reply to the message request.
 * @param msg - message to marshal
 * @param msgId - msgId received
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_SendRootUrl(AJ_Message* msg, uint32_t msgId);

/**
 * Get the widget property for the given propId and fill it in the provided message.
 * @param replyMsg - message to fill
 * @param propId - propId requested
 * @param context - context sent in
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_GetWidgetProperty(AJ_Message* replyMsg, uint32_t propId, void* context);

/**
 * Get the root property for the given propId and fill it in the provided message.
 * @param replyMsg - message to fill
 * @param propId - propId requested
 * @param context - context sent in
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_GetRootProperty(AJ_Message* replyMsg, uint32_t propId, void* context);

/**
 * Send all the root properties for the given msgId in reply to the message request.
 * @param msg - message to fill
 * @param msgId - msgId requested
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_GetAllRootProperties(AJ_Message* msg, uint32_t msgId);

/**
 * Send all the widget properties for the given msgId in reply to the message request.
 * @param msg - message to fill
 * @param msgId - msgId requested
 * @return aj_status - success/failure
 */
AJ_Status AJCPS_GetAllWidgetProperties(AJ_Message* msg, uint32_t msgId);

#endif /* CONTROLPANELRESPONSES_H_ */

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

#ifndef CONTROL_MARSHAL_UTIL_H_
#define CONTROL_MARSHAL_UTIL_H_

/** @defgroup ControlMarshalUtil Control Marshaling Utilities
 * details Functions that assist in Marshaling Controls.
 *  @{
 */

#include <alljoyn.h>
#include <alljoyn/controlpanel/Definitions.h>
#include <alljoyn/controlpanel/Common/BaseWidget.h>

/**
 * StartOptionalParams - Start Marshaling the Optional Parameters' container array.
 * @param reply - message to marshal into
 * @param arg - arg to hold the container
 * @return aj_status - success/failure
 */
#define StartOptionalParams(reply, arg) AJ_MarshalContainer(reply, arg, AJ_ARG_ARRAY)

/**
 * Add a layout Hint. Can be used for all kinds of MetaData (Property, Action etc)
 * @param reply - message to marshal into
 * @param hints - hints to marshal
 * @param numHints - number of hints to marshal
 * @return aj_status - success/failure
 */
AJ_Status AddHints(AJ_Message* reply, const uint16_t hints[], uint16_t numHints);

/**
 * Add Constraint Value
 * @param reply - message to marshal into
 * @param sig - signature of value
 * @param value - value to marshal
 * @param displayValue - displayValue to marshal
 * @return aj_status - success/failure
 */
AJ_Status AddConstraintValue(AJ_Message* reply, const char* sig, const void* value, const char* displayValue);

/**
 * Add ConstraintRange component
 * @param reply - message to marshal into
 * @param valueSig - signature of value
 * @param min - min Value
 * @param max - max Value
 * @param increment - increment Value
 * @return aj_status - success/failure
 */
AJ_Status AddConstraintRange(AJ_Message* reply, const char* valueSig, const void* min, const void* max, const void* increment);

/**
 * Start a Complex OptionalParam component
 * @param reply - message to marshal into
 * @param arg - arg to hold value
 * @param key - key in dictionary
 * @param sig - signature of value
 * @return aj_status - success/failure
 */
AJ_Status StartComplexOptionalParam(AJ_Message* reply, AJ_Arg* arg, uint16_t key, const char* sig);

/**
 * Add a basic type Optional Param
 * @param reply - message to marshal into
 * @param key - key in dictionary
 * @param sig - signature of value
 * @param value - value to marshal
 * @return aj_status - success/failure
 */
AJ_Status AddBasicOptionalParam(AJ_Message* reply, uint16_t key, const char* sig, const void* value);

/**
 * Add a property for GetAll response
 * @param reply - message to marshal into
 * @param key - key in dictionary
 * @param sig - signature of value
 * @param widget - widget being marshalled
 * @param language - language requested
 * @param functionPtr - function Pointer to marshal the value
 * @return aj_status - success/failure
 */
AJ_Status AddPropertyForGetAll(AJ_Message* reply, char* key, const char* sig,
                               BaseWidget* widget, uint16_t language, MarshalWidgetFptr functionPtr);

/**
 * Helper functions. Receives sig value and void* and does the marshalling
 * @param reply - message to marshal into
 * @param sig - signature of value
 * @param value - value as void*
 * @return aj_status - success/failure
 */
AJ_Status MarshalVariant(AJ_Message* reply, const char* sig, const void* value);

/**
 * MarshalAllRootProperties - includes only version
 * @param reply - message to marshal into
 * @return aj_status - success/failure
 */
AJ_Status MarshalAllRootProperties(AJ_Message* reply);

/**
 * Marshal Version Property for root interfaces
 * @param reply - message to marshal into
 * @return aj_status - success/failure
 */
AJ_Status MarshalVersionRootProperties(AJ_Message* reply);

/** @} */
#endif /* CONTROL_SERVICE_H_ */

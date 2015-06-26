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

#ifndef LABELWIDGET_H_
#define LABELWIDGET_H_

/** @defgroup LabelWidget Label Widget
 *
 *  @{
 */

#include <alljoyn/controlpanel/Common/BaseWidget.h>

/////////////////////////*     LabelWidget     *//////////////////////////////////////

/**
 * LabelWidget structure - widget to represent a Label
 */
typedef struct LabelWidget {
    BaseWidget base;                                                   //!< Internal BaseWidget

    const char* const* label;                                          //!< The labels of the Widget. Array of labels - one per language
    const char* (*getLabel)(struct LabelWidget* thisWidget, uint16_t); //!< The GetLabel function pointer. Receives a language index and should return the label for that language
} LabelWidget;

/**
 * Initialize the LabelWidget structure
 * @param widget - pointer to LabelWidget structure
 */
void initializeLabelWidget(LabelWidget* widget);

/**
 * Marshal Label of given widget into given reply message
 * @param widget - pointer to widget
 * @param reply - message to marshal into
 * @param language - language requested
 * @return aj_status - success/failure
 */
AJ_Status marshalLabelLabel(LabelWidget* widget, AJ_Message* reply, uint16_t language);

/**
 * Marshal All LabelProperties of given widget into given reply message
 * @param widget - pointer to widget
 * @param reply - message to marshal into
 * @param language - language requested
 * @return aj_status - success/failure
 */
AJ_Status marshalAllLabelProperties(BaseWidget* widget, AJ_Message* reply, uint16_t language);

/** @} */
#endif /* LABELWIDGET_H_ */


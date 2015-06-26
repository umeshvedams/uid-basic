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

#ifndef EVENTSANDACTIONS_H_
#define EVENTSANDACTIONS_H_

#include <alljoyn.h>

AJ_Status EventsAndActions_Init(const char* const* descriptionLanguages);

void EventsAndActions_DoWork(AJ_BusAttachment* busAttachment);

AJ_Status EventsAndActions_Finish(AJ_BusAttachment* busAttachment);

AJSVC_ServiceStatus EventsAndActionsMessageProcessor(AJ_BusAttachment* busAttachment, AJ_Message* msg, AJ_Status* msgStatus);

#endif /* EVENTSANDACTIONS_H_ */

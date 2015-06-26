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

#ifndef CONTROLPANELPROVIDED_H_
#define CONTROLPANELPROVIDED_H_

#include "alljoyn.h"
#include "alljoyn/controlpanel/Common/DateTimeUtil.h"
#include "ControlPanelGenerated.h"

//////////////////////////////////////////////////////

void disableFan();
void enableFan();
void disableTempSelect();
void enableTempSelect();

const char* getNotificationString();
const char* getNotificationActionObjPath();
uint16_t isThereANotificationToSend();

uint16_t getCurrentTargetTemp();

void checkTargetTempReached();

void setTemperatureFieldUpdate();
void setStatusFieldUpdate();
void setTempSelectorFieldUpdate();
void setFanSpeedSelectorFieldUpdate();

//char const* getCurrentTemperatureString(uint16_t lang);
//char const* getCurrentHumidityString(uint16_t lang);

void* getCurrentTemperatureString(PropertyWidget* thisWidget);
void setCurrentTemperatureString(char const* newTemp);
void* getCurrentHumidityString(PropertyWidget* thisWidget);
void setCurrentHumidityString(char const* newHumidity);

void* getTargetTemperature(PropertyWidget* thisWidget);
void setTargetTemperature(uint16_t newTemp);

void* getCurrentMode(PropertyWidget* thisWidget);
void setCurrentMode(uint16_t newMode);

void* getFanSpeed(PropertyWidget* thisWidget);
void setFanSpeed(uint16_t newSpeed);

void* getStatusString(PropertyWidget* thisWidget);
void setStatusString(const char* newStatusString);

void onTurnFanOn(ExecuteActionContext* context, uint8_t accepted);
void onTurnFanOff(ExecuteActionContext* context, uint8_t accepted);

//void simulateTemperatureChange();
uint8_t checkForUpdatesToSend();
uint8_t checkForEventsToSend();

#endif /* CONTROLPANELPROVIDED_H_ */


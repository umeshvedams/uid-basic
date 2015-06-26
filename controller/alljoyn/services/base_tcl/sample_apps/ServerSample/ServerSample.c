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

#include <alljoyn.h>
#include <aj_config.h>
#include <aj_creds.h>
#include <aj_nvram.h>
#include <aj_link_timeout.h>

#include "PropertyStoreOEMProvisioning.h"
#ifdef ONBOARDING_SERVICE
    #include <alljoyn/onboarding/OnboardingManager.h>
#endif
#ifdef CONTROLPANEL_SERVICE
    #include "ControlPanelGenerated.h"
#endif

#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include "AppHandlers.h"

#ifndef NDEBUG
#ifndef ER_DEBUG_AJSVCAPP
#define ER_DEBUG_AJSVCAPP 0
#endif
AJ_EXPORT uint8_t dbgAJSVCAPP = ER_DEBUG_AJSVCAPP;
#endif

/**
 * Application wide globals
 */

#ifndef ROUTING_NODE_NAME
#define ROUTING_NODE_NAME "org.alljoyn.BusNode"
#endif
static uint8_t isBusConnected = FALSE;

/*
 * Define timeout/pause values. Values are in milli seconds.
 * The following may be tuned according to platform requirements such as battery usage.
 */
#define AJAPP_BUS_LINK_TIMEOUT    60
#define AJAPP_CONNECT_TIMEOUT     (1000 * 60 * 10) // Override AJ_CONNECT_TIMEOUT to wait longer for a successful connection to a Routing Node
#define AJAPP_CONNECT_PAUSE       (1000 * 2)       // Override AJ_CONNECT_PAUSE to be more responsive
#define AJAPP_SLEEP_TIME          (1000 * 2)       // A little pause to let things settle
#define AJAPP_UNMARSHAL_TIMEOUT   (1000 * 1)       // Override AJ_UNMARSHAL_TIMEOUT to be more responsive

#define AJAPP_MAX_INIT_ATTEPTS    3 // Maximum number of attempts to initialize the services

static AJ_BusAttachment busAttachment;
#define AJ_ABOUT_SERVICE_PORT     900

/**
 * Application wide callbacks
 */

static uint32_t MyBusAuthPwdCB(uint8_t* buf, uint32_t bufLen)
{
    const char* myRoutingNodePwd = "000000";
    strncpy((char*)buf, myRoutingNodePwd, bufLen);
    return (uint32_t)strlen(myRoutingNodePwd);
}

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    AJ_Status status = AJ_OK;
#ifdef CONFIG_SERVICE
    const char* hexPassword = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_PASSCODE);
#else
    const char* hexPassword = "303030303030";
#endif
    size_t hexPasswordLen;
    uint32_t len = 0;

    if (hexPassword == NULL) {
        AJ_AlwaysPrintf(("Password is NULL!\n"));
        return len;
    }
    AJ_AlwaysPrintf(("Configured password=%s\n", hexPassword));
    hexPasswordLen = strlen(hexPassword);
    len = hexPasswordLen / 2;
    status = AJ_HexToRaw(hexPassword, hexPasswordLen, buffer, bufLen);
    if (status == AJ_ERR_RESOURCES) {
        len = 0;
    }

    return len;
}

/**
 * Services Provisioning
 */

#ifndef DEVICE_MANUFACTURER
#define DEVICE_MANUFACTURER "COMPANY"
#endif
#ifndef DEVICE_PRODUCT_NAME
#define DEVICE_PRODUCT_NAME "GENERIC BOARD"
#endif
const char* deviceManufactureName = DEVICE_MANUFACTURER;
const char* deviceProductName = DEVICE_PRODUCT_NAME;

static const char DEFAULT_LANGUAGE[] = "en";
static const char* DEFAULT_LANGUAGES[] = { DEFAULT_LANGUAGE };
static const char SUPPORTED_LANG2[] = "de-AT";
static const char* SUPPORTED_LANGUAGES[] = { DEFAULT_LANGUAGE, SUPPORTED_LANG2, NULL };
const char* const* propertyStoreDefaultLanguages = SUPPORTED_LANGUAGES;

/**
 * property array of default values
 */
static const char DEFAULT_DEVICE_NAME_LANG1[] = { "" }; // Leave empty to be generated at run-time
static const char DEFAULT_DEVICE_NAME_LANG2[] = { "" }; // Leave empty to be generated at run-time
static const char* DEFAULT_DEVICE_NAMES[] = { DEFAULT_DEVICE_NAME_LANG1, DEFAULT_DEVICE_NAME_LANG2 };
#if defined CONFIG_SERVICE
static const char* DEFAULT_PASSCODES[] = { "303030303030" }; // HEX encoded { '0', '0', '0', '0', '0', '0' }
#endif
#if   defined CONTROLPANEL_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Controllee" };
#elif defined NOTIFICATION_SERVICE_PRODUCER
static const char* DEFAULT_APP_NAMES[] = { "Notifier" };
#elif defined ONBOARDING_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Onboardee" };
#elif defined CONFIG_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Configuree" };
#else
static const char* DEFAULT_APP_NAMES[] = { "Announcer" };
#endif
static const char DEFAULT_DESCRIPTION_LANG1[] = "My first IOE device";
static const char DEFAULT_DESCRIPTION_LANG2[] = "Mein erstes IOE Geraet";
static const char* DEFAULT_DESCRIPTIONS[] = { DEFAULT_DESCRIPTION_LANG1, DEFAULT_DESCRIPTION_LANG2 };
static const char DEFAULT_MANUFACTURER_LANG1[] = "Company A(EN)";
static const char DEFAULT_MANUFACTURER_LANG2[] = "Firma A(DE-AT)";
static const char* DEFAULT_MANUFACTURERS[] = { DEFAULT_MANUFACTURER_LANG1, DEFAULT_MANUFACTURER_LANG2 };
static const char* DEFAULT_DEVICE_MODELS[] = { "0.0.1" };
static const char* DEFAULT_DATE_OF_MANUFACTURES[] = { "2014-05-01" };
static const char* DEFAULT_SOFTWARE_VERSIONS[] = { "0.0.1" };
static const char* DEFAULT_HARDWARE_VERSIONS[] = { "0.0.1" };
static const char DEFAULT_SUPPORT_URL_LANG1[] = "http://www.company_a.com";
static const char DEFAULT_SUPPORT_URL_LANG2[] = "http://www.company_a.com/de-AT";
static const char* DEFAULT_SUPPORT_URLS[] = { DEFAULT_SUPPORT_URL_LANG1, DEFAULT_SUPPORT_URL_LANG2 };

const char** propertyStoreDefaultValues[AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS] =
{
// "Default Values per language",    "Key Name"
    NULL,                           /*DeviceId*/
    NULL,                           /*AppId*/
    DEFAULT_DEVICE_NAMES,           /*DeviceName*/
    DEFAULT_LANGUAGES,              /*DefaultLanguage*/
#if defined CONFIG_SERVICE
    DEFAULT_PASSCODES,              /*Passcode*/
    NULL,                           /*RealmName*/
#endif
// Add other runtime or configurable keys above this line
    DEFAULT_APP_NAMES,              /*AppName*/
    DEFAULT_DESCRIPTIONS,           /*Description*/
    DEFAULT_MANUFACTURERS,          /*Manufacturer*/
    DEFAULT_DEVICE_MODELS,          /*ModelNumber*/
    DEFAULT_DATE_OF_MANUFACTURES,   /*DateOfManufacture*/
    DEFAULT_SOFTWARE_VERSIONS,      /*SoftwareVersion*/
    NULL,                           /*AJSoftwareVersion*/
#if defined CONFIG_SERVICE
    NULL,                           /*MaxLength*/
#endif
// Add other mandatory about keys above this line
    DEFAULT_HARDWARE_VERSIONS,      /*HardwareVersion*/
    DEFAULT_SUPPORT_URLS,           /*SupportUrl*/
// Add other optional about keys above this line
};

/**
 * properties array of runtime values' buffers
 */
static char machineIdVar[MACHINE_ID_LENGTH + 1] = { 0 };
static char* machineIdVars[] = { machineIdVar };
static char deviceNameVarLang1[DEVICE_NAME_VALUE_LENGTH + 1] = { 0 };
static char deviceNameVarLang2[DEVICE_NAME_VALUE_LENGTH + 1] = { 0 };
static char* deviceNameVars[] = { deviceNameVarLang1, deviceNameVarLang2 };
#ifdef CONFIG_SERVICE
static char defaultLanguageVar[LANG_VALUE_LENGTH + 1] = { 0 };
static char* defaultLanguageVars[] = { defaultLanguageVar };
static char passcodeVar[PASSWORD_VALUE_LENGTH + 1] = { 0 };
static char* passcodeVars[] = { passcodeVar };
static char realmNameVar[KEY_VALUE_LENGTH + 1] = { 0 };
static char* realmNameVars[] = { realmNameVar };
#endif

PropertyStoreConfigEntry propertyStoreRuntimeValues[AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS] =
{
//  {"Buffers for Values per language", "Buffer Size"},                  "Key Name"
    { machineIdVars,             MACHINE_ID_LENGTH + 1 },               /*DeviceId*/
    { machineIdVars,             MACHINE_ID_LENGTH + 1 },               /*AppId*/
    { deviceNameVars,            DEVICE_NAME_VALUE_LENGTH + 1 },        /*DeviceName*/
#ifdef CONFIG_SERVICE
    { defaultLanguageVars,       LANG_VALUE_LENGTH + 1 },               /*DefaultLanguage*/
    { passcodeVars,              PASSWORD_VALUE_LENGTH + 1 },           /*Passcode*/
    { realmNameVars,             KEY_VALUE_LENGTH + 1 },                /*RealmName*/
#endif
};

const char* aboutIconMimetype = { "image/png" };
const uint8_t aboutIconContent[] =
{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52 \
    , 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x32, 0x02, 0x03, 0x00, 0x00, 0x00, 0x63, 0x51, 0x60 \
    , 0x22, 0x00, 0x00, 0x00, 0x0c, 0x50, 0x4c, 0x54, 0x45, 0x65, 0x2d, 0x67, 0xeb, 0x00, 0x88, 0xec \
    , 0x06, 0x8d, 0xf1, 0x44, 0xaa, 0x1f, 0x54, 0xd3, 0x5b, 0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e \
    , 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00, 0xe8, 0x49, 0x44, 0x41, 0x54, 0x28, 0xcf \
    , 0x95, 0x92, 0x51, 0x0a, 0xc3, 0x20, 0x0c, 0x86, 0xd3, 0xc0, 0x40, 0x84, 0xdd, 0x61, 0xec, 0x49 \
    , 0x72, 0x9f, 0xde, 0x47, 0xfa, 0x34, 0x3c, 0x45, 0x18, 0x85, 0x49, 0x4e, 0xb9, 0x18, 0xab, 0xd6 \
    , 0xb1, 0x97, 0x06, 0xb4, 0x7e, 0xa6, 0x26, 0xbf, 0x89, 0x00, 0x97, 0xec, 0xb6, 0x9e, 0xc9, 0x8b \
    , 0x0e, 0xee, 0x04, 0x40, 0x92, 0x1b, 0x49, 0x04, 0x7a, 0xcb, 0x01, 0x28, 0x20, 0xc4, 0xd4, 0x7c \
    , 0x0f, 0x90, 0x11, 0x04, 0x39, 0xd0, 0x29, 0x24, 0xd3, 0x39, 0x41, 0x0c, 0x53, 0x3e, 0x4c, 0x1b \
    , 0x4b, 0x4f, 0x87, 0x29, 0x65, 0x49, 0x7b, 0x89, 0x01, 0x64, 0x91, 0x44, 0xf6, 0x2a, 0xc4, 0x26 \
    , 0xf1, 0x1f, 0x5d, 0x10, 0xbb, 0xba, 0xe5, 0x77, 0x93, 0x15, 0x4c, 0x40, 0xb5, 0x64, 0xc1, 0x9a \
    , 0x66, 0x37, 0x91, 0x2d, 0x10, 0xda, 0xf5, 0x9e, 0xba, 0xc0, 0xad, 0x39, 0x31, 0xea, 0xc0, 0xfe \
    , 0xab, 0x2b, 0x5b, 0x9d, 0x42, 0x11, 0x3e, 0xd0, 0x68, 0x5c, 0x18, 0x13, 0x74, 0xf2, 0x01, 0x4b \
    , 0x71, 0xea, 0x95, 0x3d, 0x05, 0x56, 0xcc, 0x5a, 0xb9, 0xb2, 0x19, 0x20, 0xfb, 0xa8, 0x5f, 0x3e \
    , 0x0a, 0xcd, 0xc4, 0x07, 0x89, 0xd3, 0x84, 0xcd, 0xb7, 0xa8, 0x8b, 0x4c, 0x4f, 0x39, 0xb7, 0x68 \
    , 0xd6, 0x1a, 0xbc, 0xcc, 0xf7, 0x58, 0x7c, 0xad, 0x43, 0x77, 0x8d, 0xf3, 0xd2, 0x72, 0x0c, 0xd2 \
    , 0x16, 0x0d, 0x95, 0x34, 0x91, 0xfa, 0x46, 0x67, 0x21, 0x45, 0xcb, 0xd0, 0x1a, 0x56, 0xc7, 0x41 \
    , 0x7a, 0xc6, 0xe7, 0x89, 0xe4, 0x3f, 0x81, 0x51, 0xfc, 0x79, 0x3f, 0xc3, 0x96, 0xf5, 0xda, 0x5b \
    , 0x84, 0x2f, 0x85, 0x3b, 0x47, 0x0d, 0xe8, 0x0d, 0xca, 0xd3, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45 \
    , 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, 0x82 };
const size_t aboutIconContentSize = sizeof(aboutIconContent);
const char* aboutIconUrl = { "https://www.alljoyn.org/sites/all/themes/at_alljoyn/images/img-alljoyn-logo.png" };

/**
 * The AllJoyn Message Loop
 */

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    uint8_t isUnmarshalingSuccessful = FALSE;
    AJSVC_ServiceStatus serviceStatus;
    AJ_Message msg;
    uint8_t forcedDisconnnect = FALSE;
    uint8_t rebootRequired = FALSE;

    AJ_Initialize();

    AJ_AboutSetIcon(aboutIconContent, aboutIconContentSize, aboutIconMimetype, aboutIconUrl);

    status = PropertyStore_Init();
    if (status != AJ_OK) {
        goto Exit;
    }

    status = AJServices_Init(AJ_ABOUT_SERVICE_PORT, deviceManufactureName, deviceProductName);
    if (status != AJ_OK) {
        goto Exit;
    }

    SetBusAuthPwdCallback(MyBusAuthPwdCB);

    while (TRUE) {
        status = AJ_OK;
        serviceStatus = AJSVC_SERVICE_STATUS_NOT_HANDLED;

        if (!isBusConnected) {
            status = AJSVC_RoutingNodeConnect(&busAttachment, ROUTING_NODE_NAME, AJAPP_CONNECT_TIMEOUT, AJAPP_CONNECT_PAUSE, AJAPP_BUS_LINK_TIMEOUT, &isBusConnected);
            if (!isBusConnected) { // Failed to connect to Routing Node?
                continue; // Retry establishing connection to Routing Node.
            }
            /* Setup password based authentication listener for secured peer to peer connections */
            AJ_BusSetPasswordCallback(&busAttachment, PasswordCallback);
        }

        status = AJApp_ConnectedHandler(&busAttachment, AJAPP_MAX_INIT_ATTEPTS, AJAPP_SLEEP_TIME);

        if (status == AJ_OK) {
            status = AJ_UnmarshalMsg(&busAttachment, &msg, AJAPP_UNMARSHAL_TIMEOUT);
            isUnmarshalingSuccessful = (status == AJ_OK);

            if (status == AJ_ERR_TIMEOUT) {
                if (AJ_ERR_LINK_TIMEOUT == AJ_BusLinkStateProc(&busAttachment)) {
                    status = AJ_ERR_READ;             // something's not right. force disconnect
                } else {                              // nothing on bus, do our own thing
                    AJApp_DoWork(&busAttachment);
                    continue;
                }
            }

            if (isUnmarshalingSuccessful) {
                if (serviceStatus == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                    serviceStatus = AJApp_MessageProcessor(&busAttachment, &msg, &status);
                }
                if (serviceStatus == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                    //Pass to the built-in bus message handlers
                    status = AJ_BusHandleBusMessage(&msg);
                }
                AJ_NotifyLinkActive();
            }

            //Unmarshaled messages must be closed to free resources
            AJ_CloseMsg(&msg);
        }

        if (status == AJ_ERR_READ || status == AJ_ERR_RESTART || status == AJ_ERR_RESTART_APP) {
            if (isBusConnected) {
                forcedDisconnnect = (status != AJ_ERR_READ);
                rebootRequired = (status == AJ_ERR_RESTART_APP);
                AJApp_DisconnectHandler(&busAttachment, forcedDisconnnect);
                AJSVC_RoutingNodeDisconnect(&busAttachment, forcedDisconnnect, AJAPP_SLEEP_TIME, AJAPP_SLEEP_TIME, &isBusConnected);
                if (rebootRequired) {
                    AJ_Reboot();
                }
            }
        }
    }     // while (TRUE)

    return 0;

Exit:

    return (1);
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif

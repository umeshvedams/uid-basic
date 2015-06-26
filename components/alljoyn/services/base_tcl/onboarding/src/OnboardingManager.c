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
 * The corresponding flag dbgAJOBS is defined in ServicesCommon.h and implemented
 * in ServicesCommon.c.
 */
#define AJ_MODULE AJOBS
#include <aj_debug.h>
#include <alljoyn.h>
#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include <alljoyn/onboarding/OnboardingService.h>
#include <alljoyn/onboarding/OnboardingControllerAPI.h>
#include <aj_wifi_ctrl.h>
#include <alljoyn/onboarding/OnboardingManager.h>
#include <aj_nvram.h>

/**
 * State and Error
 */
static AJOBS_State obState = AJOBS_STATE_NOT_CONFIGURED;
static AJOBS_Error obLastError = { 0, 0 };

/**
 * ScanInfos
 */
static AJ_Time obLastScan;
static AJOBS_ScanInfo obScanInfos[AJOBS_MAX_SCAN_INFOS];
static uint8_t obScanInfoCount = 0;

/**
 * Onboarding timer activated state.
 */
static uint8_t bFirstStart = TRUE;

static const AJOBS_Settings* obSettings;

#define AJ_OBS_OBINFO_NV_ID AJ_OBS_NV_ID_BEGIN

static AJ_Status OnboardingReadInfo(AJOBS_Info* info)
{
    AJ_Status status = AJ_OK;
    size_t size = sizeof(AJOBS_Info);
    AJ_NV_DATASET* nvramHandle;
    int sizeRead;

    if (NULL == info) {
        return AJ_ERR_NULL;
    }

    if (!AJ_NVRAM_Exist(AJ_OBS_OBINFO_NV_ID)) {
        return AJ_ERR_INVALID;
    }

    memset(info, 0, size);
    nvramHandle = AJ_NVRAM_Open(AJ_OBS_OBINFO_NV_ID, "r", 0);
    if (nvramHandle != NULL) {
        sizeRead = AJ_NVRAM_Read(info, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeRead != sizeRead) {
            status = AJ_ERR_NVRAM_READ;
        } else {
            AJ_AlwaysPrintf(("Read Info values: state=%d, ssid=%s authType=%d pc=%s\n", info->state, info->ssid, info->authType, info->pc));
        }
    }

    return status;
}

static AJ_Status OnboardingWriteInfo(AJOBS_Info* info)
{
    AJ_Status status = AJ_OK;
    size_t size = sizeof(AJOBS_Info);
    AJ_NV_DATASET* nvramHandle;
    int sizeWritten;

    if (NULL == info) {
        return AJ_ERR_NULL;
    }

    AJ_AlwaysPrintf(("Going to write Info values: state=%d, ssid=%s authType=%d pc=%s\n", info->state, info->ssid, info->authType, info->pc));

    nvramHandle = AJ_NVRAM_Open(AJ_OBS_OBINFO_NV_ID, "w", size);
    if (nvramHandle != NULL) {
        sizeWritten = AJ_NVRAM_Write(info, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeWritten != size) {
            status = AJ_ERR_NVRAM_WRITE;
        }
    }

    return status;
}

AJ_Status AJOBS_Start(const AJOBS_Settings* settings)
{
    AJ_Status status = AJ_OK;

    obSettings = settings;
    if (obSettings == NULL || obSettings->AJOBS_SoftAPSSID == NULL || (obSettings->AJOBS_SoftAPSSID)[0] == '\0') {
        AJ_ErrPrintf(("AJOBS_Start(): No SoftAP SSID was provided!\n"));
        status = AJ_ERR_INVALID;
    } else if (obSettings->AJOBS_SoftAPPassphrase != NULL && strlen(obSettings->AJOBS_SoftAPPassphrase) < 8) {
        AJ_ErrPrintf(("AJOBS_Start(): SoftAP Passphrase is too short! Needs to 8 to 63 characters long\n"));
        status = AJ_ERR_INVALID;
    }

    if (status == AJ_OK) {
        status = AJOBS_RegisterObjectList();
    }

    return status;
}

uint8_t AJOBS_IsWiFiConnected()
{
    AJ_WiFiConnectState state = AJ_GetWifiConnectState();

    switch (state) {
    case AJ_WIFI_STATION_OK:
    case AJ_WIFI_CONNECT_OK:
        return TRUE;

    default:
        return FALSE;
    }
}

/**
 * Onboarding state variable.
 */
AJOBS_State AJOBS_GetState()
{
    return obState;
}

void AJOBS_SetState(AJOBS_State state)
{
    obState = state;
}

/**
 * Onboarding error variable.
 */
const AJOBS_Error* AJOBS_GetError()
{
    return &obLastError;
}

/**
 * Onboarding last scan time.
 */
const AJ_Time* AJOBS_GetLastScanTime()
{
    return &obLastScan;
}

/**
 * Onboarding scan infos variable.
 */
const AJOBS_ScanInfo* AJOBS_GetScanInfos()
{
    return obScanInfos;
}

/**
 * Onboarding scan infos count variable.
 */
uint8_t AJOBS_GetScanInfoCount()
{
    return AJOBS_MAX_SCAN_INFOS;
}

static const AJOBS_Info defaultInfo = AJOBS_INFO_DEFAULT;

AJ_Status AJOBS_GetInfo(AJOBS_Info* obInfo)
{
    AJ_Status status = OnboardingReadInfo(obInfo);
    AJ_InfoPrintf(("ReadInfo status: %s\n", AJ_StatusText(status)));
    if (status == AJ_ERR_INVALID) {
        *obInfo = defaultInfo;
        status = AJ_OK; // Handle does not exist so return default info
    }
    return status;
}

AJ_Status AJOBS_SetInfo(AJOBS_Info* obInfo)
{
    AJ_Status status = OnboardingWriteInfo(obInfo);
    AJ_InfoPrintf(("WriteInfo status: %s\n", AJ_StatusText(status)));
    return status;
}

#ifndef AJ_CASE
#define AJ_CASE(_status) case _status: return # _status
#endif

static const char* AJ_WiFiConnectStateText(AJ_WiFiConnectState state)
{
#ifdef NDEBUG
    /* Expectation is that thin client status codes will NOT go beyond 255 */
    static char code[4];

#ifdef _WIN32
    _snprintf(code, sizeof(code), "%03u", state);
#else
    snprintf(code, sizeof(code), "%03u", state);
#endif

    return code;
#else
    switch (state) {
        AJ_CASE(AJ_WIFI_IDLE);
        AJ_CASE(AJ_WIFI_CONNECTING);
        AJ_CASE(AJ_WIFI_CONNECT_OK);
        AJ_CASE(AJ_WIFI_SOFT_AP_INIT);
        AJ_CASE(AJ_WIFI_SOFT_AP_UP);
        AJ_CASE(AJ_WIFI_STATION_OK);
        AJ_CASE(AJ_WIFI_CONNECT_FAILED);
        AJ_CASE(AJ_WIFI_AUTH_FAILED);
        AJ_CASE(AJ_WIFI_DISCONNECTING);

    default:
        return "<unknown>";
    }
#endif
}

int8_t AJOBS_ControllerAPI_IsWiFiSoftAP()
{
    AJ_WiFiConnectState state = AJ_GetWifiConnectState();
    switch (state) {
    case AJ_WIFI_SOFT_AP_INIT:
    case AJ_WIFI_SOFT_AP_UP:
    case AJ_WIFI_STATION_OK:
        return TRUE;

    case AJ_WIFI_IDLE:
    case AJ_WIFI_CONNECTING:
    case AJ_WIFI_CONNECT_OK:
    case AJ_WIFI_CONNECT_FAILED:
    case AJ_WIFI_AUTH_FAILED:
    case AJ_WIFI_DISCONNECTING:
    default:
        return FALSE;
    }
}

int8_t AJOBS_ControllerAPI_IsWiFiClient()
{
    AJ_WiFiConnectState state = AJ_GetWifiConnectState();
    switch (state) {
    case AJ_WIFI_CONNECTING:
    case AJ_WIFI_CONNECT_OK:
    case AJ_WIFI_DISCONNECTING:
    case AJ_WIFI_CONNECT_FAILED:
    case AJ_WIFI_AUTH_FAILED:
        return TRUE;

    case AJ_WIFI_IDLE:
    case AJ_WIFI_SOFT_AP_INIT:
    case AJ_WIFI_SOFT_AP_UP:
    case AJ_WIFI_STATION_OK:
    default:
        return FALSE;
    }
}

static AJOBS_AuthType GetAuthType(AJ_WiFiSecurityType secType, AJ_WiFiCipherType cipherType)
{
    switch (secType) {
    case AJ_WIFI_SECURITY_NONE:
        return AJOBS_AUTH_TYPE_OPEN;

    case AJ_WIFI_SECURITY_WEP:
        return AJOBS_AUTH_TYPE_WEP;

    case AJ_WIFI_SECURITY_WPA:
        switch (cipherType) {
        case AJ_WIFI_CIPHER_TKIP:
            return AJOBS_AUTH_TYPE_WPA_TKIP;

        case AJ_WIFI_CIPHER_CCMP:
            return AJOBS_AUTH_TYPE_WPA_CCMP;

        case AJ_WIFI_CIPHER_NONE:
            return AJOBS_AUTH_TYPE_WPA_AUTO;

        default:
            break;
        }
        break;

    case AJ_WIFI_SECURITY_WPA2:
        switch (cipherType) {
        case AJ_WIFI_CIPHER_TKIP:
            return AJOBS_AUTH_TYPE_WPA2_TKIP;

        case AJ_WIFI_CIPHER_CCMP:
            return AJOBS_AUTH_TYPE_WPA2_CCMP;

        case AJ_WIFI_CIPHER_NONE:
            return AJOBS_AUTH_TYPE_WPA2_AUTO;

        default:
            break;
        }
        break;
    }
    return AJOBS_AUTH_TYPE_ANY;
}

static AJ_WiFiSecurityType GetSecType(AJOBS_AuthType authType)
{
    A_UINT32 secType = AJ_WIFI_SECURITY_NONE;

    switch (authType) {
    case AJOBS_AUTH_TYPE_OPEN:
        secType = AJ_WIFI_SECURITY_NONE;
        break;

    case AJOBS_AUTH_TYPE_WEP:
        secType = AJ_WIFI_SECURITY_WEP;
        break;

    case AJOBS_AUTH_TYPE_WPA_TKIP:
    case AJOBS_AUTH_TYPE_WPA_CCMP:
        secType = AJ_WIFI_SECURITY_WPA;
        break;

    case AJOBS_AUTH_TYPE_WPA2_TKIP:
    case AJOBS_AUTH_TYPE_WPA2_CCMP:
    case AJOBS_AUTH_TYPE_WPS:
        secType = AJ_WIFI_SECURITY_WPA2;
        break;

    default:
        break;
    }

    return secType;
}

static AJ_WiFiCipherType GetCipherType(AJOBS_AuthType authType)
{
    A_UINT32 cipherType = AJ_WIFI_CIPHER_NONE;

    switch (authType) {
    case AJOBS_AUTH_TYPE_OPEN:
        cipherType = AJ_WIFI_CIPHER_NONE;
        break;

    case AJOBS_AUTH_TYPE_WEP:
        cipherType = AJ_WIFI_CIPHER_WEP;
        break;

    case AJOBS_AUTH_TYPE_WPA_TKIP:
    case AJOBS_AUTH_TYPE_WPA2_TKIP:
        cipherType = AJ_WIFI_CIPHER_TKIP;
        break;

    case AJOBS_AUTH_TYPE_WPS:
    case AJOBS_AUTH_TYPE_WPA_CCMP:
    case AJOBS_AUTH_TYPE_WPA2_CCMP:
        cipherType = AJ_WIFI_CIPHER_CCMP;
        break;

    default:
        break;
    }

    return cipherType;
}

static void GotScanInfo(const char* ssid, const uint8_t bssid[6], uint8_t rssi, AJOBS_AuthType authType)
{
    if (obScanInfoCount < AJOBS_MAX_SCAN_INFOS) {
        strncpy(obScanInfos[obScanInfoCount].ssid, ssid, AJOBS_SSID_MAX_LENGTH);
        obScanInfos[obScanInfoCount].authType = authType;
        ++obScanInfoCount;
    }
}

static void WiFiScanResult(void* context, const char* ssid, const uint8_t bssid[6], uint8_t rssi, AJ_WiFiSecurityType secType, AJ_WiFiCipherType cipherType)
{
    static const char* const sec[] = { "OPEN", "WEP", "WPA", "WPA2" };
    static const char* const typ[] = { "", ":TKIP", ":CCMP", ":WEP" };
    AJ_InfoPrintf(("WiFiScanResult found ssid=%s rssi=%d security=%s%s\n", ssid, rssi, sec[secType], typ[cipherType]));
    AJOBS_AuthType authType = GetAuthType(secType, cipherType);
    GotScanInfo(ssid, bssid, rssi, authType);
}

AJ_Status AJOBS_ControllerAPI_DoScanInfo()
{
    AJ_Status status = AJ_OK;

    memset(obScanInfos, 0, sizeof(obScanInfos));
    obScanInfoCount = 0;
    // Scan neighbouring networks and save info -> Call OBM_WiFiScanResult().
    status = AJ_WiFiScan(NULL, WiFiScanResult, AJOBS_MAX_SCAN_INFOS);
    if (status == AJ_OK) {
        AJ_InitTimer(&obLastScan);
    }

    return status;
}

AJ_Status AJOBS_ControllerAPI_GotoIdleWiFi(uint8_t reset)
{
    AJ_Status status = AJ_OK;
    AJ_WiFiConnectState wifiConnectState = AJ_GetWifiConnectState();

    if (wifiConnectState != AJ_WIFI_IDLE) {
        status = AJ_DisconnectWiFi();
        if (reset) {
            status = AJ_ResetWiFi();
        }
        wifiConnectState = AJ_GetWifiConnectState();
    }
    AJ_Sleep(1000);

    while ((wifiConnectState != AJ_WIFI_IDLE) && (wifiConnectState != AJ_WIFI_CONNECT_FAILED) && (wifiConnectState != AJ_WIFI_DISCONNECTING)) {
        wifiConnectState = AJ_GetWifiConnectState();
        AJ_InfoPrintf(("WIFI_CONNECT_STATE: %s\n", AJ_WiFiConnectStateText(wifiConnectState)));
        AJ_Sleep(500);
    }

    return status;
}

static AJ_Status DoConnectWifi(AJOBS_Info* connectInfo)
{
    AJ_Status status = AJ_OK;
    AJ_WiFiSecurityType secType = AJ_WIFI_SECURITY_NONE;
    AJ_WiFiCipherType cipherType = AJ_WIFI_CIPHER_NONE;
    AJOBS_AuthType fallback = AJOBS_AUTH_TYPE_OPEN;
    AJOBS_AuthType fallbackUntil = AJOBS_AUTH_TYPE_OPEN;
    uint8_t retries = 0;
    AJ_WiFiConnectState wifiConnectState;
    uint8_t raw[(AJOBS_PASSCODE_MAX_LENGTH / 2) + 1];
    char* password = connectInfo->pc;

    AJ_InfoPrintf(("Attempting to connect to %s with passcode=%s and auth=%d\n", connectInfo->ssid, connectInfo->pc, connectInfo->authType));

    switch (connectInfo->authType) {
    case AJOBS_AUTH_TYPE_ANY:
        if (strlen(connectInfo->pc) == 0) {
            break;
        }
        fallback = AJOBS_AUTH_TYPE_WPA2_CCMP;
        fallbackUntil = AJOBS_AUTH_TYPE_OPEN;
        break;

    case AJOBS_AUTH_TYPE_WPA_AUTO:
        if (strlen(connectInfo->pc) == 0) {
            break;
        }
        fallback = AJOBS_AUTH_TYPE_WPA_CCMP;
        fallbackUntil = AJOBS_AUTH_TYPE_WPA_TKIP;
        break;

    case AJOBS_AUTH_TYPE_WPA2_AUTO:
        if (strlen(connectInfo->pc) == 0) {
            break;
        }
        fallback = AJOBS_AUTH_TYPE_WPA2_CCMP;
        fallbackUntil = AJOBS_AUTH_TYPE_WPA2_TKIP;
        break;

    default:
        fallback = connectInfo->authType;
    }

    secType = GetSecType(fallback);
    cipherType = GetCipherType(fallback);
    AJ_InfoPrintf(("Trying to connect with auth=%d (secType=%d, cipherType=%d)\n", fallback, secType, cipherType));

    /* Only decode the password from HEX if authentication type is NOT WEP */
    if (AJOBS_AUTH_TYPE_WEP != connectInfo->authType) {
        size_t hexLen = strlen(connectInfo->pc);
        AJ_HexToRaw(connectInfo->pc, hexLen, raw, (AJOBS_PASSCODE_MAX_LENGTH / 2) + 1);
        password = (char*)raw;
        password[hexLen / 2] = '\0';
    }

    while (1) {
        if (connectInfo->state == AJOBS_STATE_CONFIGURED_NOT_VALIDATED) {
            connectInfo->state = AJOBS_STATE_CONFIGURED_VALIDATING;
        }

        status = AJ_ConnectWiFi(connectInfo->ssid, secType, cipherType, password);
        AJ_InfoPrintf(("AJ_ConnectWifi returned %s\n", AJ_StatusText(status)));

        wifiConnectState = AJ_GetWifiConnectState();

        // Set last error and state
        if ((status == AJ_OK) /* (wifiConnectState == AJ_WIFI_CONNECT_OK)*/) {
            obLastError.code = AJOBS_STATE_LAST_ERROR_VALIDATED;
            obLastError.message[0] = '\0';
            obState = AJOBS_STATE_CONFIGURED_VALIDATED;
            connectInfo->authType = fallback;
        } else if ((status == AJ_ERR_CONNECT) /* (wifiConnectState == AJ_WIFI_CONNECT_FAILED)*/) {
            obLastError.code = AJOBS_STATE_LAST_ERROR_UNREACHABLE;
            strncpy(obLastError.message, "Network unreachable!", AJOBS_ERROR_MESSAGE_LEN);
            if (connectInfo->state == AJOBS_STATE_CONFIGURED_VALIDATED || connectInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                obState = AJOBS_STATE_CONFIGURED_RETRY;
            } else {
                obState = AJOBS_STATE_CONFIGURED_ERROR;
            }
        } else if ((status == AJ_ERR_SECURITY) /* (wifiConnectState == AJ_WIFI_AUTH_FAILED)*/) {
            obLastError.code = AJOBS_STATE_LAST_ERROR_UNAUTHORIZED;
            strncpy(obLastError.message, "Authorization failed!", AJOBS_ERROR_MESSAGE_LEN);
            if (connectInfo->state == AJOBS_STATE_CONFIGURED_VALIDATED || connectInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                obState = AJOBS_STATE_CONFIGURED_RETRY;
            } else {
                obState = AJOBS_STATE_CONFIGURED_ERROR;
            }
        } else if (status == AJ_ERR_DRIVER) {
            obLastError.code = AJOBS_STATE_LAST_ERROR_ERROR_MESSAGE;
            strncpy(obLastError.message, "Driver error", AJOBS_ERROR_MESSAGE_LEN);
            obState = AJOBS_STATE_CONFIGURED_ERROR;
        } else if (status == AJ_ERR_DHCP) {
            obLastError.code = AJOBS_STATE_LAST_ERROR_ERROR_MESSAGE;
            strncpy(obLastError.message, "Failed to establish IP!", AJOBS_ERROR_MESSAGE_LEN);
            if (connectInfo->state == AJOBS_STATE_CONFIGURED_VALIDATED || connectInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                obState = AJOBS_STATE_CONFIGURED_RETRY;
            } else {
                obState = AJOBS_STATE_CONFIGURED_ERROR;
            }
        } else {
            obLastError.code = AJOBS_STATE_LAST_ERROR_ERROR_MESSAGE;
            strncpy(obLastError.message, "Failed to connect! Unexpected error", AJOBS_ERROR_MESSAGE_LEN);
            obState = AJOBS_STATE_CONFIGURED_ERROR;
        }

        if (obState == AJOBS_STATE_CONFIGURED_VALIDATED) {
            break;
        }
        AJ_WarnPrintf(("Warning - DoConnectWifi wifiConnectState = %s\n", AJ_WiFiConnectStateText(wifiConnectState)));
        AJ_WarnPrintf(("Last error set to \"%s\" (code=%d)\n", obLastError.message, obLastError.code));

        if (obState == AJOBS_STATE_CONFIGURED_ERROR || obState == AJOBS_STATE_CONFIGURED_RETRY) {
            if (retries++ >= obSettings->AJOBS_MAX_RETRIES) {
                if (obState == AJOBS_STATE_CONFIGURED_ERROR && connectInfo->state == AJOBS_STATE_CONFIGURED_VALIDATING) {
                    if (connectInfo->authType < 0 && fallback > fallbackUntil) {
                        obLastError.code = AJOBS_STATE_LAST_ERROR_UNSUPPORTED_PROTOCOL;
                        strncpy(obLastError.message, "Unsupported protocol", AJOBS_ERROR_MESSAGE_LEN);
                        fallback--; // Try next authentication protocol
                        secType = GetSecType(fallback);
                        cipherType = GetCipherType(fallback);
                        retries = 0;
                        AJ_InfoPrintf(("Trying to connect with fallback auth=%d (secType=%d, cipherType=%d)\n", fallback, secType, cipherType));
                        continue;
                    }
                }
                break;
            }
            AJ_InfoPrintf(("Retry number %d out of %d\n", retries, obSettings->AJOBS_MAX_RETRIES));
        }
    }

    connectInfo->state = obState;
    status = AJOBS_SetInfo(connectInfo);
    return status;
}

AJ_Status AJOBS_ControllerAPI_StartSoftAPIfNeededOrConnect(AJOBS_Info* obInfo)
{
    AJ_Status status = AJ_OK;

    // Check if just started and force immediate validation for previous retry
    if (bFirstStart) {
        bFirstStart = FALSE;
        if (obInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
            obInfo->state = AJOBS_STATE_CONFIGURED_VALIDATED;
        }
    }
    // Check if there is Wi-Fi connectivity and return if established already
    if (AJOBS_IsWiFiConnected()) {
        AJ_InfoPrintf(("Already connected with ConnectionState=%u, no need to establish connection!\n", (uint8_t)AJ_GetWifiConnectState()));
        return status;
    }
    while (1) {
        status = AJOBS_ControllerAPI_GotoIdleWiFi(obSettings->AJOBS_RESET_WIFI_ON_IDLE); // Go into IDLE mode, reset Wi-Fi
        if (status != AJ_OK) {
            break;
        }
        status = AJOBS_ControllerAPI_DoScanInfo(); // Perform scan of available Wi-Fi networks
        if (status != AJ_OK) {
            break;
        }
        // Check if require to switch into SoftAP mode.
        if ((obInfo->state == AJOBS_STATE_NOT_CONFIGURED || obInfo->state == AJOBS_STATE_CONFIGURED_ERROR || obInfo->state == AJOBS_STATE_CONFIGURED_RETRY)) {
            AJ_InfoPrintf(("Establishing SoftAP with ssid=%s%s auth=%s\n", obSettings->AJOBS_SoftAPSSID, (obSettings->AJOBS_SoftAPIsHidden ? " (hidden)" : ""), obSettings->AJOBS_SoftAPPassphrase == NULL ? "OPEN" : obSettings->AJOBS_SoftAPPassphrase));
            if (obInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                AJ_InfoPrintf(("Retry timer activated\n"));
                status = AJ_EnableSoftAP(obSettings->AJOBS_SoftAPSSID, obSettings->AJOBS_SoftAPIsHidden, obSettings->AJOBS_SoftAPPassphrase, obSettings->AJOBS_WAIT_BETWEEN_RETRIES);
            } else {
                status = AJ_EnableSoftAP(obSettings->AJOBS_SoftAPSSID, obSettings->AJOBS_SoftAPIsHidden, obSettings->AJOBS_SoftAPPassphrase, obSettings->AJOBS_WAIT_FOR_SOFTAP_CONNECTION);
            }
            if (status != AJ_OK) {
                if (AJ_ERR_TIMEOUT == status) {
                    //check for timer elapsed for retry
                    if (obInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                        AJ_InfoPrintf(("Retry timer elapsed at %ums\n", obSettings->AJOBS_WAIT_BETWEEN_RETRIES));
                        obInfo->state = AJOBS_STATE_CONFIGURED_VALIDATED;
                        status = AJOBS_SetInfo(obInfo);
                        if (status == AJ_OK) {
                            continue; // Loop back and connect in client mode
                        }
                    }
                }
                AJ_WarnPrintf(("Failed to establish SoftAP with status=%s\n", AJ_StatusText(status)));
            }
        } else { // Otherwise connect to given configuration and according to error code returned map to relevant onboarding state and set value for LastError and ConnectionResult.
            status = DoConnectWifi(obInfo);
            if (status == AJ_OK) {
                if (obInfo->state == AJOBS_STATE_CONFIGURED_ERROR || obInfo->state == AJOBS_STATE_CONFIGURED_RETRY) {
                    continue; // Loop back and establish SoftAP mode
                }
            } else {
                AJ_WarnPrintf(("Failed to establish connection with current configuration status=%s\n", AJ_StatusText(status)));
            }
        }
        break; // Either connected to (as client) or connected from (as SoftAP) a station
    }
    return status;
}

AJ_Status AJOBS_ClearInfo()
{
    AJOBS_Info emptyInfo;
    memset(&emptyInfo, 0, sizeof(emptyInfo));

    // Clear state
    obState = emptyInfo.state = AJOBS_STATE_NOT_CONFIGURED;

    // Clear last error
    obLastError.code = AJOBS_STATE_LAST_ERROR_VALIDATED;
    obLastError.message[0] = '\0';

    return AJOBS_SetInfo(&emptyInfo);
}

AJ_Status AJOBS_ControllerAPI_DoOffboardWiFi()
{
    return (AJOBS_ClearInfo());
}

AJ_Status AJOBS_EstablishWiFi()
{
    AJ_Status status;
    AJOBS_Info obInfo;

    status = AJOBS_GetInfo(&obInfo);
    if (status != AJ_OK) {
        goto ErrorExit;
    }

    status = AJOBS_ControllerAPI_StartSoftAPIfNeededOrConnect(&obInfo);
    if (status != AJ_OK) {
        goto ErrorExit;
    }

    return status;

ErrorExit:

    AJ_ErrPrintf(("EstablishWiFi status: %s\n", AJ_StatusText(status)));
    return status;
}

AJ_Status AJOBS_SwitchToRetry()
{
    AJ_Status status = AJ_OK;
    AJOBS_Info obInfo;

    status = AJOBS_GetInfo(&obInfo);
    if (status != AJ_OK) {
        goto ErrorExit;
    }
    obInfo.state = AJOBS_STATE_CONFIGURED_RETRY;
    status = AJOBS_SetInfo(&obInfo);
    if (status != AJ_OK) {
        goto ErrorExit;
    }
    status = AJOBS_DisconnectWiFi();
    if (status != AJ_OK) {
        goto ErrorExit;
    }

    AJ_InfoPrintf(("SwitchToRetry status: %s\n", AJ_StatusText(status)));
    return status;

ErrorExit:

    AJ_ErrPrintf(("SwitchToRetry status: %s\n", AJ_StatusText(status)));
    return status;
}

AJ_Status AJOBS_DisconnectWiFi()
{
    AJ_Status status = AJ_OK;

    status = AJOBS_ControllerAPI_GotoIdleWiFi(obSettings->AJOBS_RESET_WIFI_ON_IDLE);

    return status;
}

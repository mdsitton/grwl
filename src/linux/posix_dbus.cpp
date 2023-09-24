//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#include <cstring>
#include <climits>
#include <unistd.h>
#include <cstdio>
#include <cctype>

void _grwlInitDBusPOSIX()
{
    // Initialize DBus library functions
    _grwl.dbus.handle = NULL;
    _grwl.dbus.connection = NULL;

    _grwl.dbus.handle = _grwlPlatformLoadModule("libdbus-1.so.3");
    if (!_grwl.dbus.handle)
    {
        return;
    }

    _grwl.dbus.error_init = (PFN_dbus_error_init)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_error_init");
    _grwl.dbus.error_is_set =
        (PFN_dbus_error_is_set)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_error_is_set");
    _grwl.dbus.error_free = (PFN_dbus_error_free)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_error_free");
    _grwl.dbus.connection_unref =
        (PFN_dbus_connection_unref)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_connection_unref");
    _grwl.dbus.connection_send =
        (PFN_dbus_connection_send)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_connection_send");
    _grwl.dbus.connection_flush =
        (PFN_dbus_connection_flush)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_connection_flush");
    _grwl.dbus.bus_request_name =
        (PFN_dbus_bus_request_name)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_bus_request_name");
    _grwl.dbus.bus_get = (PFN_dbus_bus_get)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_bus_get");
    _grwl.dbus.message_unref =
        (PFN_dbus_message_unref)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_message_unref");
    _grwl.dbus.message_new_signal =
        (PFN_dbus_message_new_signal)_grwlPlatformGetModuleSymbol(_grwl.dbus.handle, "dbus_message_new_signal");
    _grwl.dbus.message_iter_init_append = (PFN_dbus_message_iter_init_append)_grwlPlatformGetModuleSymbol(
        _grwl.dbus.handle, "dbus_message_iter_init_append");
    _grwl.dbus.message_iter_append_basic = (PFN_dbus_message_iter_append_basic)_grwlPlatformGetModuleSymbol(
        _grwl.dbus.handle, "dbus_message_iter_append_basic");
    _grwl.dbus.message_iter_open_container = (PFN_dbus_message_iter_open_container)_grwlPlatformGetModuleSymbol(
        _grwl.dbus.handle, "dbus_message_iter_open_container");
    _grwl.dbus.message_iter_close_container = (PFN_dbus_message_iter_close_container)_grwlPlatformGetModuleSymbol(
        _grwl.dbus.handle, "dbus_message_iter_close_container");

    if (!_grwl.dbus.error_init || !_grwl.dbus.error_is_set || !_grwl.dbus.error_free || !_grwl.dbus.connection_unref ||
        !_grwl.dbus.connection_send || !_grwl.dbus.connection_flush || !_grwl.dbus.bus_request_name ||
        !_grwl.dbus.bus_get || !_grwl.dbus.message_unref || !_grwl.dbus.message_new_signal ||
        !_grwl.dbus.message_iter_init_append || !_grwl.dbus.message_iter_append_basic ||
        !_grwl.dbus.message_iter_open_container || !_grwl.dbus.message_iter_close_container)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "POSIX: Failed to load DBus entry points");
        return;
    }

    // Initialize DBus connection
    dbus_error_init(&_grwl.dbus.error);
    _grwl.dbus.connection = dbus_bus_get(DBUS_BUS_SESSION, &_grwl.dbus.error);

    // Check for errors
    if (dbus_error_is_set(&_grwl.dbus.error) || !_grwl.dbus.connection)
    {
        if (dbus_error_is_set(&_grwl.dbus.error))
        {
            dbus_error_free(&_grwl.dbus.error);
        }

        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to connect to DBus");

        dbus_connection_unref(_grwl.dbus.connection);
        _grwl.dbus.connection = NULL;
        return;
    }
    else
    {
        // Request name

        _grwlCacheLegalExecutableNameDBusPOSIX();
        if (!_grwl.dbus.legalExecutableName)
        {
            return;
        }

        //"org.grwl.<exe_name>_<pid>"
        char* busName = (char*)_grwl_calloc(21 + strlen(_grwl.dbus.legalExecutableName), sizeof(char));
        if (!busName)
        {
            _grwlInputError(GRWL_OUT_OF_MEMORY, "Failed to allocate memory for bus name");
            return;
        }
        memset(busName, '\0', (21 + strlen(_grwl.dbus.legalExecutableName)) * sizeof(char));

        const pid_t pid = getpid();
        sprintf(busName, "org.grwl.%s_%d", _grwl.dbus.legalExecutableName, pid);

        const int res =
            dbus_bus_request_name(_grwl.dbus.connection, busName, DBUS_NAME_FLAG_REPLACE_EXISTING, &_grwl.dbus.error);

        _grwl_free(busName);

        // Check for errors
        if (dbus_error_is_set(&_grwl.dbus.error) || res != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        {
            if (dbus_error_is_set(&_grwl.dbus.error))
            {
                dbus_error_free(&_grwl.dbus.error);
            }

            _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to request DBus name");

            dbus_connection_unref(_grwl.dbus.connection);
            _grwl.dbus.connection = NULL;
        }
    }

    _grwlCacheFullExecutableNameDBusPOSIX();
    _grwlCacheDesktopFilePathDBusPOSIX();
    _grwlCacheSignalNameDBusPOSIX();
}

void _grwlCacheSignalNameDBusPOSIX()
{
    if (!_grwl.dbus.legalExecutableName)
    {
        return;
    }

    //"/org/grwl/<exe_name>_<pid>"
    char* signalName = (char*)_grwl_calloc(22 + strlen(_grwl.dbus.legalExecutableName), sizeof(char));
    if (!signalName)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, "Failed to allocate memory for signal name");
        return;
    }

    memset(signalName, '\0', (22 + strlen(_grwl.dbus.legalExecutableName)) * sizeof(char));

    const pid_t pid = getpid();
    if (sprintf(signalName, "/org/grwl/%s_%d", _grwl.dbus.legalExecutableName, pid) < 0)
    {
        _grwlInputError(GRWL_PLATFORM, "Failed to create signal name");
        _grwl_free(signalName);
        return;
    }

    _grwl.dbus.signalName = signalName;
}

void _grwlCacheFullExecutableNameDBusPOSIX()
{
    char exeName[PATH_MAX];
    memset(exeName, 0, sizeof(char) * PATH_MAX);
    if (readlink("/proc/self/exe", exeName, PATH_MAX) == -1)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    char* exeNameEnd = strchr(exeName, '\0');
    char* lastFound = strrchr(exeName, '/');
    if (!lastFound || !exeNameEnd)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    unsigned int exeNameLength = (exeNameEnd - lastFound) - 1;

    char* exeNameFinal = (char*)_grwl_calloc(exeNameLength + 1, sizeof(char));
    if (!exeNameFinal)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, "Failed to allocate memory for executable name");
        return;
    }

    memset(exeNameFinal, 0, sizeof(char) * (exeNameLength + 1));

    memcpy(exeNameFinal, (lastFound + 1), exeNameLength);

    _grwl.dbus.fullExecutableName = exeNameFinal;
}

void _grwlCacheLegalExecutableNameDBusPOSIX()
{
    // The executable name is stripped of any illegal characters
    // according to the DBus specification

    int i = 0;
    int validExeNameLength = 0;
    int output = 0;
    char exeName[PATH_MAX];
    memset(exeName, 0, sizeof(char) * PATH_MAX);
    if (readlink("/proc/self/exe", exeName, PATH_MAX) == -1)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    char* exeNameEnd = strchr(exeName, '\0');
    char* lastFound = strrchr(exeName, '/');
    if (!lastFound || !exeNameEnd)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    unsigned int exeNameLength = (exeNameEnd - lastFound) - 1;

    for (i = 0; i < exeNameLength; ++i)
    {
        if (isalnum(*(lastFound + 1 + i)))
        {
            validExeNameLength++;
        }
    }

    char* exeNameFinal = (char*)_grwl_calloc(validExeNameLength + 1, sizeof(char));
    if (!exeNameFinal)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, "Failed to allocate memory for executable name");
        return;
    }

    memset(exeNameFinal, 0, sizeof(char) * (validExeNameLength + 1));

    for (i = 0; i < exeNameLength; ++i)
    {
        if (isalnum(*(lastFound + 1 + i)))
        {
            exeNameFinal[output++] = *(lastFound + 1 + i);
        }
    }

    _grwl.dbus.legalExecutableName = exeNameFinal;
}

void _grwlCacheDesktopFilePathDBusPOSIX()
{
    if (!_grwl.dbus.fullExecutableName)
    {
        return;
    }

    // Cache path of .desktop file

    // Create our final desktop file uri
    //"application://<exe_name>.desktop"
    unsigned int desktopFileLength =
        strlen("application://") + strlen(_grwl.dbus.fullExecutableName) + strlen(".desktop") + 1;
    _grwl.dbus.desktopFilePath = (char*)_grwl_calloc(desktopFileLength, sizeof(char));
    if (!_grwl.dbus.desktopFilePath)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, "Failed to allocate memory for .desktop file path");
        return;
    }

    memset(_grwl.dbus.desktopFilePath, 0, sizeof(char) * desktopFileLength);
    strcpy(_grwl.dbus.desktopFilePath, "application://");
    memcpy(_grwl.dbus.desktopFilePath + strlen("application://"), _grwl.dbus.fullExecutableName,
           strlen(_grwl.dbus.fullExecutableName));
    strcpy(_grwl.dbus.desktopFilePath + strlen("application://") + strlen(_grwl.dbus.fullExecutableName), ".desktop");
    _grwl.dbus.desktopFilePath[desktopFileLength - 1] = '\0';
}

void _grwlTerminateDBusPOSIX()
{
    if (_grwl.dbus.signalName)
    {
        _grwl_free(_grwl.dbus.signalName);
    }

    if (_grwl.dbus.legalExecutableName)
    {
        _grwl_free(_grwl.dbus.legalExecutableName);
    }

    if (_grwl.dbus.fullExecutableName)
    {
        _grwl_free(_grwl.dbus.fullExecutableName);
    }

    if (_grwl.dbus.desktopFilePath)
    {
        _grwl_free(_grwl.dbus.desktopFilePath);
    }

    if (_grwl.dbus.connection)
    {
        dbus_connection_unref(_grwl.dbus.connection);
        _grwl.dbus.connection = NULL;
    }

    if (_grwl.dbus.handle)
    {
        _grwlPlatformFreeModule(_grwl.dbus.handle);
        _grwl.dbus.handle = NULL;
    }
}

void _grwlUpdateTaskbarProgressDBusPOSIX(dbus_bool_t progressVisible, double progressValue)
{
    struct DBusMessage* msg = NULL;

    if (!_grwl.dbus.handle || !_grwl.dbus.connection || !_grwl.dbus.desktopFilePath || !_grwl.dbus.signalName)
    {
        return;
    }

    // Signal signature:
    // signal com.canonical.Unity.LauncherEntry.Update (in s app_uri, in a{sv} properties)

    struct DBusMessageIter args;
    memset(&args, 0, sizeof(args));

    if (!_grwlNewMessageSignalDBusPOSIX(_grwl.dbus.signalName, "com.canonical.Unity.LauncherEntry", "Update", &msg))
    {
        return;
    }

    dbus_message_iter_init_append(msg, &args);

    // Setup app_uri parameter
    _grwlAppendDataDBusPOSIX(&args, DBUS_TYPE_STRING, &_grwl.dbus.desktopFilePath);

    // Set properties parameter
    struct DBusMessageIter sub1;
    memset(&sub1, 0, sizeof(sub1));

    _grwlOpenContainerDBusPOSIX(&args, DBUS_TYPE_ARRAY, "{sv}", &sub1);

    // Set progress visible property
    const char* progressVisibleStr = "progress-visible";
    _grwlAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &progressVisibleStr, DBUS_TYPE_BOOLEAN, &progressVisible);

    // Set progress value property
    const char* progressStr = "progress";
    _grwlAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &progressStr, DBUS_TYPE_DOUBLE, &progressValue);

    _grwlCloseContainerDBusPOSIX(&args, &sub1);

    _grwlSendMessageDBusPOSIX(msg);

    // Free the message
    dbus_message_unref(msg);
}

void _grwlUpdateBadgeDBusPOSIX(dbus_bool_t badgeVisible, int badgeCount)
{
    struct DBusMessage* msg = NULL;

    if (!_grwl.dbus.handle || !_grwl.dbus.connection || !_grwl.dbus.desktopFilePath || !_grwl.dbus.signalName)
    {
        return;
    }

    long long badgeCountLL = badgeCount;

    // Signal signature:
    // signal com.canonical.Unity.LauncherEntry.Update (in s app_uri, in a{sv} properties)

    struct DBusMessageIter args;
    memset(&args, 0, sizeof(args));

    if (!_grwlNewMessageSignalDBusPOSIX(_grwl.dbus.signalName, "com.canonical.Unity.LauncherEntry", "Update", &msg))
    {
        return;
    }

    dbus_message_iter_init_append(msg, &args);

    // Setup app_uri parameter
    _grwlAppendDataDBusPOSIX(&args, DBUS_TYPE_STRING, &_grwl.dbus.desktopFilePath);

    // Set properties parameter
    struct DBusMessageIter sub1;
    memset(&sub1, 0, sizeof(sub1));

    _grwlOpenContainerDBusPOSIX(&args, DBUS_TYPE_ARRAY, "{sv}", &sub1);

    // Set count visible property
    const char* countVisibleStr = "count-visible";
    _grwlAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &countVisibleStr, DBUS_TYPE_BOOLEAN, &badgeVisible);

    // Set count value property
    const char* countValueStr = "count";
    _grwlAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &countValueStr, DBUS_TYPE_INT64, &badgeCountLL);

    _grwlCloseContainerDBusPOSIX(&args, &sub1);

    _grwlSendMessageDBusPOSIX(msg);

    // Free the message
    dbus_message_unref(msg);
}

dbus_bool_t _grwlNewMessageSignalDBusPOSIX(const char* objectPath, const char* interfaceName, const char* signalName,
                                           struct DBusMessage** outMessage)
{
    if (!outMessage)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to create new DBus message, output message pointer is NULL");
        return false;
    }

    *outMessage = dbus_message_new_signal(objectPath, interfaceName, signalName);
    if (!(*outMessage))
    {
        *outMessage = NULL;
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to create new DBus message");
        return false;
    }

    return true;
}

dbus_bool_t _grwlOpenContainerDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const char* signature,
                                        struct DBusMessageIter* subIterator)
{
    if (DBusType != DBUS_TYPE_ARRAY && DBusType != DBUS_TYPE_STRUCT_OPEN && DBusType != DBUS_TYPE_STRUCT_CLOSE &&
        DBusType != DBUS_TYPE_VARIANT && DBusType != DBUS_TYPE_DICT_ENTRY)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Invalid DBUS container type provided");
        return false;
    }
    if (!iterator || !subIterator)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "DBus message iterator is NULL");
        return false;
    }

    return dbus_message_iter_open_container(iterator, DBusType, signature, subIterator);
}

dbus_bool_t _grwlCloseContainerDBusPOSIX(struct DBusMessageIter* iterator, struct DBusMessageIter* subIterator)
{
    if (!iterator || !subIterator)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "DBus message iterator is NULL");
        return false;
    }

    return dbus_message_iter_close_container(iterator, subIterator);
}

dbus_bool_t _grwlAppendDataDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const void* data)
{
    if (DBusType == DBUS_TYPE_ARRAY || DBusType == DBUS_TYPE_VARIANT || DBusType == DBUS_TYPE_DICT_ENTRY ||
        DBusType == DBUS_TYPE_STRUCT_OPEN || DBusType == DBUS_TYPE_STRUCT_CLOSE)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Invalid DBus type provided");
        return false;
    }
    if (!iterator)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "DBus message iterator is NULL");
        return false;
    }
    if (!data)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "DBus data to append is NULL");
        return false;
    }

    return dbus_message_iter_append_basic(iterator, DBusType, data);
}

dbus_bool_t _grwlAppendDictDataDBusPOSIX(struct DBusMessageIter* iterator, int keyType, const void* keyData,
                                         int valueType, const void* valueData)
{
    struct DBusMessageIter keyIterator;
    struct DBusMessageIter valueIterator;
    memset(&keyIterator, 0, sizeof(keyIterator));
    memset(&valueIterator, 0, sizeof(valueIterator));

    if (!_grwlOpenContainerDBusPOSIX(iterator, DBUS_TYPE_DICT_ENTRY, NULL, &keyIterator))
    {
        return false;
    }

    // Append key data
    if (!_grwlAppendDataDBusPOSIX(&keyIterator, keyType, keyData))
    {
        return false;
    }

    if (!_grwlOpenContainerDBusPOSIX(&keyIterator, DBUS_TYPE_VARIANT, (const char*)&valueType, &valueIterator))
    {
        return false;
    }

    // Append value data
    if (!_grwlAppendDataDBusPOSIX(&valueIterator, valueType, valueData))
    {
        return false;
    }

    if (!_grwlCloseContainerDBusPOSIX(&keyIterator, &valueIterator))
    {
        return false;
    }

    if (!_grwlCloseContainerDBusPOSIX(iterator, &keyIterator))
    {
        return false;
    }

    return true;
}

dbus_bool_t _grwlSendMessageDBusPOSIX(struct DBusMessage* message)
{
    if (!message)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "DBus message is NULL");
        return false;
    }

    unsigned int serial = 0;
    if (!dbus_connection_send(_grwl.dbus.connection, message, &serial))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Failed to send DBus message");
        return false;
    }

    dbus_connection_flush(_grwl.dbus.connection);

    return true;
}

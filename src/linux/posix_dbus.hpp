//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

// Taken from DBus docs (https://dbus.freedesktop.org/doc/api/html/index.html)
typedef struct DBusConnection DBusConnection;
typedef struct DBusMessage DBusMessage;
typedef unsigned int dbus_bool_t;
typedef unsigned int dbus_uint32_t;

enum DBusBusType
{
    DBUS_BUS_SESSION,
    DBUS_BUS_SYSTEM,
    DBUS_BUS_STARTER
};

struct DBusError
{
    const char* name;
    const char* message;
    unsigned int dummy1 : 1;
    unsigned int dummy2 : 1;
    unsigned int dummy3 : 1;
    unsigned int dummy4 : 1;
    unsigned int dummy5 : 1;
    void* padding1;
};

struct DBusMessageIter
{
    void* dummy1;
    void* dummy2;
    dbus_uint32_t dummy3;
    int dummy4, dummy5, dummy6, dummy7, dummy8, dummy9, dummy10, dummy11;
    int pad1;
    void* pad2;
    void* pad3;
};

#define DBUS_NAME_FLAG_REPLACE_EXISTING 0x2
#define DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER 1
#define DBUS_TYPE_STRING (unsigned int)'s'
#define DBUS_TYPE_ARRAY (unsigned int)'a'
#define DBUS_TYPE_DICT_ENTRY (unsigned int)'e'
#define DBUS_TYPE_VARIANT (unsigned int)'v'
#define DBUS_TYPE_BOOLEAN (unsigned int)'b'
#define DBUS_TYPE_DOUBLE (unsigned int)'d'
#define DBUS_TYPE_INT16 (unsigned int)'n'
#define DBUS_TYPE_UINT16 (unsigned int)'q'
#define DBUS_TYPE_INT32 (unsigned int)'i'
#define DBUS_TYPE_UINT32 (unsigned int)'u'
#define DBUS_TYPE_INT64 (unsigned int)'x'
#define DBUS_TYPE_UINT64 (unsigned int)'t'
#define DBUS_TYPE_STRUCT_OPEN (unsigned int)'('
#define DBUS_TYPE_STRUCT_CLOSE (unsigned int)')'
#define DBUS_TYPE_BYTE (unsigned int)'y'
#define DBUS_TYPE_OBJECT_PATH (unsigned int)'o'
#define DBUS_TYPE_SIGNATURE (unsigned int)'g'

typedef void (*PFN_dbus_error_init)(struct DBusError*);
typedef dbus_bool_t (*PFN_dbus_error_is_set)(const struct DBusError*);
typedef void (*PFN_dbus_error_free)(struct DBusError*);
typedef void (*PFN_dbus_connection_unref)(DBusConnection*);
typedef dbus_bool_t (*PFN_dbus_connection_send)(DBusConnection*, DBusMessage*, dbus_uint32_t*);
typedef void (*PFN_dbus_connection_flush)(DBusConnection*);
typedef int (*PFN_dbus_bus_request_name)(DBusConnection*, const char*, unsigned int, struct DBusError*);
typedef DBusConnection* (*PFN_dbus_bus_get)(enum DBusBusType, struct DBusError*);
typedef void (*PFN_dbus_message_unref)(DBusMessage*);
typedef DBusMessage* (*PFN_dbus_message_new_signal)(const char*, const char*, const char*);
typedef void (*PFN_dbus_message_iter_init_append)(DBusMessage*, struct DBusMessageIter*);
typedef dbus_bool_t (*PFN_dbus_message_iter_append_basic)(struct DBusMessageIter*, int, const void*);
typedef dbus_bool_t (*PFN_dbus_message_iter_open_container)(struct DBusMessageIter*, int, const char*,
                                                            struct DBusMessageIter*);
typedef dbus_bool_t (*PFN_dbus_message_iter_close_container)(struct DBusMessageIter*, struct DBusMessageIter*);

#define dbus_error_init _grwl.dbus.error_init
#define dbus_error_is_set _grwl.dbus.error_is_set
#define dbus_error_free _grwl.dbus.error_free
#define dbus_connection_unref _grwl.dbus.connection_unref
#define dbus_connection_send _grwl.dbus.connection_send
#define dbus_connection_flush _grwl.dbus.connection_flush
#define dbus_bus_request_name _grwl.dbus.bus_request_name
#define dbus_bus_get _grwl.dbus.bus_get
#define dbus_message_unref _grwl.dbus.message_unref
#define dbus_message_new_signal _grwl.dbus.message_new_signal
#define dbus_message_iter_init_append _grwl.dbus.message_iter_init_append
#define dbus_message_iter_append_basic _grwl.dbus.message_iter_append_basic
#define dbus_message_iter_open_container _grwl.dbus.message_iter_open_container
#define dbus_message_iter_close_container _grwl.dbus.message_iter_close_container

#define GRWL_POSIX_LIBRARY_DBUS_STATE _GRWLDBusPOSIX dbus;

// POSIX-specific dbus data
//
typedef struct _GRWLDBusPOSIX
{
    void* handle;

    PFN_dbus_error_init error_init;
    PFN_dbus_error_is_set error_is_set;
    PFN_dbus_error_free error_free;
    PFN_dbus_connection_unref connection_unref;
    PFN_dbus_connection_send connection_send;
    PFN_dbus_connection_flush connection_flush;
    PFN_dbus_bus_request_name bus_request_name;
    PFN_dbus_bus_get bus_get;
    PFN_dbus_message_unref message_unref;
    PFN_dbus_message_new_signal message_new_signal;
    PFN_dbus_message_iter_init_append message_iter_init_append;
    PFN_dbus_message_iter_append_basic message_iter_append_basic;
    PFN_dbus_message_iter_open_container message_iter_open_container;
    PFN_dbus_message_iter_close_container message_iter_close_container;

    DBusConnection* connection;
    struct DBusError error;

    char* desktopFilePath;
    char* fullExecutableName;
    char* legalExecutableName;
    char* signalName;
} _GRWLDBusPOSIX;

void _grwlInitDBusPOSIX(void);
void _grwlCacheSignalNameDBusPOSIX(void);
void _grwlCacheFullExecutableNameDBusPOSIX(void);
void _grwlCacheLegalExecutableNameDBusPOSIX(void);
void _grwlCacheDesktopFilePathDBusPOSIX(void);
void _grwlTerminateDBusPOSIX(void);
void _grwlUpdateTaskbarProgressDBusPOSIX(dbus_bool_t progressVisible, double progressValue);
void _grwlUpdateBadgeDBusPOSIX(dbus_bool_t badgeVisible, int badgeCount);

dbus_bool_t _grwlNewMessageSignalDBusPOSIX(const char* objectPath, const char* interfaceName, const char* signalName,
                                           struct DBusMessage** outMessage);
dbus_bool_t _grwlOpenContainerDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const char* signature,
                                        struct DBusMessageIter* subIterator);
dbus_bool_t _grwlCloseContainerDBusPOSIX(struct DBusMessageIter* iterator, struct DBusMessageIter* subIterator);
dbus_bool_t _grwlAppendDataDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const void* data);
dbus_bool_t _grwlAppendDictDataDBusPOSIX(struct DBusMessageIter* iterator, int keyType, const void* keyData,
                                         int valueType, const void* valueData);
dbus_bool_t _grwlSendMessageDBusPOSIX(struct DBusMessage* message);

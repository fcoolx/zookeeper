/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ZOOKEEPER_H_
#define ZOOKEEPER_H_

#include <stdlib.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/time.h>
#else
#include "winconfig.h"
#endif
#include <stdio.h>
#include <ctype.h>

#include <boost/ptr_container/ptr_vector.hpp>
#include "zookeeper/zookeeper.hh"
using namespace org::apache::zookeeper;

/**
 * \file zookeeper.h 
 * \brief ZooKeeper functions and definitions.
 * 
 * ZooKeeper is a network service that may be backed by a cluster of
 * synchronized servers. The data in the service is represented as a tree
 * of data nodes. Each node has data, children, an ACL, and status information.
 * The data for a node is read and write in its entirety.
 * 
 * ZooKeeper clients can leave watches when they queries the data or children
 * of a node. If a watch is left, that client will be notified of the change.
 * The notification is a one time trigger. Subsequent chances to the node will
 * not trigger a notification unless the client issues a query with the watch
 * flag set. If the client is ever disconnected from the service, the watches do 
 * not need to be reset. The client automatically resets the watches.
 * 
 * When a node is created, it may be flagged as an ephemeral node. Ephemeral
 * nodes are automatically removed when a client session is closed or when
 * a session times out due to inactivity (the ZooKeeper runtime fills in
 * periods of inactivity with pings). Ephemeral nodes cannot have children.
 * 
 * ZooKeeper clients are identified by a server assigned session id. For
 * security reasons The server
 * also generates a corresponding password for a session. A client may save its
 * id and corresponding password to persistent storage in order to use the
 * session across program invocation boundaries.
 */

/* Support for building on various platforms */

// on cygwin we should take care of exporting/importing symbols properly 
#ifdef DLL_EXPORT
#    define ZOOAPI __declspec(dllexport)
#else
#  if (defined(__CYGWIN__) || defined(WIN32)) && !defined(USE_STATIC_LIB)
#    define ZOOAPI __declspec(dllimport)
#  else
#    define ZOOAPI
#  endif
#endif

/** zookeeper return constants **/

enum ZOO_ERRORS {
  ZOK = 0, /*!< Everything is OK */

  /** System and server-side errors.
   * This is never thrown by the server, it shouldn't be used other than
   * to indicate a range. Specifically error codes greater than this
   * value, but lesser than {@link #ZAPIERROR}, are system errors. */
  ZSYSTEMERROR = -1,
  ZRUNTIMEINCONSISTENCY = -2, /*!< A runtime inconsistency was found */
  ZDATAINCONSISTENCY = -3, /*!< A data inconsistency was found */
  ZCONNECTIONLOSS = -4, /*!< Connection to the server has been lost */
  ZMARSHALLINGERROR = -5, /*!< Error while marshalling or unmarshalling data */
  ZUNIMPLEMENTED = -6, /*!< Operation is unimplemented */
  ZOPERATIONTIMEOUT = -7, /*!< Operation timeout */
  ZBADARGUMENTS = -8, /*!< Invalid arguments */
  ZINVALIDSTATE = -9, /*!< Invliad zhandle state */

  /** API errors.
   * This is never thrown by the server, it shouldn't be used other than
   * to indicate a range. Specifically error codes greater than this
   * value are API errors (while values less than this indicate a 
   * {@link #ZSYSTEMERROR}).
   */
  ZAPIERROR = -100,
  ZNONODE = -101, /*!< Node does not exist */
  ZNOAUTH = -102, /*!< Not authenticated */
  ZBADVERSION = -103, /*!< Version conflict */
  ZNOCHILDRENFOREPHEMERALS = -108, /*!< Ephemeral nodes may not have children */
  ZNODEEXISTS = -110, /*!< The node already exists */
  ZNOTEMPTY = -111, /*!< The node has children */
  ZSESSIONEXPIRED = -112, /*!< The session has been expired by the server */
  ZINVALIDCALLBACK = -113, /*!< Invalid callback specified */
  ZINVALIDACL = -114, /*!< Invalid ACL specified */
  ZAUTHFAILED = -115, /*!< Client authentication failed */
  ZCLOSING = -116, /*!< ZooKeeper is closing */
  ZNOTHING = -117, /*!< (not error) no server responses to process */
  ZSESSIONMOVED = -118 /*!<session moved to another server, so operation is ignored */ 
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Interest Consts
 * These constants are used to express interest in an event and to
 * indicate to zookeeper which events have occurred. They can
 * be ORed together to express multiple interests. These flags are
 * used in the interest and event parameters of 
 * \ref zookeeper_interest and \ref zookeeper_process.
 */
// @{
extern ZOOAPI const int ZOOKEEPER_WRITE;
extern ZOOAPI const int ZOOKEEPER_READ;
// @}

/**
 * @name Create Flags
 * 
 * These flags are used by zoo_create to affect node create. They may
 * be ORed together to combine effects.
 */
// @{
extern ZOOAPI const int ZOO_EPHEMERAL;
extern ZOOAPI const int ZOO_SEQUENCE;
// @}

/**
 * @name Watch Types
 * These constants indicate the event that caused the watch event. They are
 * possible values of the first parameter of the watcher callback.
 */
// @{
/**
 * \brief a node has been created.
 * 
 * This is only generated by watches on non-existent nodes. These watches
 * are set using \ref zoo_exists.
 */
extern ZOOAPI const int ZOO_CREATED_EVENT;
/**
 * \brief a node has been deleted.
 * 
 * This is only generated by watches on nodes. These watches
 * are set using \ref zoo_exists and \ref zoo_get.
 */
extern ZOOAPI const int ZOO_DELETED_EVENT;
/**
 * \brief a node has changed.
 * 
 * This is only generated by watches on nodes. These watches
 * are set using \ref zoo_exists and \ref zoo_get.
 */
extern ZOOAPI const int ZOO_CHANGED_EVENT;
/**
 * \brief a change as occurred in the list of children.
 * 
 * This is only generated by watches on the child list of a node. These watches
 * are set using \ref zoo_get_children or \ref zoo_get_children2.
 */
extern ZOOAPI const int ZOO_CHILD_EVENT;
/**
 * \brief a session has been lost.
 * 
 * This is generated when a client loses contact or reconnects with a server.
 */
extern ZOOAPI const int ZOO_SESSION_EVENT;

/**
 * \brief a watch has been removed.
 * 
 * This is generated when the server for some reason, probably a resource
 * constraint, will no longer watch a node for a client.
 */
extern ZOOAPI const int ZOO_NOTWATCHING_EVENT;
// @}

/**
 * \brief ZooKeeper handle.
 *
 * This is the handle that represents a connection to the ZooKeeper service.
 * It is needed to invoke any ZooKeeper function. A handle is obtained using
 * \ref zookeeper_init.
 */
class zhandle_t;

/**
 * \brief client id structure.
 * 
 * This structure holds the id and password for the session. This structure
 * should be treated as opaque. It is received from the server when a session
 * is established and needs to be sent back as-is when reconnecting a session.
 */
typedef struct {
    int64_t client_id;
    char passwd[16];
} clientid_t;

/**
 * \brief signature of a watch function.
 * 
 * There are two ways to receive watch notifications: legacy and watcher object.
 * <p>
 * The legacy style, an application wishing to receive events from ZooKeeper must 
 * first implement a function with this signature and pass a pointer to the function 
 * to \ref zookeeper_init. Next, the application sets a watch by calling one of 
 * the getter API that accept the watch integer flag (for example, \ref zoo_aexists, 
 * \ref zoo_get, etc).
 * <p>
 * The watcher object style uses an instance of a "watcher object" which in 
 * the C world is represented by a pair: a pointer to a function implementing this
 * signature and a pointer to watcher context -- handback user-specific data. 
 * When a watch is triggered this function will be called along with 
 * the watcher context. An application wishing to use this style must use
 * the getter API functions with the "w" prefix in their names (for example, \ref
 * zoo_awexists, \ref zoo_wget, etc).
 * 
 * \param zh zookeeper handle
 * \param type event type. This is one of the *_EVENT constants. 
 * \param state connection state. The state value will be one of the *_STATE constants.
 * \param path znode path for which the watcher is triggered. NULL if the event 
 * type is ZOO_SESSION_EVENT
 * \param watcherCtx watcher context.
 */
typedef void (*watcher_fn)(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx);

/**
 * \brief create a handle to used communicate with zookeeper.
 * 
 * This method creates a new handle and a zookeeper session that corresponds
 * to that handle. Session establishment is asynchronous, meaning that the
 * session should not be considered established until (and unless) an
 * event of state ZOO_CONNECTED_STATE is received.
 * \param host comma separated host:port pairs, each corresponding to a zk
 *   server. e.g. "127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002"
 * \param fn the global watcher callback function. When notifications are
 *   triggered this function will be invoked.
 * \param clientid the id of a previously established session that this
 *   client will be reconnecting to. Pass 0 if not reconnecting to a previous
 *   session. Clients can access the session id of an established, valid,
 *   connection by calling \ref zoo_client_id. If the session corresponding to
 *   the specified clientid has expired, or if the clientid is invalid for 
 *   any reason, the returned zhandle_t will be invalid -- the zhandle_t 
 *   state will indicate the reason for failure (typically
 *   ZOO_EXPIRED_SESSION_STATE).
 * \param context the handback object that will be associated with this instance 
 *   of zhandle_t. Application can access it (for example, in the watcher 
 *   callback) using \ref zoo_get_context. The object is not used by zookeeper 
 *   internally and can be null.
 * \param flags reserved for future use. Should be set to zero.
 * \return a pointer to the opaque zhandle structure. If it fails to create 
 * a new zhandle the function returns NULL and the errno variable 
 * indicates the reason.
 */
ZOOAPI zhandle_t *zookeeper_init(const char *host, boost::shared_ptr<Watch> watch,
  int recv_timeout, const clientid_t *clientid, int flags);

/**
 * \brief close the zookeeper handle and free up any resources.
 * 
 * After this call, the client session will no longer be valid. The function
 * will flush any outstanding send requests before return. As a result it may 
 * block.
 *
 * This method should only be called only once on a zookeeper handle. Calling
 * twice will cause undefined (and probably undesirable behavior). Calling any other
 * zookeeper method after calling close is undefined behaviour and should be avoided.
 *
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \return a result code. Regardless of the error code returned, the zhandle 
 * will be destroyed and all resources freed. 
 *
 * ZOK - success
 * ZBADARGUMENTS - invalid input parameters
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 * ZOPERATIONTIMEOUT - failed to flush the buffers within the specified timeout.
 * ZCONNECTIONLOSS - a network error occured while attempting to send request to server
 * ZSYSTEMERROR -- a system (OS) error occured; it's worth checking errno to get details
 */
ZOOAPI int zookeeper_close(zhandle_t *zh);

/**
 * \brief return the client session id, only valid if the connections
 * is currently connected (ie. last watcher state is ZOO_CONNECTED_STATE)
 */
ZOOAPI const clientid_t *zoo_client_id(zhandle_t *zh);

/**
 * \brief return the timeout for this session, only valid if the connections
 * is currently connected (ie. last watcher state is ZOO_CONNECTED_STATE). This
 * value may change after a server re-connect.
 */
ZOOAPI int zoo_recv_timeout(zhandle_t *zh);

/**
 * \brief return the context for this handle.
 */
ZOOAPI const void *zoo_get_context(zhandle_t *zh);

/**
 * \brief set the context for this handle.
 */
ZOOAPI void zoo_set_context(zhandle_t *zh, void *context);

/**
 * \brief set a watcher function
 * \return previous watcher function
 */
ZOOAPI watcher_fn zoo_set_watcher(zhandle_t *zh,watcher_fn newFn);

/**
 * \brief returns the socket address for the current connection
 * \return socket address of the connected host or NULL on failure, only valid if the
 * connection is current connected
 */
ZOOAPI struct sockaddr* zookeeper_get_connected_host(zhandle_t *zh,
        struct sockaddr *addr, socklen_t *addr_len);

/**
 * \brief signature of a completion function for a call that returns void.
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*void_completion_t)(int rc, const void *data);
typedef void (*multi_completion_t)(int rc,
             const boost::ptr_vector<OpResult>& results,
             const void *data);

/**
 * \brief signature of a completion function that returns a Stat structure.
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param stat a pointer to the stat information for the node involved in
 *   this function. If a non zero error code is returned, the content of
 *   stat is undefined. The programmer is NOT responsible for freeing stat.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*stat_completion_t)(int rc, const data::Stat& stat,
        const void *data);

/**
 * \brief signature of a completion function that returns data.
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param value the value of the information returned by the asynchronous call.
 *   If a non zero error code is returned, the content of value is undefined.
 *   The programmer is NOT responsible for freeing value.
 * \param value_len the number of bytes in value.
 * \param stat a pointer to the stat information for the node involved in
 *   this function. If a non zero error code is returned, the content of
 *   stat is undefined. The programmer is NOT responsible for freeing stat.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*data_completion_t)(int rc, const std::string& value,
        const org::apache::zookeeper::data::Stat& stat, const void *data);

/**
 * \brief signature of a completion function that returns a list of strings.
 *
 * This method will be invoked at the end of a asynchronous call and also as
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param strings a pointer to the structure containng the list of strings of the
 *   names of the children of a node. If a non zero error code is returned,
 *   the content of strings is undefined. The programmer is NOT responsible
 *   for freeing strings.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*strings_completion_t)(int rc,
        const std::vector<std::string>& strings, const void *data);

/**
 * \brief signature of a completion function that returns a list of strings and stat.
 * .
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param strings a pointer to the structure containng the list of strings of the
 *   names of the children of a node. If a non zero error code is returned,
 *   the content of strings is undefined. The programmer is NOT responsible
 *   for freeing strings.
 * \param stat a pointer to the stat information for the node involved in
 *   this function. If a non zero error code is returned, the content of
 *   stat is undefined. The programmer is NOT responsible for freeing stat.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*strings_stat_completion_t)(int rc,
        const std::vector<std::string>& strings, const data::Stat& stat,
        const void *data);

/**
 * \brief signature of a completion function that returns a list of strings.
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param value the value of the string returned.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void
        (*string_completion_t)(int rc, const std::string& value, const void *data);

/**
 * \brief signature of a completion function that returns an ACL.
 * 
 * This method will be invoked at the end of a asynchronous call and also as 
 * a result of connection loss or timeout.
 * \param rc the error code of the call. Connection loss/timeout triggers 
 * the completion with one of the following error codes:
 * ZCONNECTIONLOSS -- lost connection to the server
 * ZOPERATIONTIMEOUT -- connection timed out
 * Data related events trigger the completion with error codes listed the 
 * Exceptions section of the documentation of the function that initiated the
 * call. (Zero indicates call was successful.)
 * \param acl a pointer to the structure containng the ACL of a node. If a non 
 *   zero error code is returned, the content of strings is undefined. The
 *   programmer is NOT responsible for freeing acl.
 * \param stat a pointer to the stat information for the node involved in
 *   this function. If a non zero error code is returned, the content of
 *   stat is undefined. The programmer is NOT responsible for freeing stat.
 * \param data the pointer that was passed by the caller when the function
 *   that this completion corresponds to was invoked. The programmer
 *   is responsible for any memory freeing associated with the data
 *   pointer.
 */
typedef void (*acl_completion_t)(int rc, const std::vector<data::ACL>& acl,
        const data::Stat& stat, const void *data);

/**
 * \brief get the state of the zookeeper connection.
 * 
 * The return value will be one of the \ref State Consts.
 */
ZOOAPI SessionState::type zoo_state(zhandle_t *zh);

/**
 * \brief create a node.
 * 
 * This method will create a node in ZooKeeper. A node can only be created if
 * it does not already exists. The Create Flags affect the creation of nodes.
 * If ZOO_EPHEMERAL flag is set, the node will automatically get removed if the
 * client session goes away. If the ZOO_SEQUENCE flag is set, a unique
 * monotonically increasing sequence number is appended to the path name. The
 * sequence number is always fixed length of 10 digits, 0 padded.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path The name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param value The data to be stored in the node.
 * \param valuelen The number of bytes in data.
 * \param acl The initial ACL of the node. The ACL must not be null or empty.
 * \param flags this parameter can be set to 0 for normal create or an OR
 *    of the Create Flags
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the parent node does not exist.
 * ZNODEEXISTS the node already exists
 * ZNOAUTH the client does not have permission.
 * ZNOCHILDRENFOREPHEMERALS cannot create children of ephemeral nodes.
 * \param data The data that will be passed to the completion routine when the 
 * function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_acreate(zhandle_t *zh, const std::string& path, const char *value,
        int valuelen, const std::vector<org::apache::zookeeper::data::ACL>& acl,
        int flags, string_completion_t completion, const void *data,
        bool isSynchronous);

/**
 * \brief delete a node in zookeeper.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param version the expected version of the node. The function will fail if the
 *    actual version of the node does not match the expected version.
 *  If -1 is used the version check will not take place. 
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * ZBADVERSION expected version does not match actual version.
 * ZNOTEMPTY children are present; node cannot be deleted.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_adelete(zhandle_t *zh, const std::string& path, int version,
        void_completion_t completion, const void *data, bool isSynchronous);


/**
 * \brief checks the existence of a node in zookeeper.
 * 
 * This function is similar to \ref zoo_axists except it allows one specify 
 * a watcher object - a function pointer and associated context. The function
 * will be called once the watch has fired. The associated context data will be 
 * passed to the function as the watcher context parameter. 
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param watcher if non-null a watch will set on the specified znode on the server.
 * The watch will be set even if the node does not exist. This allows clients 
 * to watch for nodes to appear.
 * \param watcherCtx user specific data, will be passed to the watcher callback.
 * Unlike the global context set by \ref zookeeper_init, this watcher context
 * is associated with the given instance of the watcher only.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when the 
 * function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_awexists(zhandle_t *zh, const std::string& path,
        boost::shared_ptr<Watch> watch,
        stat_completion_t completion, const void *data, bool isSynchronous);

/**
 * \brief gets the data associated with a node.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param watch if nonzero, a watch will be set at the server to notify 
 * the client if the node changes.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either in ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_aget(zhandle_t *zh, const std::string& path, int watch,
        data_completion_t completion, const void *data);

/**
 * \brief gets the data associated with a node.
 * 
 * This function is similar to \ref zoo_aget except it allows one specify 
 * a watcher object rather than a boolean watch flag. 
 *
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param watcher if non-null, a watch will be set at the server to notify 
 * the client if the node changes.
 * \param watcherCtx user specific data, will be passed to the watcher callback.
 * Unlike the global context set by \ref zookeeper_init, this watcher context
 * is associated with the given instance of the watcher only.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either in ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_awget(zhandle_t *zh, const std::string& path,
        boost::shared_ptr<Watch> watch,
        data_completion_t completion, const void *data, bool isSynchronous);

/**
 * \brief sets the data associated with a node.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param buffer the buffer holding data to be written to the node.
 * \param buflen the number of bytes from buffer to write.
 * \param version the expected version of the node. The function will fail if 
 * the actual version of the node does not match the expected version. If -1 is 
 * used the version check will not take place. * completion: If null, 
 * the function will execute synchronously. Otherwise, the function will return 
 * immediately and invoke the completion routine when the request completes.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * ZBADVERSION expected version does not match actual version.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_aset(zhandle_t *zh, const std::string& path, const char *buffer, int buflen, 
        int version, stat_completion_t completion, const void *data,
        bool isSynchronous);

/**
 * \brief lists the children of a node, and get the parent stat.
 * 
 * This function is similar to \ref zoo_aget_children2 except it allows one specify 
 * a watcher object rather than a boolean watch flag.
 *  
 * This function is new in version 3.3.0
 *
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param watcher if non-null, a watch will be set at the server to notify 
 * the client if the node changes.
 * \param watcherCtx user specific data, will be passed to the watcher callback.
 * Unlike the global context set by \ref zookeeper_init, this watcher context
 * is associated with the given instance of the watcher only.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_awget_children2(zhandle_t *zh, const std::string& path,
        boost::shared_ptr<Watch> watch,
        strings_stat_completion_t completion, const void *data,
        bool isSynchronous);

/**
 * \brief Flush leader channel.
 *
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes
 * separating ancestors of the node.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */

ZOOAPI int zoo_async(zhandle_t *zh, const std::string& path,
        string_completion_t completion, const void *data);


/**
 * \brief gets the acl associated with a node.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_aget_acl(zhandle_t *zh, const std::string& path, acl_completion_t completion, 
        const void *data, bool isSynchronous);

/**
 * \brief sets the acl associated with a node.
 * 
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param path the name of the node. Expressed as a file name with slashes 
 * separating ancestors of the node.
 * \param buffer the buffer holding the acls to be written to the node.
 * \param buflen the number of bytes from buffer to write.
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with one of the following codes passed in as the rc argument:
 * ZOK operation completed successfully
 * ZNONODE the node does not exist.
 * ZNOAUTH the client does not have permission.
 * ZINVALIDACL invalid ACL specified
 * ZBADVERSION expected version does not match actual version.
 * \param data the data that will be passed to the completion routine when 
 * the function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 */
ZOOAPI int zoo_aset_acl(zhandle_t *zh, const std::string& path, int version,
        const std::vector<org::apache::zookeeper::data::ACL>& acl,
        void_completion_t, const void *data, bool isSynchronous);

/**
 * \brief atomically commits multiple zookeeper operations.
 *
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param count the number of operations
 * \param ops an array of operations to commit
 * \param results an array to hold the results of the operations
 * \param completion the routine to invoke when the request completes. The completion
 * will be triggered with any of the error codes that can that can be returned by the 
 * ops supported by a multi op (see \ref zoo_acreate, \ref zoo_adelete, \ref zoo_aset).
 * \param data the data that will be passed to the completion routine when
 * the function completes.
 * \return the return code for the function call. This can be any of the
 * values that can be returned by the ops supported by a multi op (see
 * \ref zoo_acreate, \ref zoo_adelete, \ref zoo_aset).
 */
ZOOAPI int zoo_amulti(zhandle_t *zh,
        const boost::ptr_vector<org::apache::zookeeper::Op>& ops,
        multi_completion_t, const void *data, bool isSynchronous);

/**
 * \brief specify application credentials.
 * 
 * The application calls this function to specify its credentials for purposes
 * of authentication. The server will use the security provider specified by 
 * the scheme parameter to authenticate the client connection. If the 
 * authentication request has failed:
 * - the server connection is dropped
 * - the watcher is called with the ZOO_AUTH_FAILED_STATE value as the state 
 * parameter.
 * \param zh the zookeeper handle obtained by a call to \ref zookeeper_init
 * \param scheme the id of authentication scheme. Natively supported:
 * "digest" password-based authentication
 * \param cert application credentials. The actual value depends on the scheme.
 * \param certLen the length of the data parameter
 * \param completion the routine to invoke when the request completes. One of 
 * the following result codes may be passed into the completion callback:
 * ZOK operation completed successfully
 * ZAUTHFAILED authentication failed 
 * \param data the data that will be passed to the completion routine when the 
 * function completes.
 * \return ZOK on success or one of the following errcodes on failure:
 * ZBADARGUMENTS - invalid input parameters
 * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
 * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
 * ZSYSTEMERROR - a system error occured
 */
ZOOAPI int zoo_add_auth(zhandle_t *zh,const char* scheme,const char* cert, 
	int certLen, void_completion_t completion, const void *data,
        bool isSynchronous);

/**
 * \brief checks if the current zookeeper connection state can't be recovered.
 * 
 *  The application must close the zhandle and try to reconnect.
 * 
 * \param zh the zookeeper handle (see \ref zookeeper_init)
 * \return ZINVALIDSTATE if connection is unrecoverable
 */
ZOOAPI int is_unrecoverable(zhandle_t *zh);

/**
 * \brief enable/disable quorum endpoint order randomization
 * 
 * Note: typically this method should NOT be used outside of testing.
 *
 * If passed a non-zero value, will make the client connect to quorum peers
 * in the order as specified in the zookeeper_init() call.
 * A zero value causes zookeeper_init() to permute the peer endpoints
 * which is good for more even client connection distribution among the 
 * quorum peers.
 */
ZOOAPI void zoo_deterministic_conn_order(int yesOrNo);

#ifdef __cplusplus
}
#endif

#endif /*ZOOKEEPER_H_*/

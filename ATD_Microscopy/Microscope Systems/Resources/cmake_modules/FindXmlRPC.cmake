INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (LIBRARIES_DIR_PATH UIModule ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Libraries NO_DEFAULT_PATH)

IF (LIBRARIES_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Gci Libraries")
ENDIF (LIBRARIES_DIR_PATH-NOTFOUND)

MAKE_WINDOWS_PATH(LIBRARIES_DIR_PATH)

SET(XMLRPC_SRCS 	${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/base64.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpc.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcClient.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcClient.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcDispatch.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcDispatch.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcException.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServer.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServer.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServerConnection.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServerConnection.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServerMethod.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcServerMethod.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcSocket.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcSocket.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcSource.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcSource.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcUtil.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcUtil.h
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcValue.cpp
					${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src/XmlRpcValue.h
)

INCLUDE_DIRECTORIES(${LIBRARIES_DIR_PATH}/xmlrpc++0.7/src)
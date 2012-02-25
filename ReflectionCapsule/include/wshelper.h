#ifndef _poco_ws_helper_h_
# define _poco_ws_helper_h_

#include "poco.h"

#if defined(WIN32)
#  if defined(BUILD_WS_DLL)
#    define _POCO_WS_EXPORT __declspec(dllexport)
#  else
#    define _POCO_WS_EXPORT __declspec(dllimport)
#  endif
#else
#  define _POCO_WS_EXPORT
#endif

class POCO_WSServer;	// server runtime
class POCO_WSClient;	// client runtime

#ifdef USE_WASP
#include <waspc-impl/service/CppServiceInstance.h>
#include <waspc/client/Stub.h>
typedef WASP_CppServiceInstance		POCO_WSSkel;
typedef WASP_Stub			POCO_WSStub;
#endif

class _POCO_WS_EXPORT POCO_WSHelper {
  public:
	static POCO_WSServer*	WS_init(const char* configFileName);
	static void		WS_term(POCO_WSServer* server);
	static POCO_WSClient*	WS_client_init(const char* configFileName);
	static void		WS_client_term(POCO_WSClient* client);
	static void		WS_run();

	// to support Event/Notification

	static void export_service(
				POCO_WSServer*		server,
				POCO_WSSkel*		serviceImpl,
				const char*		serviceName,
				const char*		wsdlFileName,
				const char*		dispatcherName);

	static void binding(
				POCO_WSClient*		client,
				POCO_WSStub*		stub,
				const char*		url);
};

#define POCO_WSHELPER_WS_BINDING(client, stub, url)	(POCO_WSHelper::binding(client, stub, url), stub)

#endif

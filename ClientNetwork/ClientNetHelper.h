#pragma once

#pragma comment(lib, "ClientNetwork.lib")

#ifdef _DEBUG
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#pragma comment(lib, "DbgHelp.lib")

#include "ClientNetworkPch.h"
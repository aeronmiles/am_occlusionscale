/*
*  This header file provides the classes for writing to the modo event log and debug output.
*/

// Include the required modo SDK headers
#include <lx_plugin.hpp>
#include <lx_log.hpp>
#include <lxu_log.hpp>

// Set any namespaces
using namespace std;

#define PLUGIN_DEBUG_LOG	"myPlugin"
#define PLUGIN_VERSION		"601"
#define PLUGIN_COPYRIGHT	"Copyright 2012, My Company"

//--------------------------------------------------------------
//  Define the classes and methods for the debug and event log output.
//--------------------------------------------------------------

class plugin_logMessage : public CLxLogMessage
{
public:
	plugin_logMessage(char *subSys = NULL) : CLxLogMessage(PLUGIN_DEBUG_LOG) { Setup(); }
	virtual ~plugin_logMessage() {}

	const char * GetFormat() { return PLUGIN_DEBUG_LOG; }
	const char * GetVersion() { return PLUGIN_VERSION; }
	const char * GetCopyright() { return PLUGIN_COPYRIGHT; }
};

class plugin_debug
{
	/*
	*  This function creates a CLxUser_LogService item and defines functions for writing out to the debug output.
	*  The debug output is defined using the -Debug: Verbose, Trace, filename.txt ... etc when launching modo.
	*/

public:
	CLxUser_LogService		plugin_debug_log;
	plugin_logMessage		plugin_event_log;

	void debug(const char * message, int level = 2);
	void debug(string message, int level = 2);

	void eventLog(const char * message, int level = 1);
	void eventLog(string message, int level = 1);
}; 

#pragma once

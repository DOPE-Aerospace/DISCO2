#ifndef log_lib
#define log_lib

#include <Timer.h>
#include "status_handler.h"
#include "config.h"
#include "file_lib.h"

class Logger 
{
	
	smart_file file;
	
	String job_name, catagories;
	
	void create_log();
	
public:
	
	Logger() = default;

	Logger(const String& _job_name, const String& _catagories);
	
	void record_event(const String& text);
	void record_event(const char* text);

};

class MessageLogger : public Logger
{

	void call_base_record_event(const String& text);

public:

	using Logger::Logger;

	void record_event(const String& text);
	void record_event(const char* text);

};


#endif

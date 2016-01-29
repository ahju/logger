#ifndef __LOGGER_H
#define __LOGGER_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define INFO 0
#define WARN 1
#define ERROR 2


void *log_file_holder(void *parm);
bool start_log_thread();

class Logger
{
private:
	Logger(string path, string name, int level=-1);
	
	//如下两个函数不实现，因为不允许复制和拷贝操作
	Logger(const Logger &);  
    Logger & operator = (const Logger &);  
    
	virtual ~Logger();
	
	void logit(string msg,int level);
	bool name_exists(string name);
	//给文件改名 : $xxx.log => $xxx.YYYYMMDD-HH(.N) : name=$xxx
	void log_rename(string name,bool nohour=false, bool crenew=true);
	
	static Logger *s_logger;
	static pthread_mutex_t s_instance_mux;
	
	string m_path;
	string m_name;
	int m_log_level;
	
	int m_log_fd;
	
public:
	pthread_mutex_t m_log_mux;
	static Logger *get_instance(string path="", string name="",int level=-1);
	void logit_debug(string msg);
	void logit_info(string msg);
	void logit_warn(string msg);
	void logit_error(string msg);
	//TODO 需要实现如下功能
	//string get_path();
	//void set_path(string path);
	//string get_name();
	//void set_name(string name);
	friend void *log_file_holder(void *parm);
	friend bool start_log_thread(void *parm);
};

#endif
	

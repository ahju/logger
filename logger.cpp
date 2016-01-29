/******************************************************************* 
 *  Copyright(c) 2001-2016 Changyou 17173
 *  All rights reserved. 
 *   
 *  文件名称: logger.cpp
 *  简要描述: 封装日志记录模块，使记录日志更简单
 *  
 *  
 *   
 *  创建日期: 2016-01-13
 *  作者: 阿祖
 *  说明: 使用单实例模式封装，功能包括
 *	1.能够接收路径，创建实例及文件
 *	2.能够根据文件名定时切换，每天生成一个文件
 *
 *  存在问题：
 *  1.目前一个程序只用使用一个logger，无法针对不同的文件使用多个
 *  2.日志的配置无法定制
 *   
 ******************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "logger.h"

	
/*************************************************
  Function:       Logger
  Description:    构造函数
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        path:日志存储路径
        name:日志名称
        level:日志级别，默认为DEBUG
  Output:         
  Return:         
  Others:         如果路径存入空值则在当前路径下建log目录
                  如果日志名称为空值则默认为tmp.log
*************************************************/
Logger::Logger(string path, string name,int level)
{
	if(path.empty() || "" == path)
	{
		path = "./log";
	}
	m_path = path;
	
	
	if(0 != access(m_path.c_str(), F_OK))
	{
		if(0 != mkdir(m_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
		{
			cerr<<"log path:"<<m_path<<" does not exit, and cannot be builed. emsg:"<<strerror(errno)<<endl;
		}
	}
	
	if(name.empty() || "" == name)
	{
		name = "tmp.log";
	}
	m_name = name;
	
	string filename = "";
	if(m_path[m_path.length()-1] == '/')
	{
		filename = m_path + m_name;
	}
	else
	{
		filename = m_path + "/" + m_name;
	}
	m_log_fd=open(filename.c_str(),O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE,0644);
	if(m_log_fd < 0)
	{
		printf("ERROR::open file %s error\n", filename.c_str());
	}
	
	m_log_level = level;
	pthread_mutex_init(&m_log_mux,NULL);
	start_log_thread(this);
}

Logger *Logger::s_logger = NULL;
pthread_mutex_t Logger::s_instance_mux = PTHREAD_MUTEX_INITIALIZER;

Logger::~Logger()
{
	if(m_log_fd > 0)
	{
		close(m_log_fd);
		m_log_fd = -1;
	}
	if(NULL != s_logger)
	{
		delete s_logger;
		s_logger = NULL;
	}
}


//TODO 需要实现如下功能
// 实现是更改路径和名称都需要重新生成文件，有多线程同步的问题
/*
string Logger::get_path()
{
	if(m_path.empty())
	{
		return "";
	}
	return m_path;
}

void Logger::set_path(string path)
{
	m_path = path;
}

string Logger::get_name()
{
	if(m_name.empty())
	{
		return "";
	}
	return m_name;
}

void Logger::set_name(string name)
{
	m_name = name;
}
*/
	
/*************************************************
  Function:       name_exists
  Description:    判断文件是否存在
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        name:需要判断的文件名
  Output:         
  Return:         true-存在；flase-不存在
  Others:         
*************************************************/
bool Logger::name_exists(string name)
{
	FILE *fp=fopen(name.c_str(),"r");
	if(fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}


/*************************************************
  Function:       log_rename
  Description:    修改文件名，将$xxx.log 修改为 $xxx.YYYYMMDD-HH(.N)
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        name:需要判断的文件名
        nohour:若为true则为按天更改文件名，false则为按小时更改
        crenew:当文件名存在时是否创建新文件，新文件则为原文件名加.N
  Output:         
  Return:         
  Others:         
*************************************************/
void Logger::log_rename(string name,bool nohour,bool crenew)
{
	if(!name_exists(name+".log"))
	{
		return;
	}
	time_t tm=time(NULL)-3600;
	char tmstr[64];
	if(nohour)
	{
		strftime(tmstr,64,"%Y%m%d",localtime(&tm));
	}
	else
	{
		strftime(tmstr,64,"%Y%m%d-%H",localtime(&tm));
	}
	if(!name_exists(name+"."+tmstr))
	{
		if(rename(string(name+".log").c_str(),string(name+"."+tmstr).c_str())<0)
		{
		}
	}
	else
	{
		if(crenew)
		{
			int ext=1;
			while(1)
			{
				char buf[64];
				sprintf(buf,"%lld",ext);
				if(!name_exists(name+"."+tmstr+"."+string(buf)))
				{
					if(rename(string(name+".log").c_str(),string(name+"."+tmstr+"."+string(buf)).c_str())<0)
					{
					}
					break;
				}
				++ext;
			}
		}
	}
	return;
}
	

/*************************************************
  Function:       log_file_holder
  Description:    全局的日志文件处理函数，该函数为线程函数
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        parm:线程参数
  Output:         
  Return:         
  Others:       按天生成新文件  
*************************************************/
void *log_file_holder(void *parm)
{
	int nowhour=-1;
	int nowday=-1;
	sleep(3);

	Logger *logger = (Logger *)parm;
	
	while(1)
	{
		time_t nowtm=time(NULL);
		struct tm *nowtms=localtime(&nowtm);
		int tmday=nowtms->tm_yday;
		int tmhour=nowtms->tm_hour;

		//超过1天了
		if(tmday!=nowday)
		{
			pthread_mutex_lock(&logger->m_log_mux);
			if(logger->m_log_fd)
			{
				close(logger->m_log_fd);
				logger->m_log_fd=0;
			}
			int n= 0;
			string filename = "";
			if(logger->m_path[logger->m_path.length()-1] == '/')
			{
				filename = logger->m_path + logger->m_name;
			}
			else
			{
				filename = logger->m_path + "/" + logger->m_name;
			}
			if((n = filename.find(".log")) != string::npos)
			{
				string fname = filename.substr(0, n);
				logger->log_rename(fname,true);
			}
			logger->m_log_fd = open(filename.c_str(),O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE,0644);
			if(!logger->m_log_fd)
			{
				printf("open %s error %s", filename.c_str(), strerror(errno));
			}
			pthread_mutex_unlock(&logger->m_log_mux);

			nowday=tmday;
		}
		
		
		sleep(1);
	}
	return NULL;
}

/*************************************************
  Function:       start_log_thread
  Description:    启动线程函数
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        parm:线程参数
  Output:         
  Return:         
  Others:        
*************************************************/
bool start_log_thread(void *parm)
{
	pthread_t logholder_pid;
	if(pthread_create(&logholder_pid,NULL,log_file_holder,parm))
	{
		printf("pthread_create():%s\n", strerror(errno));
		return false;
	}
	return true;
}


/*************************************************
  Function:       get_instance
  Description:    获取日志句柄
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        path:日志存储路径
        name:日志名称
        level:日志级别，默认为DEBUG
  Output:         
  Return:         
  Others:        
*************************************************/
Logger *Logger::get_instance(string path, string name,int level)
{
	if(NULL == s_logger)
	{
		pthread_mutex_lock(&s_instance_mux);
		if(NULL == s_logger)
		{
			s_logger = new Logger(path,name,level);
		}
		pthread_mutex_unlock(&s_instance_mux);
	}
	return s_logger;
}


/*************************************************
  Function:       logit
  Description:    记录日志
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:日志内容
        level:日志级别
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit(string msg,int level)
{
	if(msg.empty())
	{
		return;
	}
	if(level < m_log_level)
	{
		return;
	}
	bool isprint = false;
	char buf[32];
	time_t tmp_time = time(NULL);
	buf[strftime(buf,32,"%Y-%m-%d %H:%M:%S",localtime(&tmp_time))]='\0';
	string emsg = "[" + string(buf) + "]";
	if(level == 0)
	{
		emsg += " INFO ";
	}
	else if(level==1)
	{
		emsg += " WARN ";
	}
	else if(level==2)
	{
		emsg += " ERROR ";
	}
	else
	{
		emsg += " DEBUG ";
		isprint = true;  //DEBUG情况下打印到屏幕
	}
	emsg += msg + "\n";
	if(m_log_fd <= 0)
	{ 
		//stderr.
		cerr<<emsg;
	}
	else
	{
		pthread_mutex_lock(&m_log_mux);
		write(m_log_fd,emsg.data(),emsg.length());	
		pthread_mutex_unlock(&m_log_mux);
		if(isprint)
		{
			printf("%s", emsg.c_str());
		}
	}
}

/*************************************************
  Function:       logit_debug
  Description:    记录DEBUG日志
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:日志内容
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit_debug(string msg)
{
	logit(msg,-1);
}

/*************************************************
  Function:       logit_info
  Description:    记录INFO日志
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:日志内容
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit_info(string msg)
{
	logit(msg,INFO);
}

/*************************************************
  Function:       logit_warn
  Description:    记录WARN日志
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:日志内容
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit_warn(string msg)
{
	logit(msg,WARN);
}

/*************************************************
  Function:       logit_error
  Description:    记录ERROR日志
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:日志内容
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit_error(string msg)
{
	logit(msg,ERROR);
}

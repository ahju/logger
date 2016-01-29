/******************************************************************* 
 *  Copyright(c) 2001-2016 Changyou 17173
 *  All rights reserved. 
 *   
 *  �ļ�����: logger.cpp
 *  ��Ҫ����: ��װ��־��¼ģ�飬ʹ��¼��־����
 *  
 *  
 *   
 *  ��������: 2016-01-13
 *  ����: ����
 *  ˵��: ʹ�õ�ʵ��ģʽ��װ�����ܰ���
 *	1.�ܹ�����·��������ʵ�����ļ�
 *	2.�ܹ������ļ�����ʱ�л���ÿ������һ���ļ�
 *
 *  �������⣺
 *  1.Ŀǰһ������ֻ��ʹ��һ��logger���޷���Բ�ͬ���ļ�ʹ�ö��
 *  2.��־�������޷�����
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
  Description:    ���캯��
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        path:��־�洢·��
        name:��־����
        level:��־����Ĭ��ΪDEBUG
  Output:         
  Return:         
  Others:         ���·�������ֵ���ڵ�ǰ·���½�logĿ¼
                  �����־����Ϊ��ֵ��Ĭ��Ϊtmp.log
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


//TODO ��Ҫʵ�����¹���
// ʵ���Ǹ���·�������ƶ���Ҫ���������ļ����ж��߳�ͬ��������
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
  Description:    �ж��ļ��Ƿ����
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        name:��Ҫ�жϵ��ļ���
  Output:         
  Return:         true-���ڣ�flase-������
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
  Description:    �޸��ļ�������$xxx.log �޸�Ϊ $xxx.YYYYMMDD-HH(.N)
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        name:��Ҫ�жϵ��ļ���
        nohour:��Ϊtrue��Ϊ��������ļ�����false��Ϊ��Сʱ����
        crenew:���ļ�������ʱ�Ƿ񴴽����ļ������ļ���Ϊԭ�ļ�����.N
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
  Description:    ȫ�ֵ���־�ļ����������ú���Ϊ�̺߳���
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        parm:�̲߳���
  Output:         
  Return:         
  Others:       �����������ļ�  
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

		//����1����
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
  Description:    �����̺߳���
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        parm:�̲߳���
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
  Description:    ��ȡ��־���
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        path:��־�洢·��
        name:��־����
        level:��־����Ĭ��ΪDEBUG
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
  Description:    ��¼��־
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:��־����
        level:��־����
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
		isprint = true;  //DEBUG����´�ӡ����Ļ
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
  Description:    ��¼DEBUG��־
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:��־����
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
  Description:    ��¼INFO��־
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:��־����
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
  Description:    ��¼WARN��־
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:��־����
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
  Description:    ��¼ERROR��־
  Calls:          
  Called By:      
  Table Accessed: 
  Table Updated:  
  Input:          
        msg:��־����
  Output:         
  Return:         
  Others:        
*************************************************/
void Logger::logit_error(string msg)
{
	logit(msg,ERROR);
}

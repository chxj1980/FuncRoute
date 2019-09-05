#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__

#include <stdio.h>
#include "thread.h"


#if defined(_WIN32) || defined(_WIN64)

#else
//	#define __FUNCTION__     __PRETTY_FUNCTION__
#endif


class CLogFile
{
public:
	static int      m_ref_cnt; //���ü���
	static FILE *   m_fp_log; //��־�ļ����
	static CMutex   m_mutex;

	bool            m_is_print_to_file; //�Ƿ���Ϣ��ӡ�������ļ���[Ĭ����]
	bool            m_is_print_to_stdio; //�Ƿ���Ϣ��ӡ����׼�����[Ĭ����]

	char            m_log_path[600];

public:
	CLogFile();
	CLogFile(char *log_path);
	~CLogFile();

	int createLogFile();
	int closeLogFile();
	int writeLogFile(const char *pszFormat, ...);
	int getTimeStr(char *timeStr);
	int getExeDirPath(char *dir_path, int size);
	bool isFolderExist(const char *dir); //�ļ���·���Ƿ����
	int createDirectory(const char *pathName);
	int readConfigFile(char *filename);
};

#endif //__LOG_FILE_H__

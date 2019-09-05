#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include "../LogFile.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include "../../version.h"
#include <intrin.h>


volatile short g_log_file_ref_cnt = 0;
int CLogFile::m_ref_cnt = 0;
FILE * CLogFile::m_fp_log = NULL;
CMutex CLogFile::m_mutex;


CLogFile::CLogFile()
{
	memset(m_log_path, 0, sizeof(m_log_path));
	
	m_is_print_to_file = true;
	m_is_print_to_stdio = true;
	
	//-----------------------------------------------
	int size = 600; //MAX_PATH
	char exe_dir_path[600] = {0};

	int ret22 = getExeDirPath(exe_dir_path, size);
	if(ret22 == 0)
	{
		sprintf(exe_dir_path, "%s/video_decoder.cfg", exe_dir_path);
		int ret3 = readConfigFile(exe_dir_path); //���ⲿһ�θ��Ĳ����Ļ���
	}
	
	//-----------------------------------------------
//	if(m_ref_cnt == 0) //��һ�����ʵ�����𴴽���־�ļ�
	if(_InterlockedIncrement16(&g_log_file_ref_cnt) == 0)
	{
		int ret = createLogFile();
	}

	m_ref_cnt++;
}


CLogFile::CLogFile(char *log_path)
{
	sprintf(m_log_path, "%s", log_path);
	
	m_is_print_to_file = true;
	m_is_print_to_stdio = true;

	//-----------------------------------------------
	int size = 600; //MAX_PATH
	char exe_dir_path[600] = {0};

	int ret22 = getExeDirPath(exe_dir_path, size);
	if(ret22 == 0)
	{
		sprintf(exe_dir_path, "%s/video_decoder.cfg", exe_dir_path);
		int ret3 = readConfigFile(exe_dir_path); //���ⲿһ�θ��Ĳ����Ļ���
	}
	
	//-----------------------------------------------
//	if(m_ref_cnt == 0) //��һ�����ʵ�����𴴽���־�ļ�
	if(_InterlockedIncrement16(&g_log_file_ref_cnt) == 0)
	{
		int ret = createLogFile();
	}
	
	m_ref_cnt++;
}


CLogFile::~CLogFile()
{
	m_ref_cnt--;

//	if(m_ref_cnt == 0)
	if(_InterlockedDecrement16(&g_log_file_ref_cnt) == 0)
	{
		int ret = closeLogFile();
	}
}


int CLogFile::createLogFile()
{
	int ret = 0;
	int size = 600; //MAX_PATH
	char exe_dir_path[600] = {0};
	char log_dir_path[600] = {0};

	ret = getExeDirPath(exe_dir_path, size);
	if(ret != 0){return -21;}

	printf("[INFO] %s: exe dir path: %s\n", __FUNCTION__, exe_dir_path);
	
	//-----------------------------------------------
	sprintf(log_dir_path, "%s\\log", exe_dir_path);

	bool isExist = isFolderExist(log_dir_path);
	if(isExist == true && m_is_print_to_file == true) //Ŀ¼���ڣ���������־�ļ�
	{
		char str_time[80];
		struct tm newtime;
		time_t long_time = time(NULL);
		errno_t err = _localtime64_s(&newtime, &long_time);
		strftime(str_time, 80, "%Y-%m-%d", &newtime);

		sprintf(log_dir_path, "%s\\video_decoder\\%s", log_dir_path, str_time);
		if(isFolderExist(log_dir_path) == false)
		{
			ret = createDirectory(log_dir_path);
			if(ret != 0)
			{
				printf("Error: %s: can not create log dir path: %s\n", __FUNCTION__, log_dir_path);
				return -1;
			}
		}

		DWORD pid = ::GetCurrentProcessId();
		
//		strftime(str_time, 80, "%H.%M.%S", &newtime);

		if(strlen(m_log_path) == 0)
		{
//			sprintf(m_log_path, "%s\\%s_pid%ld.log", log_dir_path, str_time, pid);
			sprintf(m_log_path, "%s\\pid%ld.log", log_dir_path, pid);
		}
		printf("[INFO] %s: log file path: %s\n", __FUNCTION__, m_log_path);

		if(m_fp_log != NULL)
		{
			this->writeLogFile("Error: %s: the log file had been open before.\n", __FUNCTION__);
			return -2;
		}

		m_fp_log = fopen(m_log_path, "a"); //�Ը��ӵķ�ʽ��ֻд�ļ�,���ļ������ڣ���ᴴ�����ļ���
		if(m_fp_log == NULL)
		{
			printf("Error: Cannot open file to write. [%s]\n", m_log_path);
			return -31; //���ļ�ʧ��
		}

		this->writeLogFile("[INFO] %s: open the log file successfully. %s\n", __FUNCTION__, m_log_path);
		this->writeLogFile("[INFO] %s: decoder version: %s\n", __FUNCTION__, VERSION_STR3(VERSION_STR));
	}

	return 0;
}


int CLogFile::closeLogFile()
{
	this->writeLogFile("[INFO] %s: close the log file.\n", __FUNCTION__);

	if(m_fp_log){fclose(m_fp_log); m_fp_log = NULL;}
	return 0;
}


int CLogFile::writeLogFile(const char *pszFormat, ...)
{
	CAutoMutex autoLock(m_mutex);

	int ret = 0;

	const int logSize = 2 * 1024;
	static char strLog[logSize];
	
	va_list args;
	va_start(args, pszFormat);
	vsnprintf(strLog, logSize - 1, pszFormat, args);
	va_end(args);
	
	strLog[logSize - 1] = '\0';
	
	if(m_fp_log)
	{
		char time_str[100] = {0};
		ret = getTimeStr(time_str);
//		fwrite(time_str, strlen(time_str), 1, m_fp_log);
//		fwrite(strLog, strlen(strLog), 1, m_fp_log);
		fprintf(m_fp_log, "%s%s", time_str, strLog);
		ret = fflush(m_fp_log);
	}
	
	if(m_is_print_to_stdio == true)
	{
		printf("%s", strLog);
	}
	
	return ret;
}


int CLogFile::getTimeStr(char *timeStr)
{
	int ret = 0;
	
	char str_time[80];
	struct tm newtime;
	time_t long_time = time(NULL);
	errno_t err = _localtime64_s(&newtime, &long_time);
	strftime(str_time, 80, "%Y-%m-%d %H:%M:%S", &newtime);

	DWORD tid = GetCurrentThreadId();

	sprintf(timeStr, "[%s] [TID 0x%08x] ", str_time, tid);

	return ret;
}


int CLogFile::getExeDirPath(char *dir_path, int size)
{
	int ret = GetModuleFileNameA(NULL, dir_path, size);
	if(!ret)
	{
		ret = GetLastError();
		printf("Error: GetModuleFileNameA failed; ret=%d\n", ret);
		return -2;
	}

	char * p = strrchr(dir_path, '\\');
	if(!p){return -3;}
	
	p[0] = '\0';

	return 0;
}


bool CLogFile::isFolderExist(const char *dir)
{
	WIN32_FIND_DATA wfd;
	bool isExist = false;

	HANDLE hFind = FindFirstFile(dir, &wfd);
	
	if((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		isExist = true;
	}
	
	FindClose(hFind);
	
	return isExist;
}


int CLogFile::createDirectory(const char *pathName)
{
	char path[MAX_PATH] = {0};
	char pathName2[MAX_PATH] = {0};

	sprintf(pathName2, "%s\\", pathName);
	const char * pos = pathName2;

	while((pos = strchr(pos, '\\')) != NULL)
	{
		memcpy(path, pathName2, pos - pathName2 + 1);
		pos++;
		if(_access(path, 0) == 0)
		{
			continue;
		}else
		{
			int ret = _mkdir(path);
			if(ret != 0)
			{
				return -1;
			}
		}
	}

	return 0;
}


/*
�����ļ� video_decoder.cfg ��ʽ������ʹ��'//'ע�͵�һ�У�
//decoder_type=DECODER_TYPE_VIDEO_FFMPEG_CPU;
//decoder_thread_mode=DECODER_THREAD_MODE_SINGLE;
//number_of_packets=6000;
//number_of_pictures=0;
//is_demux_and_decoder_sync=false;
is_enable_decoder_plugin=true;
is_log_print_to_file=true;
is_log_print_to_stdio=true;
*/
int CLogFile::readConfigFile(char *filename)
{
	FILE *fp = fopen(filename, "r");
	if(!fp){return -1;}

	char line[200];
	char decoder_type[] = "decoder_type=";
	char decoder_thread_mode[] = "decoder_thread_mode=";
	char number_of_packets[] = "number_of_packets=";
	char number_of_pictures[] = "number_of_pictures=";
	char is_demux_and_decoder_sync[] = "is_demux_and_decoder_sync=";
	char is_enable_decoder_plugin[] = "is_enable_decoder_plugin=";
	char is_log_print_to_file[] = "is_log_print_to_file=";
	char is_log_print_to_stdio[] = "is_log_print_to_stdio=";

	int decoder_type_len = strlen(decoder_type);
	int decoder_thread_mode_len = strlen(decoder_thread_mode);
	int number_of_packets_len = strlen(number_of_packets);
	int number_of_pictures_len = strlen(number_of_pictures);
	int is_demux_and_decoder_sync_len = strlen(is_demux_and_decoder_sync);
	int is_enable_decoder_plugin_len = strlen(is_enable_decoder_plugin);
	int is_log_print_to_file_len = strlen(is_log_print_to_file);
	int is_log_print_to_stdio_len = strlen(is_log_print_to_stdio);

	while(fgets(line, 200, fp) != NULL)
	{
		if(strncmp(line, decoder_type, decoder_type_len) == 0)
		{
			
		}else if(strncmp(line, decoder_thread_mode, decoder_thread_mode_len) == 0)
		{
			
		}else if(strncmp(line, number_of_packets, number_of_packets_len) == 0)
		{
			
		}else if(strncmp(line, number_of_pictures, number_of_pictures_len) == 0)
		{

		}else if(strncmp(line, is_demux_and_decoder_sync, is_demux_and_decoder_sync_len) == 0)
		{

		}else if(strncmp(line, is_enable_decoder_plugin, is_enable_decoder_plugin_len) == 0)
		{

		}else if(strncmp(line, is_log_print_to_file, is_log_print_to_file_len) == 0)
		{
			if(strncmp(line + is_log_print_to_file_len, "true;", strlen("true;")) == 0)
			{
				m_is_print_to_file = true;
			}else if(strncmp(line + is_log_print_to_file_len, "false;", strlen("false;")) == 0)
			{
				m_is_print_to_file = false;
			}
		}else if(strncmp(line, is_log_print_to_stdio, is_log_print_to_stdio_len) == 0)
		{
			if(strncmp(line + is_log_print_to_stdio_len, "true;", strlen("true;")) == 0)
			{
				m_is_print_to_stdio = true;
			}else if(strncmp(line + is_log_print_to_stdio_len, "false;", strlen("false;")) == 0)
			{
				m_is_print_to_stdio = false;
			}
		}
	}

	fclose(fp);

	return 0;
}

#endif // #if defined(_WIN32) || defined(_WIN64)


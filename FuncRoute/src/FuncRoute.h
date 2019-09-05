#ifndef __FUNC_ROUTE_H__
#define __FUNC_ROUTE_H__

#include <string>
#include <vector>


typedef struct _BYTES_STREAM_
{
	unsigned char *buffer;
	unsigned int bufferSize;
	unsigned int offset;
}BYTES_STREAM;


typedef struct _STRING_POSITON_
{
	unsigned char *start;
	unsigned char *end;
	unsigned int lineNumberOfStart;
	unsigned int lineNumberOfEnd;
	unsigned int length;
}STRING_POSITON;


typedef struct _FUNCTION_STRUCTURE_
{
	STRING_POSITON functionReturnValue; //��������ֵ
	STRING_POSITON functionName; //������
	STRING_POSITON functionParameter; //��������
	STRING_POSITON functionBody; //������
	char funcString[1024]; //��������ֵ + ������ + ��������
}FUNCTION_STRUCTURE;


typedef struct _FUNCTIONS_
{
	unsigned char fllename[600]; //�����ļ���
	std::vector<FUNCTION_STRUCTURE> funcs;
}FUNCTIONS;


//---------C/C++Դ�����ļ��������ù�ϵ��-----------------
class CFuncRoute
{
public:
	std::string m_srcCodesFilePath; //C/C++Դ�����ļ�·��
	std::vector<std::string> m_fileSuffixes; //C/C++Դ�����ļ���׺�������Դ�Сд�����飬���� [".h", ".hpp", ".c", ".cpp", ".cc", "*"]

public:
	CFuncRoute();
	~CFuncRoute();

	int findAllFunctionsName(std::string filePath, std::vector<std::string> suffixes); //��Դ�����ļ����棬��ȡ�����к�����
	int search_C_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C���Ժ�����
	int search_CPP_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C++���Ժ�����
};

#endif //__FUNC_ROUTE_H__

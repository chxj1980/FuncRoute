#ifndef __FUNC_ROUTE_H__
#define __FUNC_ROUTE_H__

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include "CommonData.h"


//---------C/C++Դ�����ļ��������ù�ϵ��-----------------
class CFuncRoute
{
public:
	std::string m_srcCodesFilePath; //C/C++Դ�����ļ�·��
	std::vector<std::string> m_fileSuffixes; //C/C++Դ�����ļ���׺�������Դ�Сд�����飬���� [".h", ".hpp", ".c", ".cpp", ".cc", "*"]
	std::string m_filePathForPdfTex; //��������pdf��tex�ļ�����������Ϊ��pdflatex ./out_FuncRoute.tex�������ڵ�ǰĿ¼����./out_FuncRoute.pdf�ļ�

public:
	CFuncRoute();
	~CFuncRoute();

	int CFuncRoute::splitDirsBySemicolon(std::string dirs, std::vector<std::string> &vecDirs); //�÷ֺ�';'�ֿ��ַ���
	int findAllFunctionsName(std::vector<std::string> dirsInclude, std::vector<std::string> fileDirsExclude, std::vector<std::string> suffixes); //��Դ�����ļ����棬��ȡ�����к�����
	int search_C_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C���Ժ�����
	int search_CPP_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C++���Ժ�����
	bool isKeyword(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Թؼ���
	bool isKeywordVarType(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Ա����ؼ���
	int findOverloadOperatorsBack(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, unsigned char *&leftCharPos); //�����Ƿ���C++�����������
	int replaceAllCodeCommentsBySpace(unsigned char *buffer, int bufferSize); //��������"//..."��"/*...*/"ע�͵��Ĵ����ÿո�' '����
	int findCurLineStartAndEndPos(unsigned char *buffer, int bufferSize, unsigned char *curPos, unsigned char *&startPos, unsigned char *&endPos);
	int isBetweenInDoubleQuotes(unsigned char *buffer, int bufferSize, unsigned char *curPos, unsigned char *&startPos, unsigned char *&endPos); //��ǰ�ַ��Ƿ���һ��˫����֮��
	int isBetweenInSingleQuotes(unsigned char *buffer, int bufferSize, unsigned char *curPos, unsigned char *&startPos, unsigned char *&endPos); //��ǰ�ַ��Ƿ���һ�Ե�����֮��
	int replaceAllStrBySpace(unsigned char *buffer, int bufferSize); //��������˫����""�Ĵ����÷�������'`'����
	int replaceAllMacroDefineStrBySpace(unsigned char *buffer, int bufferSize); //������#define�궨���ÿո�' '����
	int findStr(unsigned char *buffer, int bufferSize, const char *str, int &pos); //���ڴ��У�����ָ�����ַ���
	int findAllMacros(std::vector<std::string> files, std::vector<MACRO> &macros); //�����д���Դ�ļ��У��ҵ����еĺ궨��
	int findAllClassAndStructDeclare(unsigned char *buffer, int bufferSize, std::vector<CLASS_STRUCT> &classes); //���ڴ��в�������������C++��/�ṹ������
	int findAllFuncsInFunctionBody(unsigned char *buffer, int bufferSize, char *funcParameter, std::vector<CLASS_INSTANCE> &funcsWhichInFunctionBody, unsigned char *bufferBase, int lineNumberBase); //���Һ������ڲ����õ�������������
	int findWholeFuncCalled(unsigned char *buffer, int bufferSize, char *funcParameter, unsigned char *parentheseLeft, CLASS_INSTANCE &classInstance, unsigned char *bufferBase, int lineNumberBase); //������С����'('��λ�ã�����һ�������ĺ�������
	int findWholeFuncDeclare(unsigned char *buffer, int bufferSize, unsigned char *parentheseLeft, FUNCTION_STRUCTURE &funcDeclare, unsigned char *bufferBase, int lineNumberBase); //������С����'('��λ�ã�����һ�������ĺ�������
	int findNextMacroDefine(unsigned char *buffer, int bufferSize, unsigned char *&leftPos, unsigned char *&rightCharPos); //���Ҳ�������һ��������#define������
	int findNextCodeComments(unsigned char *buffer, int bufferSize, unsigned char *&leftPos, unsigned char *&rightCharPos); //���Ҳ�������һ��������ע�����

	int skipWhiteSpaceForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos, int &lineNumber); //ǰ�������հ��ַ�
	int skipWhiteSpaceBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos, int &lineNumber); //���������հ��ַ�
	int findPairCharForward(unsigned char *buffer, int bufferSize, unsigned char *leftCharPos, char leftChar, char rightChar, unsigned char *&rightCharPos); //ǰ���������ַ�������"{}","<>","()","[]","''"
	int findPairCharBack(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, unsigned char *&leftCharPos); //�����������ַ�������"{}","<>","()","[]","''"
	int findPairCharBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, char *stopChar, unsigned char *&leftCharPos); //�����������ַ�������ֹͣ���򷵻�ʧ�ܣ�����"{}","<>","()","[]","''"
	int findCharForward(unsigned char *buffer, int bufferSize, char ch, unsigned char *&rightCharPos); //ǰ�����ָ���ַ�
	int findCharForwardStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&rightCharPos); //ǰ�����ָ���ַ�������ֹͣ���򷵻�ʧ��
	int findCharBack(unsigned char *buffer, int bufferSize, char ch, unsigned char *&leftCharPos); //�������ָ���ַ�
	int findCharsBackGreedy(unsigned char *buffer, int bufferSize, char *chars, unsigned char *&leftCharPos); //�������ָ����ĳ�����ַ���̰������ֱ���ҵ�Ϊֹ
	int findCharBackStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&leftCharPos); //�������ָ���ַ�������ֹͣ���򷵻�ʧ��
	int findCharStrsBackStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, const char stopStrs[][20], int stopStrsSize, unsigned char *&leftCharPos); //�������ָ���ַ�������ֹͣ����ָ�����ַ����򷵻�ʧ��
	int findStrForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos); //ǰ������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos); //��������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrVarTypeBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos); //������ұ��������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrCharsBackGreedy(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *chars, unsigned char *&leftPos); //��������ַ�������ָ�����ַ���̰������ֱ���ҵ�Ϊֹ��C++ �������ͱ��������������� + ��ĸ + �»���
	int findQueryStrBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *queryStr, char *stopChar, unsigned char *&leftPos); //�������ָ���ַ���������ֹͣ���򷵻�ʧ��
	int findQueryStrForwardStop(unsigned char *buffer, int bufferSize, unsigned char *leftPos, char *queryStr, char *stopChar, unsigned char *&rightPos); //ǰ�����ָ���ַ���������ֹͣ���򷵻�ʧ��
	int findVarDeclareForward(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType); //ǰ����ұ�������������
	int findVarDeclareBack(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType); //������ұ�������������
	bool isValidVarName(unsigned char *buffer, int bufferSize); //�Ƿ���һ����Ч�ı���������C++ �������ͱ��������������� + ��ĸ + �»���
	bool isValidVarChar(char ch); //�Ƿ���һ����Ч�ı���������C++ �������ͱ��������������� + ��ĸ + �»���
	bool isWhiteSpace(char ch); //�Ƿ���һ���հ��ַ���' ', '\t', '\r', '\n'
	int replaceTwoMoreWhiteSpaceByOneSpace(unsigned char *buffer, int bufferSize); //����������Ŀհ��ַ���һ���ո����
	int statBufferLinesCount(unsigned char *buffer, int bufferSize); //ͳ�Ƹ����ڴ��л��з�'\n'�ĸ���
	
	int macroExpand(); //���궨��չ��
	bool isParentClass(std::string child, std::string parent, std::vector<FUNCTIONS> &vFunctions); //�ж�parent���Ƿ���child�ĸ���
	int splitParentsClass(unsigned char *buffer, int bufferSize, std::vector<MY_STRING> &classParents); //��ָ����ǣ����磺"class B : public A, public C{};"
	int updateParentClass(std::vector<FUNCTIONS> &vFunctions); //��Ҫ����������ϵĸ���̳�
	int findAllMemberVarsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase); //����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����
	int findAllMemberFuncsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase); //����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����
	bool isFunctionArgsMatch(std::string parameter, std::string functionArgs); //�����������Ĳ����б�ͺ����ĵ��ô���Ĳ����б��Ƿ�ƥ��
	int statAllFuns(std::vector<FUNCTIONS> &vFunctions); //ͳ�����к���֮��ĵ��ù�ϵ
	int createAllFunsCalledTree(std::vector<FUNCTIONS> &vFunctions, FUNCS_CALLED_TREE &trees); //�����������ù�ϵ�����
	int createFunsCalledTree(std::vector<FUNCTIONS> &vFunctions, int queryFuncIndex, FUNC_INDEX **&arry, int arryLen); //�����������ù�ϵ�����
	int getFuncsPos(std::vector<FUNCTIONS> &vFunctions, int queryFuncIndex, int &index1, int &index2); //�����������ù�ϵ�����
	int dumpBufferToFile(unsigned char *buffer, int bufferSize, char *filename); //���ڴ�����д�������ļ�
	int printInfo(std::vector<FUNCTIONS> &vFunctions);

	int createPdfTexHeader(std::string &strTexHeader); //���� test.texͷ�� ����ת���� test.pdf
	int createPdfTexLogo(std::string &strTexlogo); //���� test.tex logo ����ת���� test.pdf
	int createPdfTexBody(std::vector<FUNCTIONS> &vFunctions, _FUNC_INDEX_ *rootNode, std::string &strTexBody, FILE *fp, long long &writeBytes, 
		int colMax, int rowMax, int colBase, int rowBase, std::vector<FUNC_INDEX_POS> &vecNodes); //���� test.tex���壬ת���� test.pdf
	int createPdfTexBodySub(std::vector<FUNCTIONS> &vFunctions, FILE *fp, long long &writeBytes, int colMax, int rowMax, int colBase, int rowBase, std::vector<FUNC_INDEX_POS> &vecNodes); //���� test.tex���壬ת���� test.pdf
	int createPdfTexTailer(std::string &strTexTailer); //���� test.texβ������ת���� test.pdf
	int getTexFuncName(std::vector<FUNCTIONS> &vFunctions, _FUNC_INDEX_ *node, bool isRecursiveFunction, int cloNum, int rowNum, std::string &strFuncName);
	
	int getBuildDate1(char *szBuildDate);
	int getBuildDate2(char *szBuildDate);
	int printDeltaTime(long long timeStart, long long timeEnd);
};

#endif //__FUNC_ROUTE_H__

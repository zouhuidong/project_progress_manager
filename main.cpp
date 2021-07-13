/*
	project_progress_manager
	main.cpp
	项目进度管理器

	所需文件要求：
	1. 程序同目录下必须存在project.dat（存储项目进度数据），settings.ini（配置文件）
	2. project.dat 文件格式要求
		第一行：项目名
		第二行：项目代码文件路径，如果有多个就每个写一行，可以写相对路径。如果要包含一个目录，就在此行写目录路径（如 "D:\myproject\web"），
			并在下一行写要纳入代码统计的文件的后缀名（如".cpp"），每个后缀名之间留一个空格，".*" 表示所有后缀名。
		第三行：时间（格式：月.日，个位数日期无需补0为两位数） 代码行数（单位行） 代码长度（单位字节）
		第n行以第三行类推
		注意：文件中不得含有多余的空格和换行

		文件示例1：
		testproject ver1.1
		D:\myproject\testproject\main.cpp
		7.10 100 900
		7.11 233 2562

		文件示例2：
		testproject ver1.1
		..\main.cpp
		..\png.cpp
		7.10 100 900
		7.11 233 2562

		文件示例3：
		testproject ver1.1
		D:\projects\testproject\
		.c .cpp
		7.10 100 900
		7.11 233 2562

	3. settings.ini 文件格式要求详见文件内注释。

	制作者：huidong <mailkey@yeah.net>
	创建时间：2021.7.10
	最后修改：2021.7.12
*/


#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <io.h>
#include <string>
using namespace std;

// 程序版本
const char strVersion[] = "Ver 1.0";
const char strReleaseTime[] = "2021.7.13";

// 文件路径
const char strDataFilePath[] = "./project.dat";				// 项目进度文件
const wchar_t* wstrSettingsFilePath = L"./settings.ini";	// 配置文件

// 进度信息
struct ProgressData
{
	int month;	// 月
	int day;	// 日
	int line;	// 代码行数
	int length;	// 代码长度
};


//
// FindFiles
// 寻找目录下的文件，返回满足条件的文件列表
// 
// lpPath 目标目录路径（路径末尾可以带'\'或者'/'，也可以不带）
// bGetChildFolder 返回的文件列表中是否包含子文件夹本身的目录路径（只有当secName为".*"时才能正常返回子文件夹目录路径）
// bGetChildFolderFiles 返回的文件列表中是否包含子文件夹中的文件
// p_wstrFilesList 传入一个空指针的地址，将得到和返回值相同的内容。
// pFilesNum 得到查找到的文件数量
// secName 要查找的文件的后缀名要求，".*"表示所有后缀名。只支持填写一个后缀名，且需要带上点('.')。
// recursion_flag 递归标志。标志当前函数是否在递归中，用户调用此函数时请保持此值为false
// 
// 注意：如果返回的文件列表中含有子文件夹路径，它们都会以'\'结尾，比如 "D:\new_folder\"
//
wstring* FindFiles(const wchar_t* lpPath, bool bGetChildFolder, bool bGetChildFolderFiles, wstring** p_wstrFilesList, int* pFilesNum, const wchar_t* secName = L".*", bool inner_flag = false)
{
	wchar_t szFind[MAX_PATH];
	wchar_t szFile[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	static vector<wstring> files_list;

	lstrcpy(szFind, lpPath);

	// '/' 全部改 '\'
	for (int i = 0; i < (int)lstrlen(szFind); i++)
		if (szFind[i] == L'/')
			szFind[i] = L'\\';

	// 末尾不含'\'时，带上'\'
	if (szFind[lstrlen(szFind) - 1] != '\\')
		lstrcat(szFind, L"\\");

	lstrcat(szFind, L"*");
	lstrcat(szFind, secName);

	// 如果有文件后缀名要求，且需要得到子文件夹中的文件
	// 如果限制后缀，其实是无法得到文件目录的，所以需要单独获取文件子目录
	if (bGetChildFolderFiles && lstrcmp(secName, L".*") != 0)
	{
		// 得到参数目录下的所有文件（含子文件夹内的文件）
		wstring* p_wstrAllFilesList = NULL;
		int nAllFilesNum = 0;
		FindFiles(lpPath, false, true, &p_wstrAllFilesList, &nAllFilesNum);

		// 遍历所有文件，查找符合后缀的文件
		for (int i = 0; i < nAllFilesNum; i++)
		{
			// 找到此文件路径的最后一个点('.')的位置
			int nLastPointIndex = 0;
			for (int j = 0; j < (int)p_wstrAllFilesList[i].size(); j++)
				if (p_wstrAllFilesList[i][j] == L'.')
					nLastPointIndex = j;
			// 后缀名一致，存入vector
			if (lstrcmp(&p_wstrAllFilesList[i][nLastPointIndex], secName) == 0)
			{
				files_list.push_back(p_wstrAllFilesList[i]);
			}
		}
	}

	// 如果没有上述要求，则正常进行读取
	else
	{
		HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);

		// 空目录
		if (INVALID_HANDLE_VALUE == hFind)
		{
			return NULL;
		}

		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 是文件夹
				if (FindFileData.cFileName[0] != '.')
				{
					lstrcpy(szFile, lpPath);

					// '/' 全部改 '\'
					for (int i = 0; i < (int)lstrlen(szFile); i++)
						if (szFile[i] == L'/')
							szFile[i] = L'\\';

					if (szFile[lstrlen(szFile) - 1] != '\\')
						lstrcat(szFile, L"\\");

					lstrcat(szFile, FindFileData.cFileName);
					lstrcat(szFile, L"\\");

					if (bGetChildFolder)
						files_list.push_back(szFile);

					if (bGetChildFolderFiles)
						FindFiles(szFile, bGetChildFolder, bGetChildFolderFiles, p_wstrFilesList, pFilesNum, secName, true);
				}
			}
			else
			{
				if (szFile[0])
				{
					wstring filePath = lpPath;
					if (lpPath[lstrlen(lpPath) - 1] != '\\')
						filePath += L"\\";
					filePath += FindFileData.cFileName;
					files_list.push_back(filePath);
				}
				else
				{
					wstring filePath = szFile;
					filePath += FindFileData.cFileName;
					files_list.push_back(filePath);
				}
			}

		} while (::FindNextFile(hFind, &FindFileData));

		::FindClose(hFind);
	}

	*pFilesNum = files_list.size();
	if (!inner_flag)
	{
		*p_wstrFilesList = new wstring[*pFilesNum];
		for (int i = 0; i < (int)files_list.size(); i++)
		{
			(*p_wstrFilesList)[i] = files_list[i];
		}
		files_list.clear();
	}

	return *p_wstrFilesList;
}

// 判断一个字符串是否为文件（夹）路径
// 非路径返回 0
// 绝对路径返回 1
// 相对路径返回 2
int isPath(const char* strPath)
{
	/*
		'/'和'\'可以互换
		绝对路径
		D:\path
		相对路径
		.\xxx.txt
		..\xxx.txt
		xxx.txt
	*/

	if (isalpha(strPath[0]))
	{
		if(strPath[1] == ':')
			return 1;
		else if (isalpha(strPath[0]))
			return 2;
	}
	else if (strPath[0] == '.' && (strPath[1] == '\\' || strPath[1] == '/' || strPath[2] == '\\' || strPath[2] == '/'))
	{
		return 2;
	}
	return 0;
}

// 判断一个字符串是否为文件路径，文件夹路径不算
// 非文件路径返回 0
// 绝对文件路径返回 1
// 相对文件路径返回 2
int isFilePath(const char* strPath)
{
	if(!isPath(strPath))
		return 0;

	// 倒序遍历路径，如果先找到 '\' 说明是文件夹路径
	// 如果先找到 '.' 再找到 '\' 说明是文件路径
	bool bPointFlag = false;
	for (int i = strlen(strPath) - 1; i >= 0; i--)
	{
		if (strPath[i] == '.')
		{
			bPointFlag = true;
		}
		else if (strPath[i] == '\\' || strPath[i] == '/')
		{
			// 文件路径
			if (bPointFlag)
			{
				// 绝对路径 E:\file\word.txt
				if (strPath[0] != '.' && strPath[1] == ':')
				{
					return 1;
				}
				// 相对路径 ./xxx.txt ../word.txt word.txt
				else if ((strPath[0] == '.') || (strPath[0] != '.' && strPath[1] != ':'))
				{
					return 2;
				}
			}
			// 文件夹路径
			else
			{
				return 0;
			}
		}
	}

	return 0;
}

// 得到项目进度信息
// 返回是否成功读取，成功返回true，失败返回false
bool GetProgressData(
	const char* p_strDataFilePath,		// in:数据文件位置
	char* p_strProjectName,				// out:项目名
	int p_max_project_name,				// in:项目名最大长度
	char** p_strFilesPath,			// out:项目文件路径集
	int p_max_files_num,				// in:文件数量最大声线
	int p_max_file_path_length,			// in:文件路径长度上限
	int* p_files_num,					// out:读取到的项目文件数量
	ProgressData* p_dataProject,		// out:项目进度数据数组
	int* p_data_index					// out:项目进度数据数
)
{
	FILE* fp;
	if (fopen_s(&fp, p_strDataFilePath, "r"))
	{
		return false;
	}

	char strFirstFile[1024] = { 0 };
	fgets(p_strProjectName, p_max_project_name, fp);
	fgets(strFirstFile, p_max_file_path_length, fp);

	p_strProjectName[strlen(p_strProjectName) - 1] = '\0';
	if (strFirstFile[strlen(strFirstFile) - 1] == '\n')
		strFirstFile[strlen(strFirstFile) - 1] = '\0';

	char strNextLine[1024] = { 0 };
	fgets(strNextLine, 1024, fp);

	fseek(fp, -1 * strlen(strNextLine), SEEK_CUR);
	if (strNextLine[strlen(strNextLine) - 1] == '\n')
	{
		strNextLine[strlen(strNextLine) - 1] = '\0';
		fseek(fp,-1,SEEK_CUR);
	}

	// 使用项目路径必须在下一行写上后缀名要求，可以是 ".*"
	// 若下一行的第一个字符是 '.' 而且不是相对路径，说明是规定后缀名，则使用了项目目录
	if (strNextLine[0] == '.' && !isPath(strNextLine))
	{
		// 读后缀名要求
		char strSuffix[32] = { 0 };
		while (fscanf_s(fp, "%s", strSuffix, 32))
		{
			wstring* wstrFilesList = NULL;
			int nListNum = 0;

			wchar_t wstrFilePath[1024] = { 0 };
			MultiByteToWideChar(CP_ACP, 0, strFirstFile, -1, wstrFilePath, 1024);
			wchar_t wstrSuffix[1024] = { 0 };
			MultiByteToWideChar(CP_ACP, 0, strSuffix, -1, wstrSuffix, 32);
			FindFiles(wstrFilePath, false, true, &wstrFilesList, &nListNum, wstrSuffix);

			// 写入文件路径集
			for (int i = 0; i < nListNum; i++)
			{
				char strProjectFilePath[1024] = { 0 };
				int nSize = WideCharToMultiByte(CP_ACP, 0, wstrFilesList[i].c_str(), -1, NULL, 0, NULL, NULL);
				WideCharToMultiByte(CP_ACP, 0, wstrFilesList[i].c_str(), -1, strProjectFilePath, nSize, NULL, NULL);
				sprintf_s(p_strFilesPath[(*p_files_num) + i], p_max_file_path_length, "%s", strProjectFilePath);
			}

			*p_files_num += nListNum;
			delete[] wstrFilesList;

			// 若后缀名要求已经读完
			char chNext = fgetc(fp);
			fseek(fp, -1, SEEK_CUR);
			if (chNext == '\n' || chNext == EOF)
			{
				// 读去'\n'，为后面读取信息扫清障碍
				fgetc(fp);
				break;
			}
		}

	}

	// 如果下一行是文件路径，即表示需要统计多个指定文件
	else if(isPath(strNextLine))
	{
		sprintf_s(p_strFilesPath[0], p_max_file_path_length, "%s", strFirstFile);
		*p_files_num = 1;

		// 得到接下来所有需要统计的文件的路径
		char strLine[1024] = { 0 };
		for (int i = 1;; i++)
		{
			memset(strLine, 0, 1024);
			fgets(strLine, 1024, fp);

			// 如果这一行不是路径，就结束读取
			if (!isPath(strLine))
			{
				fseek(fp, -1 * strlen(strLine), SEEK_CUR);
				if (strLine[strlen(strLine) - 1] == '\n')
					fseek(fp, -1, SEEK_CUR);
				break;
			}

			if (strLine[strlen(strLine) - 1] == '\n')
				strLine[strlen(strLine) - 1] = '\0';

			sprintf_s(p_strFilesPath[i], p_max_file_path_length, "%s", strLine);
			(*p_files_num)++;
		}
	}

	// 除去以上两种情况，下一行就只可能是项目进度数据了
	// 由于至少要统计一个文件，所以 strFirstFile 一定是文件路径，则将其写入路径集
	else
	{
		// 如果至少写入了一个文件路径，strFirstFile 就一定是文件路径
		// 否则说明文件出错
		if (!isPath(strFirstFile))
		{
			fclose(fp);
			return false;
		}

		sprintf_s(p_strFilesPath[0], p_max_file_path_length, "%s", strFirstFile);
		*p_files_num = 1;
	}

	// 读取项目进度数据
	char strDate[6] = { 0 };
	int line = 0;
	int length = 0;
	while (fscanf_s(fp, "%s %d %d\n", strDate, 6, &line, &length) != EOF)
	{
		int m = 0, d = 0;
		char strMonth[3] = { 0 };
		char strDay[3] = { 0 };
		int flag_index = 0;
		for (int i = 0; i < (int)strlen(strDate); i++)
		{
			if (!flag_index)
			{
				if (strDate[i] != '.')
				{
					strMonth[i] = strDate[i];
				}
				else
				{
					flag_index = i;
				}
			}
			else
			{
				strDay[i - flag_index - 1] = strDate[i];
			}
		}
		m = atoi(strMonth);
		d = atoi(strDay);

		p_dataProject[*p_data_index] = { m,d,line,length };
		(*p_data_index)++;
	}

	fclose(fp);
	return true;
}

// 得到文件列表中所有文件的信息
// 返回第一个读取失败的文件在文件列表中的下标，所有文件读取成功时返回0
int GetFileData(char** p_strFilesPath, int p_files_num, int* p_line, int* p_length)
{
	*p_line = 0;
	*p_length = 0;
	for (int i = 0; i < p_files_num; i++)
	{
		FILE* fp = NULL;
		if (fopen_s(&fp, p_strFilesPath[i], "r"))
		{
			return i;
		}

		(*p_length) += _filelength(_fileno(fp));

		char* buf = new char[(*p_length) + 1];
		memset(buf, 0, (*p_length) + 1);
		fread_s(buf, (*p_length) + 1, *p_length, 1, fp);

		int line = 0;
		for (int i = 0; i < (int)strlen(buf); i++)
			if (buf[i] == '\n')
				line++;
		(*p_line) += line;

		fclose(fp);
		delete[] buf;
	}

	return 0;
}

// 更新项目进度文件信息
// 返回是否成功更新，成功返回true，失败返回false
bool UpdateDataFile(const char* p_strDataFilesPath, ProgressData* p_dataProject, int p_data_index, int p_month, int p_day, int p_line, int p_length)
{
	FILE* fp;
	if (fopen_s(&fp, p_strDataFilesPath, "rt+"))
	{
		return false;
	}

	char strLine[128] = { 0 };
	sprintf_s(strLine, 128, "%d.%d %d %d", p_month, p_day, p_line, p_length);

	// 如果最后一条的日期和当前日期一致，覆盖最后一条内容
	if (p_dataProject[p_data_index - 1].month == p_month && p_dataProject[p_data_index - 1].day == p_day)
	{
		int nDataFileLength = _filelength(_fileno(fp));
		const int nAddStringLength = 1024;
		char* buf = new char[nDataFileLength + nAddStringLength];
		memset(buf, 0, nDataFileLength + nAddStringLength);
		fread_s(buf, nDataFileLength + nAddStringLength, nDataFileLength, 1, fp);
		
		for (int i = strlen(buf) - 2; i > 0; i--)
		{
			if (buf[i] == '\n')
			{
				buf[i] = '\0';
				break;
			}
		}

		sprintf_s(buf, nDataFileLength + nAddStringLength, "%s\n%s", buf, strLine);

		// 重新打开文件，以wt+模式擦除文件内容
		fclose(fp);
		if (fopen_s(&fp, p_strDataFilesPath, "wt+"))
		{
			return false;
		}

		fputs(buf, fp);

		delete[] buf;
		buf = NULL;
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		fputs("\n", fp);
		fputs(strLine, fp);
	}

	fclose(fp);
	return true;
}

// 得到代码长度贡献等级要求
// level 等级，1~6
// 返回该等级的码量要求，获取信息失败返回-1
int GetLengthContributionLevel(int level)
{
	wchar_t wstr[12] = { 0 };
	wsprintf(wstr, L"level_%d", level);
	return (int)GetPrivateProfileInt(L"update_length", wstr, -1, wstrSettingsFilePath);
}

// 得到每行平均码量限制
// mode 模式。为true表示得到最大值，false表示得到最小值
// 返回传参模式对应的码量限制值，读取信息失败返回-1
int GetWordsNumOfLineLimit(bool mode)
{
	if (mode)
		return (int)GetPrivateProfileInt(L"update_line", L"max", -1, wstrSettingsFilePath);
	else
		return (int)GetPrivateProfileInt(L"update_line", L"min", -1, wstrSettingsFilePath);
}

// 显示图表比较
void ShowGraphComparison(const char* p_strProjectName, ProgressData* p_dataProject, int p_data_index)
{
	initgraph(640, 480);
	setbkcolor(WHITE);
	setbkmode(TRANSPARENT);
	cleardevice();

	// 贡献量
	struct Contribution
	{
		int update_line;
		int update_length;
	};

	// 每日贡献量，下标为第几天，具体对应日期在 p_dataProject 中查找
	Contribution* pContribution = new Contribution[p_data_index];
	memset(pContribution, 0, sizeof(Contribution) * p_data_index);

	// 计算每日贡献
	for (int i = 0; i < p_data_index; i++)
	{
		if (i - 1 >= 0)
		{
			pContribution[i].update_line = p_dataProject[i].line - p_dataProject[i - 1].line;
			pContribution[i].update_length = p_dataProject[i].length - p_dataProject[i - 1].length;
		}
		else
		{
			pContribution[i].update_line = p_dataProject[i].line;
			pContribution[i].update_length = p_dataProject[i].length;
		}
	}

	// 各项数据最大值
	int max_line = 0, max_length = 0;
	int max_update_line = 0, max_update_length = 0;

	// 计算各项数据最大值
	for (int i = 0; i < p_data_index; i++)
	{
		if (p_dataProject[i].line > max_line)
			max_line = p_dataProject[i].line;
		if (p_dataProject[i].length > max_length)
			max_length = p_dataProject[i].length;
		if (pContribution[i].update_line > max_update_line)
			max_update_line = pContribution[i].update_line;
		if (pContribution[i].update_length > max_update_length)
			max_update_length = pContribution[i].update_length;
	}

	// 左上角显示项目名称
	settextcolor(BLUE);
	settextstyle(20, 0, L"consolas", 0, 0, 1000, false, false, false);
	outtextxy(20, 20, L"Project:");

	wchar_t wstrProjectName[1024] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, p_strProjectName, -1, wstrProjectName, 1024);

	settextcolor(BLACK);
	settextstyle(20, 0, L"consolas", 0, 0, 0, false, false, false);
	outtextxy(100, 20, wstrProjectName);

	// 右上角显示制作者信息
	wchar_t wstrVersion[32] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, strVersion, -1, wstrVersion, 32);
	wchar_t wstrReleaseTime[32] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, strReleaseTime, -1, wstrReleaseTime, 32);
	wchar_t wstrIntroduceMaker[1024] = { 0 };
	wsprintf(wstrIntroduceMaker, L"Project progress manager %s made by huidong <mailkey@yeah.net> in %s", wstrVersion, wstrReleaseTime);
	settextstyle(12, 0, L"consolas");
	outtextxy(getwidth() - textwidth(wstrIntroduceMaker), getheight() - textheight(wstrIntroduceMaker), wstrIntroduceMaker);

	// 绘制图表

	int bottom_line_height = 20;	// 底部线条高度
	int each_day_data_width = /*70*/100;	// 每天的数据的占宽
	int histogram_height = 200;		// 柱状图最大占高
	int line_chart_height = 120;		// 折线图最大占高（柱状图和折线图不重叠）
	double histogram_line_white_precent = 0.2;		// 柱状图的代码行数柱体空白部分占比
	double histogram_length_white_precent = 0.1;	// 柱状图的代码字数柱体空白部分占比
	double line_chart_white_precent = 0.4;			// 折线图上半部分空白占比
	int line_chart_line_up_offset = 25;		// 折线图的代码行数图像向上偏移像素
	int line_chart_length_up_offset = 0;	// 折线图的代码字数图像向上偏移像素
	COLORREF histogram_line_color = BLUE;		// 柱状图的代码行数柱体填充颜色
	COLORREF histogram_length_color = YELLOW;	// 柱状图的代码字数柱体填充颜色
	int histogram_line_width = /*25*/40;		// 柱状图的代码行数柱体宽度
	int histogram_length_width = /*25*/40;	// 柱状图的代码字数柱体宽度
	COLORREF line_chart_line_color = BLUE;					// 折线图的代码行数线条颜色
	COLORREF line_chart_length_color = RGB(240, 160, 0);	// 折线图的代码字数线条颜色
	int line_chart_circle_r = 4;		// 折线图标点的半径

	// 图表内容image
	IMAGE img(each_day_data_width * p_data_index, histogram_height + line_chart_height + bottom_line_height);
	SetWorkingImage(&img);
	setbkcolor(WHITE);
	cleardevice();
	setbkmode(TRANSPARENT);

	// 亮灰色，底部一条线
	setlinecolor(LIGHTGRAY);
	line(0, getheight() - bottom_line_height, getwidth(), getheight() - bottom_line_height);

	// 上一次的折线图的点
	POINT pointOldLine = { 0 };
	POINT pointOldLength = { 0 };

	for (int i = 0; i < p_data_index; i++)
	{
		// 每一天的数据背景颜色取决于贡献量
		if (pContribution[i].update_length > GetLengthContributionLevel(1))
		{
			setfillcolor(RGB(0, 255, 0));
		}
		else if (pContribution[i].update_length > GetLengthContributionLevel(2))
		{
			setfillcolor(RGB(100, 255, 0));
		}
		else if (pContribution[i].update_length > GetLengthContributionLevel(3))
		{
			setfillcolor(RGB(150, 255, 0));
		}
		else if (pContribution[i].update_length > GetLengthContributionLevel(4))
		{
			setfillcolor(RGB(255, 255, 0));
		}
		else if (pContribution[i].update_length > GetLengthContributionLevel(5))
		{
			setfillcolor(RGB(255, 220, 120));
		}
		else if (pContribution[i].update_length > GetLengthContributionLevel(6))
		{
			setfillcolor(RGB(255, 50, 0));
		}
		else
		{
			setfillcolor(RGB(255, 0, 0));
		}

		setlinecolor(getfillcolor());
		fillrectangle(each_day_data_width * i, 0, each_day_data_width * (i + 1), getheight() - bottom_line_height);

		// 字体：小号、灰色、斜体
		settextcolor(LIGHTGRAY);
		settextstyle(17, 0, L"consolas", 0, 0, 1000, false, false, false);
		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);

		// 在线条下输出日期
		wchar_t date[6] = { 0 };
		wsprintf(date, L"%d.%d", p_dataProject[i].month, p_dataProject[i].day);
		outtextxy(each_day_data_width * i + 5, getheight() - bottom_line_height + 2, date);

		// 绘制柱状图
		int line_height = (int)(((double)(histogram_height * (1 - histogram_line_white_precent)) / (double)max_line) * p_dataProject[i].line);
		int length_height = (int)(((double)(histogram_height * (1 - histogram_length_white_precent)) / (double)max_length) * p_dataProject[i].length);
		int histogram_white = (int)((each_day_data_width - histogram_line_width - histogram_length_width) / 2);	// 柱体两侧留空大小

		setfillcolor(histogram_line_color);
		fillrectangle(
			each_day_data_width * i + histogram_white,
			getheight() - bottom_line_height - line_height,
			each_day_data_width * i + histogram_white + histogram_line_width,
			getheight() - bottom_line_height
		);

		setfillcolor(histogram_length_color);
		fillrectangle(
			each_day_data_width * i + histogram_white + histogram_line_width,
			getheight() - bottom_line_height - length_height,
			each_day_data_width * i + histogram_white + histogram_line_width + histogram_length_width,
			getheight() - bottom_line_height
		);

		// 在柱状图上显示具体数据
		wchar_t wstrLine[8] = { 0 };
		wchar_t wstrLength[16] = { 0 };
		wsprintf(wstrLine, L"%d", p_dataProject[i].line);
		wsprintf(wstrLength, L"%d", p_dataProject[i].length);

		settextstyle(13, 0, L"consolas");
		settextcolor(histogram_line_color);
		outtextxy(each_day_data_width * i + histogram_white, getheight() - bottom_line_height - line_height - textheight('0'), wstrLine);
		// 注意：此处输出文字用了折线图的颜色，因为柱形图的颜色输出的文字看不清
		settextcolor(line_chart_length_color);
		outtextxy(each_day_data_width * i + histogram_white + histogram_line_width, getheight() - bottom_line_height - length_height - textheight('0'), wstrLength);

		// 画折线图
		int update_line_height = getheight() - bottom_line_height - histogram_height - (int)((double)((double)(line_chart_height * (1 - line_chart_white_precent)) / (double)max_update_line) * pContribution[i].update_line);
		int update_length_height = getheight() - bottom_line_height - histogram_height - (int)((double)((double)(line_chart_height * (1 - line_chart_white_precent)) / (double)max_update_length) * pContribution[i].update_length);

		POINT point_line = { each_day_data_width * i + each_day_data_width / 2,update_line_height - line_chart_line_up_offset };
		POINT point_length = { each_day_data_width * i + each_day_data_width / 2,update_length_height - line_chart_length_up_offset };

		// 标点
		setlinecolor(line_chart_line_color);
		setfillcolor(line_chart_line_color);
		fillcircle(point_line.x, point_line.y, line_chart_circle_r);
		setlinecolor(line_chart_length_color);
		setfillcolor(line_chart_length_color);
		fillcircle(point_length.x, point_length.y, line_chart_circle_r);

		// 连线
		if (i > 0)
		{
			setlinestyle(PS_SOLID, 2);
			setlinecolor(line_chart_line_color);
			line(pointOldLine.x, pointOldLine.y, point_line.x, point_line.y);
			setlinecolor(line_chart_length_color);
			line(pointOldLength.x, pointOldLength.y, point_length.x, point_length.y);
		}

		// 记录旧点
		pointOldLine = point_line;
		pointOldLength = point_length;

		// 标记数据
		wchar_t wstrUpdateLine[8] = { 0 };
		wchar_t wstrUpdateLength[16] = { 0 };
		wsprintf(wstrUpdateLine, L"%d", pContribution[i].update_line);
		wsprintf(wstrUpdateLength, L"%d", pContribution[i].update_length);

		settextstyle(14, 0, L"consolas", 0, 0, 0, false, false, false);

		int line_text_y = point_line.y - textheight(wstrUpdateLine) * 3 / 2;
		settextcolor(line_chart_line_color);
		// 防文字重叠判断
		if (point_line.y >= point_length.y && line_text_y < point_length.y)
		{
			outtextxy(point_line.x - textwidth(wstrUpdateLine) / 2, point_line.y + textheight(wstrUpdateLine) / 2, wstrUpdateLine);
		}
		else
		{
			outtextxy(point_line.x - textwidth(wstrUpdateLine) / 2, line_text_y, wstrUpdateLine);
		}

		int length_text_y = point_length.y - textheight(wstrUpdateLength) * 3 / 2;
		settextcolor(line_chart_length_color);
		// 防文字重叠判断
		if (point_length.y > point_line.y && length_text_y < point_line.y)
		{
			outtextxy(point_length.x - textwidth(wstrUpdateLength) / 2, point_length.y + textheight(wstrUpdateLength) / 2, wstrUpdateLength);
		}
		else
		{
			outtextxy(point_length.x - textwidth(wstrUpdateLength) / 2, length_text_y, wstrUpdateLength);
		}
	}

	SetWorkingImage();

	// 输出今日贡献

	COLORREF colorContribution = RED;		// 显示贡献值的颜色
	COLORREF colorTip = BLACK;				// 第一提示语的颜色
	COLORREF colorWordsNumOfLine = WHITE;	// 每行平均码量对应的颜色

	// 平均每行的字符数
	int nWordsNumOfLine = 0;
	if (pContribution[p_data_index - 1].update_line != 0)
	{
		nWordsNumOfLine = (int)(pContribution[p_data_index - 1].update_length / pContribution[p_data_index - 1].update_line);

		// 每行平均小于1个字节
		if (nWordsNumOfLine == 0)
			nWordsNumOfLine = 1;
		// 负增长
		if(pContribution[p_data_index - 1].update_length < 0 || pContribution[p_data_index - 1].update_line < 0)
			nWordsNumOfLine = 0;
	}

	// 提示语
	wchar_t wstrTip[1024] = { 0 };					// 第一提示语
	wchar_t wstrWordsNumOfLineTip[1024] = { 0 };	// 每行平均码量提示语

	// 根据今日贡献确定文字颜色，提示文本
	if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(1))
	{
		colorContribution = RGB(0, 255, 0);
		wsprintf(wstrTip, L"码神就是我。");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(2))
	{
		colorContribution = RGB(50, 200, 0);
		wsprintf(wstrTip, L"今天思路清晰，“码”不停蹄！");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(3))
	{
		colorContribution = RGB(100, 190, 0);
		wsprintf(wstrTip, L"进入状态，再码百行~");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(4))
	{
		colorContribution = RGB(150, 150, 0);
		wsprintf(wstrTip, L"刚才只是热身，现在才刚刚开始。");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(5))
	{
		colorContribution = RGB(200, 120, 0);
		wsprintf(wstrTip, L"码量也忒少了点，赶紧干活。");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(6))
	{
		colorContribution = RGB(255, 100, 0);
		wsprintf(wstrTip, L"码量甚是寒碜。");
	}
	else
	{
		colorContribution = RGB(255, 0, 0);
		wsprintf(wstrTip, L"千里之行，始于足下。开工！");
	}

	// 明显在水行
	if (nWordsNumOfLine < GetWordsNumOfLineLimit(false) && nWordsNumOfLine != 0)
	{
		if (pContribution[p_data_index - 1].update_length > (int)GetPrivateProfileInt(L"update_length", L"level_3", 0, wstrSettingsFilePath))
		{
			wsprintf(wstrWordsNumOfLineTip, L"咱这行数是多了点，是因为咱们的代码结构比较清晰。");
		}
		else
		{
			wsprintf(wstrWordsNumOfLineTip, L"咳咳，咱这代码……是不是……写的太散了点？");
		}

		colorWordsNumOfLine = RED;
	}
	// 每行的内容过多
	else if (nWordsNumOfLine > GetWordsNumOfLineLimit(true))
	{
		wsprintf(wstrWordsNumOfLineTip, L"哇，您的代码真是太“浓”了。");
		colorWordsNumOfLine = RGB(100, 0, 200);
	}
	// 每行平均码量适宜
	else if (nWordsNumOfLine != 0)
	{
		wsprintf(wstrWordsNumOfLineTip, L"您的代码真不错。");
		colorWordsNumOfLine = GREEN;
	}

	wchar_t wstrText[1024] = { 0 };
	wchar_t wstrWordsNumOfLineText[1024] = { 0 };
	wsprintf(wstrText, L"%d 行  %d 字节", pContribution[p_data_index - 1].update_line, pContribution[p_data_index - 1].update_length);
	wsprintf(wstrWordsNumOfLineText, L"%d 字节/行", nWordsNumOfLine);

	setfillcolor(BLACK);
	fillrectangle(60, 50, 320, 110);

	settextstyle(20, 0, L"微软雅黑");
	settextcolor(WHITE);
	outtextxy(67, 58, L"今日代码贡献：");

	settextcolor(colorContribution);
	outtextxy(80, 80, wstrText);

	settextcolor(colorWordsNumOfLine);
	outtextxy(235, 80, wstrWordsNumOfLineText);

	settextstyle(25, 0, L"微软雅黑");
	settextcolor(colorTip);
	outtextxy(330, 55, wstrTip);

	settextcolor(colorWordsNumOfLine);
	settextstyle(18, 0, L"微软雅黑");
	outtextxy(330, 90, wstrWordsNumOfLineTip);

	// 输出图表
	int nPutImageX = 70;	// 输出图像x坐标
	int nPutImageY = 130;	// 输出图像y坐标
	int nPutImageWidth = 500;	// 输出图像宽度
	int nStartX = img.getwidth() - nPutImageWidth;	// 裁剪图像的x起始坐标
	putimage(nPutImageX, nPutImageY, nPutImageWidth, img.getheight(), &img, nStartX, 0);

	// 绘制按钮
	setlinecolor(BLACK);
	settextcolor(BLACK);
	settextstyle(20, 0, L"system");
	rectangle(20, 270, 50, 300);
	outtextxy(30, 280, L'<');
	rectangle(590, 270, 620, 300);
	outtextxy(600, 280, L'>');

	// 绘制图表解释
	settextstyle(16, 0, L"consolas");
	setlinecolor(line_chart_line_color);
	setfillcolor(line_chart_line_color);
	fillcircle(85, 120, line_chart_circle_r);
	outtextxy(93, 112, L"相对增长行数");

	setlinecolor(line_chart_length_color);
	setfillcolor(line_chart_length_color);
	fillcircle(210, 120, line_chart_circle_r);
	outtextxy(218, 112, L"相对增长码量");

	setlinecolor(BLACK);
	setfillcolor(histogram_line_color);
	fillrectangle(330, 116, 338, 124);
	outtextxy(342, 112, L"总代码行数");

	setlinecolor(BLACK);
	setfillcolor(histogram_length_color);
	fillrectangle(435, 116, 443, 124);
	outtextxy(447, 112, L"总代码字节数");

	// 响应鼠标消息
	int time = 0;
	while (true)
	{
		MOUSEMSG msg;
		if (time == 0)
		{
			msg = GetMouseMsg();
		}
		else
		{
			PeekMouseMsg(&msg);
		}

		// 按钮长按支持
		if (msg.mkLButton)
		{
			if (time == 0)
			{
				time = clock();
				continue;
			}
			else
			{
				if (clock() - time < 600)
				{
					continue;
				}
			}
		}
		else
		{
			time = 0;
			if (msg.uMsg != WM_LBUTTONUP)
			{
				continue;
			}
		}

		if (msg.x > 20 && msg.x < 50 && msg.y > 270 && msg.y < 300)
		{
			if (nStartX - 10 < 0)
				nStartX = 0;
			else
				nStartX -= 10;
		}
		else if (msg.x > 590 && msg.x < 620 && msg.y >270 && msg.y < 300)
		{
			if (nStartX + 10 > img.getwidth() - 500)
				nStartX = img.getwidth() - 500;
			else
				nStartX += 10;
		}

		// 清除图像
		BeginBatchDraw();
		clearrectangle(nPutImageX, nPutImageY, nPutImageX + nPutImageWidth, img.getheight() + nPutImageY);
		putimage(nPutImageX, nPutImageY, nPutImageWidth, img.getheight(), &img, nStartX, 0);
		EndBatchDraw();

		// 防止长按导致的移动过快
		Sleep(100);
	}

	closegraph();
	delete[] pContribution;
}

int main()
{
	printf("Project progress manager %s by huidong.\n", strVersion);
	printf("Loading");

	ProgressData* dataProject = new ProgressData[1024];
	memset(dataProject, 0, sizeof(ProgressData) * 1024);
	int nDataIndex = 0;
	char strProjectName[1024] = { 0 };
	char** strFilesPath = NULL;
	int nFilesNum = 0;
	int nTotalLine = 0;
	int nTotalLength = 0;
	int nThisMonth = 0;
	int nThisDay = 0;

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	nThisMonth = sys.wMonth;
	nThisDay = sys.wDay;

	// 文件路径集
	const int nMaxFilesNum = 10240;
	strFilesPath = new char* [nMaxFilesNum];
	for (int i = 0; i < nMaxFilesNum; i++)
	{
		strFilesPath[i] = new char[1024];
		memset(strFilesPath[i], 0, 1024);
	}

	printf(".");

	if (!GetProgressData(strDataFilePath, strProjectName, 1024, strFilesPath, 10240, 1024, &nFilesNum, dataProject, &nDataIndex))
	{
		printf("error get data form \"project.dat\".\n");
		system("pause");
	}

	printf(".");

	int nErrorFileId = GetFileData(strFilesPath, nFilesNum, &nTotalLine, &nTotalLength);
	if (nErrorFileId != 0)
	{
		printf("error get data form \"%s\".\n", strFilesPath[nErrorFileId]);
		system("pause");
	}

	printf(".");

	// update data
	if (!UpdateDataFile(strDataFilePath, dataProject, nDataIndex, nThisMonth, nThisDay, nTotalLine, nTotalLength))
	{
		printf("error update data.\n");
		system("pause");
	}

	printf(".");

	// 更新今日数据
	if (!(dataProject[nDataIndex - 1].month == nThisMonth && dataProject[nDataIndex - 1].day == nThisDay))
		nDataIndex++;
	dataProject[nDataIndex - 1] = { nThisMonth,nThisDay,nTotalLine,nTotalLength };

	printf(".");

	// show graph
	ShowGraphComparison(strProjectName, dataProject, nDataIndex);

	delete[] dataProject;
	dataProject = NULL;

	for (int i = 0; i < nMaxFilesNum; i++)
		delete[] strFilesPath[i];
	delete[] strFilesPath;
	strFilesPath = NULL;

	return 0;
}

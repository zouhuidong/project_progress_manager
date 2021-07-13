/*
	project_progress_manager
	main.cpp
	项目进度管理器

	所需文件要求：
	1. 程序同目录下必须存在project.dat（存储项目进度数据），settings.ini（配置文件）
	2. project.dat 文件格式要求
		第一行：项目名
		第二行：项目代码文件路径，如果有多个就每个写一行。如果要包含一个目录，就在此行写目录路径，
			并在下一行写要纳入代码统计的文件的后缀名（不带点 '.'），每个后缀名之间留一个空格，'*' 表示所有后缀名。
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
		c cpp h html js css php
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
#include <io.h>

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

// 得到项目进度信息
// 返回是否成功读取，成功返回true，失败返回false
bool GetProgressData(
	const char* p_strDataFilePath,		// in:数据文件位置
	char* p_strProjectName,				// out:项目名
	size_t p_sizeMaxName,				// in:项目名最大长度
	char* p_strFilePath,				// out:项目文件路径
	size_t p_sizeMaxPath,				// in:项目文件路径最大长度
	ProgressData* p_dataProject,		// out:项目进度数据数组
	int* p_data_index					// out:项目进度数据数
)
{
	FILE* fp;
	if (fopen_s(&fp, p_strDataFilePath, "r"))
	{
		return false;
	}

	fgets(p_strProjectName, p_sizeMaxName, fp);
	fgets(p_strFilePath, p_sizeMaxPath, fp);

	// delete '\n'
	p_strProjectName[strlen(p_strProjectName) - 1] = '\0';
	if (p_strFilePath[strlen(p_strFilePath) - 1] == '\n')	// 这个结尾未必是\n
		p_strFilePath[strlen(p_strFilePath) - 1] = '\0';

	char strDate[6] = { 0 };
	int line = 0;
	int length = 0;

	// 得到每行数据
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

// 得到文件信息
// 返回是否成功读取，成功返回true，失败返回false
bool GetFileData(const char* p_strFilePath, int* p_line, int* p_length)
{
	FILE* fp;
	if (fopen_s(&fp, p_strFilePath, "r"))
	{
		return false;
	}

	*p_length = _filelength(_fileno(fp));

	char* buf = new char[10240000];
	memset(buf, 0, 10240000);
	fread_s(buf, 10240000, *p_length, 1, fp);

	int line = 0;
	for (int i = 0; i < (int)strlen(buf); i++)
		if (buf[i] == '\n')
			line++;
	*p_line = line;

	fclose(fp);
	delete[] buf;
	return true;
}

// 更新文件信息
// 返回是否成功更新，成功返回true，失败返回false
bool UpdateData(const char* p_strDataFilePath, ProgressData* p_dataProject, int p_data_index, int p_month, int p_day, int p_line, int p_length)
{
	FILE* fp;
	if (fopen_s(&fp, p_strDataFilePath, "rt+"))
	{
		return false;
	}

	char strLine[128] = { 0 };
	sprintf_s(strLine, 128, "%d.%d %d %d", p_month, p_day, p_line, p_length);

	// 如果最后一条的日期和当前日期一致，覆盖最后一条内容
	if (p_dataProject[p_data_index - 1].month == p_month && p_dataProject[p_data_index - 1].day == p_day)
	{
		char* buf = new char[10240];
		memset(buf, 0, 10240);
		fread_s(buf, 10240, 10240, 1, fp);

		for (int i = strlen(buf) - 2; i > 0; i--)
		{
			if (buf[i] == '\n')
			{
				buf[i] = '\0';
				break;
			}
		}

		sprintf_s(buf, 10240, "%s\n%s", buf, strLine);

		// 重新打开文件，以wt+模式擦除文件内容
		fclose(fp);
		if (fopen_s(&fp, p_strDataFilePath, "wt+"))
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
	if(mode)
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
	const wchar_t* wstrIntroduceMaker = L"ProjectProgressManager(simple version) made by huidong <mailkey@yeah.net> in 2021.7.11";
	settextstyle(12, 0, L"consolas");
	outtextxy(getwidth() - textwidth(wstrIntroduceMaker), getheight() - textheight(wstrIntroduceMaker), wstrIntroduceMaker);

	// 绘制图表

	int bottom_line_height = 20;	// 底部线条高度
	int each_day_data_width = 70;	// 每天的数据的占宽
	int histogram_height = 200;		// 柱状图最大占高
	int line_chart_height = 120;		// 折线图最大占高（柱状图和折线图不重叠）
	double histogram_line_white_precent = 0.2;		// 柱状图的代码行数柱体空白部分占比
	double histogram_length_white_precent = 0.1;	// 柱状图的代码字数柱体空白部分占比
	double line_chart_white_precent = 0.4;			// 折线图上半部分空白占比
	int line_chart_line_up_offset = 25;		// 折线图的代码行数图像向上偏移像素
	int line_chart_length_up_offset = 0;	// 折线图的代码字数图像向上偏移像素
	COLORREF histogram_line_color = BLUE;		// 柱状图的代码行数柱体填充颜色
	COLORREF histogram_length_color = YELLOW;	// 柱状图的代码字数柱体填充颜色
	int histogram_line_width = 25;		// 柱状图的代码行数柱体宽度
	int histogram_length_width = 25;	// 柱状图的代码字数柱体宽度
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
		if (pContribution[i].update_length > GetLengthContributionLevel(2))
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
		fillrectangle(each_day_data_width* i, 0, each_day_data_width* (i + 1), getheight() - bottom_line_height);

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
		wsprintf(wstrTip, L"今天我思维活跃，还可以继续码！");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(3))
	{
		colorContribution = RGB(100, 190, 0);
		wsprintf(wstrTip, L"码这点对我来说只是小菜一碟~");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(4))
	{
		colorContribution = RGB(150, 150, 0);
		wsprintf(wstrTip, L"再接再厉，继续敲码！");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(5))
	{
		colorContribution = RGB(200, 120, 0);
		wsprintf(wstrTip, L"码量也忒少了点，赶紧干活。");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(6))
	{
		colorContribution = RGB(255, 100, 0);
		wsprintf(wstrTip, L"今天这寒碜的码量是因为我还没开工呢。");
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
		wsprintf(wstrWordsNumOfLineTip, L"哇，您的代码真是太浓了。");
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
	wsprintf(wstrWordsNumOfLineText, L"平均 %d 字节/行", nWordsNumOfLine);

	setfillcolor(BLACK);
	fillrectangle(60, 50, 320, 110);

	settextstyle(20, 0, L"微软雅黑");
	settextcolor(WHITE);
	outtextxy(67, 58, L"今日代码贡献：");

	settextcolor(colorContribution);
	outtextxy(80, 80, wstrText);

	settextcolor(colorWordsNumOfLine);
	outtextxy(205, 80, wstrWordsNumOfLineText);

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
	fillrectangle(330,116,338,124);
	outtextxy(342,112,L"总代码行数");

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
	ProgressData* dataProject = new ProgressData[1024];
	memset(dataProject, 0, sizeof(ProgressData) * 1024);
	int nDataIndex = 0;
	char strProjectName[1024] = { 0 };
	char strFilePath[1024] = { 0 };
	int nTodayLine = 0;
	int nTodayLength = 0;
	int nThisMonth = 0;
	int nThisDay = 0;

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	nThisMonth = sys.wMonth;
	nThisDay = sys.wDay;

	if (!GetProgressData(strDataFilePath, strProjectName, 1024, strFilePath, 1024, dataProject, &nDataIndex))
	{
		printf("error get data form project.dat\n");
		system("pause");
	}
	if (!GetFileData(strFilePath, &nTodayLine, &nTodayLength))
	{
		printf("error get data form \"%s\"\n", strFilePath);
		system("pause");
	}

	// update data
	UpdateData(strDataFilePath, dataProject, nDataIndex, nThisMonth, nThisDay, nTodayLine, nTodayLength);

	// 更新数据后重新再读一次，更新今日数据
	memset(dataProject, 0, sizeof(ProgressData) * 1024);
	nDataIndex = 0;
	if (!GetProgressData(strDataFilePath, strProjectName, 1024, strFilePath, 1024, dataProject, &nDataIndex))
	{
		printf("error get data form project.dat\n");
		system("pause");
	}

	// show graph
	ShowGraphComparison(strProjectName, dataProject, nDataIndex);

	delete[] dataProject;
	dataProject = NULL;
	return 0;
}

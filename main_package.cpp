/*
	project_progress_manager
	main.cpp
	��Ŀ���ȹ�����

	�����ļ�Ҫ��
	1. ����ͬĿ¼�±������project.dat���洢��Ŀ�������ݣ���settings.ini�������ļ���
	2. project.dat �ļ���ʽҪ��
		��һ�У���Ŀ��
		�ڶ��У���Ŀ�����ļ�·��������ж����ÿ��дһ�С����Ҫ����һ��Ŀ¼�����ڴ���дĿ¼·����
			������һ��дҪ�������ͳ�Ƶ��ļ��ĺ�׺���������� '.'����ÿ����׺��֮����һ���ո�'*' ��ʾ���к�׺����
		�����У�ʱ�䣨��ʽ����.�գ���λ���������貹0Ϊ��λ���� ������������λ�У� ���볤�ȣ���λ�ֽڣ�
		��n���Ե���������
		ע�⣺�ļ��в��ú��ж���Ŀո�ͻ���

		�ļ�ʾ��1��
		testproject ver1.1
		D:\myproject\testproject\main.cpp
		7.10 100 900
		7.11 233 2562

		�ļ�ʾ��2��
		testproject ver1.1
		..\main.cpp
		..\png.cpp
		7.10 100 900
		7.11 233 2562

		�ļ�ʾ��3��
		testproject ver1.1
		D:\projects\testproject\
		c cpp h html js css php
		7.10 100 900
		7.11 233 2562

	3. settings.ini �ļ���ʽҪ������ļ���ע�͡�

	�����ߣ�huidong <mailkey@yeah.net>
	����ʱ�䣺2021.7.10
	����޸ģ�2021.7.12
*/


#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <io.h>

// �ļ�·��
const char strDataFilePath[] = "./project.dat";				// ��Ŀ�����ļ�
const wchar_t* wstrSettingsFilePath = L"./settings.ini";	// �����ļ�

// ������Ϣ
struct ProgressData
{
	int month;	// ��
	int day;	// ��
	int line;	// ��������
	int length;	// ���볤��
};

// �õ���Ŀ������Ϣ
// �����Ƿ�ɹ���ȡ���ɹ�����true��ʧ�ܷ���false
bool GetProgressData(
	const char* p_strDataFilePath,		// in:�����ļ�λ��
	char* p_strProjectName,				// out:��Ŀ��
	size_t p_sizeMaxName,				// in:��Ŀ����󳤶�
	char* p_strFilePath,				// out:��Ŀ�ļ�·��
	size_t p_sizeMaxPath,				// in:��Ŀ�ļ�·����󳤶�
	ProgressData* p_dataProject,		// out:��Ŀ������������
	int* p_data_index					// out:��Ŀ����������
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
	if (p_strFilePath[strlen(p_strFilePath) - 1] == '\n')	// �����βδ����\n
		p_strFilePath[strlen(p_strFilePath) - 1] = '\0';

	char strDate[6] = { 0 };
	int line = 0;
	int length = 0;

	// �õ�ÿ������
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

// �õ��ļ���Ϣ
// �����Ƿ�ɹ���ȡ���ɹ�����true��ʧ�ܷ���false
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

// �����ļ���Ϣ
// �����Ƿ�ɹ����£��ɹ�����true��ʧ�ܷ���false
bool UpdateData(const char* p_strDataFilePath, ProgressData* p_dataProject, int p_data_index, int p_month, int p_day, int p_line, int p_length)
{
	FILE* fp;
	if (fopen_s(&fp, p_strDataFilePath, "rt+"))
	{
		return false;
	}

	char strLine[128] = { 0 };
	sprintf_s(strLine, 128, "%d.%d %d %d", p_month, p_day, p_line, p_length);

	// ������һ�������ں͵�ǰ����һ�£��������һ������
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

		// ���´��ļ�����wt+ģʽ�����ļ�����
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

// �õ����볤�ȹ��׵ȼ�Ҫ��
// level �ȼ���1~6
// ���ظõȼ�������Ҫ�󣬻�ȡ��Ϣʧ�ܷ���-1
int GetLengthContributionLevel(int level)
{
	wchar_t wstr[12] = { 0 };
	wsprintf(wstr, L"level_%d", level);
	return (int)GetPrivateProfileInt(L"update_length", wstr, -1, wstrSettingsFilePath);
}

// �õ�ÿ��ƽ����������
// mode ģʽ��Ϊtrue��ʾ�õ����ֵ��false��ʾ�õ���Сֵ
// ���ش���ģʽ��Ӧ����������ֵ����ȡ��Ϣʧ�ܷ���-1
int GetWordsNumOfLineLimit(bool mode)
{
	if(mode)
		return (int)GetPrivateProfileInt(L"update_line", L"max", -1, wstrSettingsFilePath);
	else
		return (int)GetPrivateProfileInt(L"update_line", L"min", -1, wstrSettingsFilePath);
}

// ��ʾͼ��Ƚ�
void ShowGraphComparison(const char* p_strProjectName, ProgressData* p_dataProject, int p_data_index)
{
	initgraph(640, 480);
	setbkcolor(WHITE);
	setbkmode(TRANSPARENT);
	cleardevice();

	// ������
	struct Contribution
	{
		int update_line;
		int update_length;
	};

	// ÿ�չ��������±�Ϊ�ڼ��죬�����Ӧ������ p_dataProject �в���
	Contribution* pContribution = new Contribution[p_data_index];
	memset(pContribution, 0, sizeof(Contribution) * p_data_index);

	// ����ÿ�չ���
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

	// �����������ֵ
	int max_line = 0, max_length = 0;
	int max_update_line = 0, max_update_length = 0;

	// ��������������ֵ
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

	// ���Ͻ���ʾ��Ŀ����
	settextcolor(BLUE);
	settextstyle(20, 0, L"consolas", 0, 0, 1000, false, false, false);
	outtextxy(20, 20, L"Project:");

	wchar_t wstrProjectName[1024] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, p_strProjectName, -1, wstrProjectName, 1024);

	settextcolor(BLACK);
	settextstyle(20, 0, L"consolas", 0, 0, 0, false, false, false);
	outtextxy(100, 20, wstrProjectName);

	// ���Ͻ���ʾ��������Ϣ
	const wchar_t* wstrIntroduceMaker = L"ProjectProgressManager(simple version) made by huidong <mailkey@yeah.net> in 2021.7.11";
	settextstyle(12, 0, L"consolas");
	outtextxy(getwidth() - textwidth(wstrIntroduceMaker), getheight() - textheight(wstrIntroduceMaker), wstrIntroduceMaker);

	// ����ͼ��

	int bottom_line_height = 20;	// �ײ������߶�
	int each_day_data_width = 70;	// ÿ������ݵ�ռ��
	int histogram_height = 200;		// ��״ͼ���ռ��
	int line_chart_height = 120;		// ����ͼ���ռ�ߣ���״ͼ������ͼ���ص���
	double histogram_line_white_precent = 0.2;		// ��״ͼ�Ĵ�����������հײ���ռ��
	double histogram_length_white_precent = 0.1;	// ��״ͼ�Ĵ�����������հײ���ռ��
	double line_chart_white_precent = 0.4;			// ����ͼ�ϰ벿�ֿհ�ռ��
	int line_chart_line_up_offset = 25;		// ����ͼ�Ĵ�������ͼ������ƫ������
	int line_chart_length_up_offset = 0;	// ����ͼ�Ĵ�������ͼ������ƫ������
	COLORREF histogram_line_color = BLUE;		// ��״ͼ�Ĵ����������������ɫ
	COLORREF histogram_length_color = YELLOW;	// ��״ͼ�Ĵ����������������ɫ
	int histogram_line_width = 25;		// ��״ͼ�Ĵ�������������
	int histogram_length_width = 25;	// ��״ͼ�Ĵ�������������
	COLORREF line_chart_line_color = BLUE;					// ����ͼ�Ĵ�������������ɫ
	COLORREF line_chart_length_color = RGB(240, 160, 0);	// ����ͼ�Ĵ�������������ɫ
	int line_chart_circle_r = 4;		// ����ͼ���İ뾶

	// ͼ������image
	IMAGE img(each_day_data_width * p_data_index, histogram_height + line_chart_height + bottom_line_height);
	SetWorkingImage(&img);
	setbkcolor(WHITE);
	cleardevice();
	setbkmode(TRANSPARENT);

	// ����ɫ���ײ�һ����
	setlinecolor(LIGHTGRAY);
	line(0, getheight() - bottom_line_height, getwidth(), getheight() - bottom_line_height);

	// ��һ�ε�����ͼ�ĵ�
	POINT pointOldLine = { 0 };
	POINT pointOldLength = { 0 };

	for (int i = 0; i < p_data_index; i++)
	{
		// ÿһ������ݱ�����ɫȡ���ڹ�����
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

		// ���壺С�š���ɫ��б��
		settextcolor(LIGHTGRAY);
		settextstyle(17, 0, L"consolas", 0, 0, 1000, false, false, false);
		setlinecolor(BLACK);
		setlinestyle(PS_SOLID, 1);

		// ���������������
		wchar_t date[6] = { 0 };
		wsprintf(date, L"%d.%d", p_dataProject[i].month, p_dataProject[i].day);
		outtextxy(each_day_data_width * i + 5, getheight() - bottom_line_height + 2, date);

		// ������״ͼ
		int line_height = (int)(((double)(histogram_height * (1 - histogram_line_white_precent)) / (double)max_line) * p_dataProject[i].line);
		int length_height = (int)(((double)(histogram_height * (1 - histogram_length_white_precent)) / (double)max_length) * p_dataProject[i].length);
		int histogram_white = (int)((each_day_data_width - histogram_line_width - histogram_length_width) / 2);	// �����������մ�С

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

		// ����״ͼ����ʾ��������
		wchar_t wstrLine[8] = { 0 };
		wchar_t wstrLength[16] = { 0 };
		wsprintf(wstrLine, L"%d", p_dataProject[i].line);
		wsprintf(wstrLength, L"%d", p_dataProject[i].length);

		settextstyle(13, 0, L"consolas");
		settextcolor(histogram_line_color);
		outtextxy(each_day_data_width * i + histogram_white, getheight() - bottom_line_height - line_height - textheight('0'), wstrLine);
		// ע�⣺�˴����������������ͼ����ɫ����Ϊ����ͼ����ɫ��������ֿ�����
		settextcolor(line_chart_length_color);
		outtextxy(each_day_data_width * i + histogram_white + histogram_line_width, getheight() - bottom_line_height - length_height - textheight('0'), wstrLength);

		// ������ͼ
		int update_line_height = getheight() - bottom_line_height - histogram_height - (int)((double)((double)(line_chart_height * (1 - line_chart_white_precent)) / (double)max_update_line) * pContribution[i].update_line);
		int update_length_height = getheight() - bottom_line_height - histogram_height - (int)((double)((double)(line_chart_height * (1 - line_chart_white_precent)) / (double)max_update_length) * pContribution[i].update_length);

		POINT point_line = { each_day_data_width * i + each_day_data_width / 2,update_line_height - line_chart_line_up_offset };
		POINT point_length = { each_day_data_width * i + each_day_data_width / 2,update_length_height - line_chart_length_up_offset };

		// ���
		setlinecolor(line_chart_line_color);
		setfillcolor(line_chart_line_color);
		fillcircle(point_line.x, point_line.y, line_chart_circle_r);
		setlinecolor(line_chart_length_color);
		setfillcolor(line_chart_length_color);
		fillcircle(point_length.x, point_length.y, line_chart_circle_r);

		// ����
		if (i > 0)
		{
			setlinestyle(PS_SOLID, 2);
			setlinecolor(line_chart_line_color);
			line(pointOldLine.x, pointOldLine.y, point_line.x, point_line.y);
			setlinecolor(line_chart_length_color);
			line(pointOldLength.x, pointOldLength.y, point_length.x, point_length.y);
		}

		// ��¼�ɵ�
		pointOldLine = point_line;
		pointOldLength = point_length;

		// �������
		wchar_t wstrUpdateLine[8] = { 0 };
		wchar_t wstrUpdateLength[16] = { 0 };
		wsprintf(wstrUpdateLine, L"%d", pContribution[i].update_line);
		wsprintf(wstrUpdateLength, L"%d", pContribution[i].update_length);

		settextstyle(14, 0, L"consolas", 0, 0, 0, false, false, false);

		int line_text_y = point_line.y - textheight(wstrUpdateLine) * 3 / 2;
		settextcolor(line_chart_line_color);
		// �������ص��ж�
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
		// �������ص��ж�
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

	// ������չ���

	COLORREF colorContribution = RED;		// ��ʾ����ֵ����ɫ
	COLORREF colorTip = BLACK;				// ��һ��ʾ�����ɫ
	COLORREF colorWordsNumOfLine = WHITE;	// ÿ��ƽ��������Ӧ����ɫ

	// ƽ��ÿ�е��ַ���
	int nWordsNumOfLine = 0;
	if (pContribution[p_data_index - 1].update_line != 0)
	{
		nWordsNumOfLine = (int)(pContribution[p_data_index - 1].update_length / pContribution[p_data_index - 1].update_line);
	}

	// ��ʾ��
	wchar_t wstrTip[1024] = { 0 };					// ��һ��ʾ��
	wchar_t wstrWordsNumOfLineTip[1024] = { 0 };	// ÿ��ƽ��������ʾ��

	// ���ݽ��չ���ȷ��������ɫ����ʾ�ı�
	if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(1))
	{
		colorContribution = RGB(0, 255, 0);
		wsprintf(wstrTip, L"��������ҡ�");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(2))
	{
		colorContribution = RGB(50, 200, 0);
		wsprintf(wstrTip, L"������˼ά��Ծ�������Լ����룡");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(3))
	{
		colorContribution = RGB(100, 190, 0);
		wsprintf(wstrTip, L"����������˵ֻ��С��һ��~");
		colorTip = BLUE;
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(4))
	{
		colorContribution = RGB(150, 150, 0);
		wsprintf(wstrTip, L"�ٽ��������������룡");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(5))
	{
		colorContribution = RGB(200, 120, 0);
		wsprintf(wstrTip, L"����Ҳ߯���˵㣬�Ͻ��ɻ");
	}
	else if (pContribution[p_data_index - 1].update_length > GetLengthContributionLevel(6))
	{
		colorContribution = RGB(255, 100, 0);
		wsprintf(wstrTip, L"�����⺮�׵���������Ϊ�һ�û�����ء�");
	}
	else
	{
		colorContribution = RGB(255, 0, 0);
		wsprintf(wstrTip, L"ǧ��֮�У�ʼ�����¡�������");
	}

	// ������ˮ��
	if (nWordsNumOfLine < GetWordsNumOfLineLimit(false) && nWordsNumOfLine != 0)
	{
		if (pContribution[p_data_index - 1].update_length > (int)GetPrivateProfileInt(L"update_length", L"level_3", 0, wstrSettingsFilePath))
		{
			wsprintf(wstrWordsNumOfLineTip, L"���������Ƕ��˵㣬����Ϊ���ǵĴ���ṹ�Ƚ�������");
		}
		else
		{
			wsprintf(wstrWordsNumOfLineTip, L"�ȿȣ�������롭���ǲ��ǡ���д��̫ɢ�˵㣿");
		}

		colorWordsNumOfLine = RED;
	}
	// ÿ�е����ݹ���
	else if (nWordsNumOfLine > GetWordsNumOfLineLimit(true))
	{
		wsprintf(wstrWordsNumOfLineTip, L"�ۣ����Ĵ�������̫Ũ�ˡ�");
		colorWordsNumOfLine = RGB(100, 0, 200);
	}
	// ÿ��ƽ����������
	else if (nWordsNumOfLine != 0)
	{
		wsprintf(wstrWordsNumOfLineTip, L"���Ĵ����治��");
		colorWordsNumOfLine = GREEN;
	}

	wchar_t wstrText[1024] = { 0 };
	wchar_t wstrWordsNumOfLineText[1024] = { 0 };
	wsprintf(wstrText, L"%d ��  %d �ֽ�", pContribution[p_data_index - 1].update_line, pContribution[p_data_index - 1].update_length);
	wsprintf(wstrWordsNumOfLineText, L"ƽ�� %d �ֽ�/��", nWordsNumOfLine);

	setfillcolor(BLACK);
	fillrectangle(60, 50, 320, 110);

	settextstyle(20, 0, L"΢���ź�");
	settextcolor(WHITE);
	outtextxy(67, 58, L"���մ��빱�ף�");

	settextcolor(colorContribution);
	outtextxy(80, 80, wstrText);

	settextcolor(colorWordsNumOfLine);
	outtextxy(205, 80, wstrWordsNumOfLineText);

	settextstyle(25, 0, L"΢���ź�");
	settextcolor(colorTip);
	outtextxy(330, 55, wstrTip);

	settextcolor(colorWordsNumOfLine);
	settextstyle(18, 0, L"΢���ź�");
	outtextxy(330, 90, wstrWordsNumOfLineTip);

	// ���ͼ��
	int nPutImageX = 70;	// ���ͼ��x����
	int nPutImageY = 130;	// ���ͼ��y����
	int nPutImageWidth = 500;	// ���ͼ����
	int nStartX = img.getwidth() - nPutImageWidth;	// �ü�ͼ���x��ʼ����
	putimage(nPutImageX, nPutImageY, nPutImageWidth, img.getheight(), &img, nStartX, 0);

	// ���ư�ť
	setlinecolor(BLACK);
	settextcolor(BLACK);
	settextstyle(20, 0, L"system");
	rectangle(20, 270, 50, 300);
	outtextxy(30, 280, L'<');
	rectangle(590, 270, 620, 300);
	outtextxy(600, 280, L'>');

	// ����ͼ�����
	settextstyle(16, 0, L"consolas");
	setlinecolor(line_chart_line_color);
	setfillcolor(line_chart_line_color);
	fillcircle(85, 120, line_chart_circle_r);
	outtextxy(93, 112, L"�����������");

	setlinecolor(line_chart_length_color);
	setfillcolor(line_chart_length_color);
	fillcircle(210, 120, line_chart_circle_r);
	outtextxy(218, 112, L"�����������");

	setlinecolor(BLACK);
	setfillcolor(histogram_line_color);
	fillrectangle(330,116,338,124);
	outtextxy(342,112,L"�ܴ�������");

	setlinecolor(BLACK);
	setfillcolor(histogram_length_color);
	fillrectangle(435, 116, 443, 124);
	outtextxy(447, 112, L"�ܴ����ֽ���");

	// ��Ӧ�����Ϣ
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

		// ��ť����֧��
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

		// ���ͼ��
		BeginBatchDraw();
		clearrectangle(nPutImageX, nPutImageY, nPutImageX + nPutImageWidth, img.getheight() + nPutImageY);
		putimage(nPutImageX, nPutImageY, nPutImageWidth, img.getheight(), &img, nStartX, 0);
		EndBatchDraw();

		// ��ֹ�������µ��ƶ�����
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

	// �������ݺ������ٶ�һ�Σ����½�������
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

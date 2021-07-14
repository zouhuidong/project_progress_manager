# Project progress manager
项目进度管理器

用easyx写的简易项目进度管理器，主要是想记录一下自己每天的项目进度。

__效果图片：__

![运行截图](https://github.com/zouhuidong/project_progress_manager/blob/master/screenshots/2.png)

![运行截图](https://github.com/zouhuidong/project_progress_manager/blob/master/screenshots/1.jpg)


__编译环境__

VS2019 + EasyX(Ver 2021.02.24)（官网 easyx.cn ）


## 配置方式

__所需文件要求__

	1. 程序同目录下必须存在project.dat（存储项目进度数据），settings.ini（配置文件）
	2. project.dat 文件格式要求
		第一行：项目名
		第二行：项目代码文件路径，如果有多个就每个写一行，可以写相对路径。
     			如果要包含一个目录，就在此行写目录路径（如 "D:\myproject\web"，可以是相对路径），
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

	3. settings.ini 文件格式要求
	    这个配置文件可以修改的原因是不同语言的码量可以做的事情不同。

	    update_length（更新代码的字节数）的 level 分六个等级，
	    level_1等级最高。
	    level_1到level_6表示每个等级对应的码量。

	    update_line（更新代码的行数）有low和max两个键。
	    将更新代码的字节数除以更新代码的行数，得到平均每行代码的
	    字节数，这个数反映出用户是否在水行，如果此值低于最低标准low
	    则判定为水行；如果高于max则会提示用户代码密度过大；若此数的大小在low和max
	    之间，则被判定为代码密度没问题。
	    
	    文件示例（默认设置）：
		[update_length]
		level_1=10000
		level_2=8000
		level_3=5000
		level_4=2800
		level_5=1500
		level_6=1000
		[update_line]
		low=7
		max=100

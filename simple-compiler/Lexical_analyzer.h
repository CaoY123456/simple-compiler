#pragma once
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>
#include<set>
#include<cstdio>
#include<iterator>
#include<algorithm>
using namespace std;

class Grammar_analyzer;

//构建这个类的目的：就是给定一个文件名，能给出关于文件的词法分析内容，采取的策略是边读入边处理判断
class Lexical_analyzer {
	friend class Grammar_analyzer;//使得语法分析类可以使用词法分析类的private成员
private:
	string file_name;//表示处理的文件的名字
	vector<pair<size_t, string> >table;//最终生成的种类-对象二元序列表格，不过它的生成不依赖于构造函数，而是依赖于create_table成员函数及已经却确定的file_name，
	//即一个file_name（代表一个文件）对应一个目标二元序列表格

	vector<int>row_order;//标记table容器中的每个单词对应的行序（从0开始）

	string notation;//专门用于存储注释中的内容
	bool file_flag = false;//当table对应的文件是file_name所代表的文件时，这个值为true，反之则为false

	pair<int, string> chars_flag;//用于标记当前正在检测执行对什么东西的检测：
	//1、如果是关键字，则其值为：(1,keywords)
	//2、如果是分隔符，其值为：(2,separaters)
	//3、如果是各种运算符，其值为：(3,operators)
	//4、如果是过滤符，其值为：(4,filters)
	//5、如果是标识符，其值为：(5,names)
	//6、如果是常数，其值为：(6,number)
	//7、其他类型，其值为：(7,other)

	vector<string>keyword = { "if","else","while","signed","throw","union","this",
		"int","char","double","unsigned","const","goto","virtual","for","float",
	"break","auto","class","operator","case","do","long","typedef","static","friend",
	"template","default","new","void","register","extern","return","enum","inline",
	"try","short","continue","sizeof","switch","private","protected","asm","catch",
	"delete","public","volatile","struct","begin","end","call","string","procedure","def","end.","and","or" };//关键字容器，用于确定一个所检测到的某一个字符串是不是我们所规定（记录）的关键字（目前记录了55个）

	//用关联容器set存储，将具有较快的查找效率
	set<char>separater = { ';',',','{','}','[',']','(',')' };//分隔符记录容器（目前记录了8个）

	set<char>num_operators = { '+','-','*','/','%' };//算数运算符记录容器（目前记录了5个）

	set<string>cmp_operators = { ">","<","==","!=",">=","<=","<>" };//比较运算符，记录了7个

	set<string>logic_operators = { "&&","||","!" };//逻辑运算符，记录了3个

	set<string>pos_operators = { "&","|","^",">>","<<" };//位运算符，记录了5个

	set<string>put_operators = { "=","+=","-=","*=","/=","%=",">>=","<<=","&=","|=","^=" };//赋值运算符，记录了11个

	set<char>filter = { ' ','\t','\n','\r' };//过滤符记录容器（目前记录了4个）

	//我们将上述不同字符串（或字符）的种别码作出如下规定：
	//1、关键字的种别码分别为1-48中的任意一个数，具体的与keyword容器中每一个关键字对应的位序相同
	//2、分隔符的种别码均对应为70；
	//3、算术运算符的种别码均对应为100；
	//4、比较运算符的种别码均对应为150；
	//5、逻辑运算符的种别码均对应为200；
	//6、位运算符的种别码均对应为249；
	//7、赋值运算符的种别码均对应为300；
	//8、过滤符的种别码对应为350；
	//9、所有的标识符的种别码对应为400；
	//10、常数对应的种别码为450
	//11、其他不可识别字符的种别码对应为500
	//12、注释符‘/**/’或‘//’标记为550

public:
	//构造函数：
	//1、默认的空的构造函数：
	Lexical_analyzer() :file_name("") {}
	//2、带一个表示要读取文件名的参数的构造函数：
	Lexical_analyzer(const string& name) :file_name(name), file_flag(false) {}

	//默认的析构函数：是空的，不需要管也可以
	~Lexical_analyzer() {}

	//file_name更改赋值函数：（但是要注意，每一次改变file_name，都必须要改变table里的内容，为此我们会输出一个提示内容，还会把相应的flag置为false）
	void set_file(const string& name);

	//判断类操作：（以下函数主要是辅助主操作函数的小型函数）
	int is_keyword(const string& word)const;//判断是否是关键字，如果是，则返回对应的位序加1，如果不是，则返回0

	bool is_separater(const char& ch)const;//判断是否为分隔符标记

	bool is_lower(const char& ch)const;//判断是不是小写字母函数：
	bool is_upper(const char& ch)const;//判断是否是大写字母函数
	bool is_digital(const char& ch)const;//判断字符是否为0-9的数字

	bool is_num_operator(const char& ch)const;//判断是否是算数运算符
	bool is_cmp_operators(const string& s)const;//判断是否为比较运算符
	bool is_logic_operators(const string& s)const;//判断是否为逻辑运算符
	bool is_pos_operators(const string& s)const;//判断是否为位运算符
	bool is_put_operators(const string& s)const;//判断是否为赋值运算符

	bool is_operators(const char& s)const;//判断传进来的字符串是否为由单字符组成的且属于以上五种运算符的运算符（将会用于识别单个字符的种类以及辅助判断
	//一些字符串所属类型）

	bool is_filter(const char& ch)const;//判断是否是过滤符

	void analyse();//用于生成file_name所对应的种类序号-对象二元序列表格

	bool is_blank(const char& ch)const;//判断这个字符是不是空白字符

	void is_what(const char& ch);//根据读取到的第一个字符的情况，来分别分析可能单词可能是是哪种成分

	void show_inf()const;//信息展示函数：（展示的信息包括：所处理的文件的名字file_name以及该文件对应的目标表格table以文件中的注释内容notation）
};

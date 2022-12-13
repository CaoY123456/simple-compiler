#pragma once
#include<iostream>
#include<cstring>
#include<vector>
#include<set>
using namespace std;

class Lexical_analyzer;
class Quaternion;

class cmp {
public:
	bool operator()(const pair<size_t, string>& a, const pair<size_t, string>& b)const {
		if (a.first != b.first) {
			return a.first < b.first;
		}
		else {
			return a.second < b.second;
		}
	}
};

class Grammar_analyzer {
	friend class Quaternion;//使得四元分析式类可以使用语法分析类的private成员
private:
	shared_ptr<vector<pair<size_t, string> > >p;//用于共享Lexical_analyzer对象的table中的内容
	shared_ptr<vector<int> >p_row;//用于共享Lexical_analyzer中的row_order的内容

	vector<pair<size_t, string> >error;//为了用于记录错误语句的位置及错误的原因

	set<char>filter = { ' ','\t','\n','\r' };//过滤符记录容器（目前记录了4个）

	bool is_enter();//是不是遇到了一个回车，如果遇到了，那么就再更新

	void is_program();//是不是一个程序
	bool is_part_program();//是不是一个分程序
	bool is_stmt_list();//是不是一个语句列表（这里的语句列表是专门放在分程序里的）

	bool is_stmt_list_2();//是不是复合语句里的语句列表

	bool is_stmt();//是不是一个语句

	bool is_define();//是不是一个定义语句
	bool is_assign();//是不是一个赋值语句
	bool is_condition();//是不是一个条件语句
	bool is_loop();//是不是一个循环语句
	bool is_func();//是不是一个过程调用语句
	bool is_compound();//是不是一个复合语句

	bool is_cond_expr();//是不是一个条件表达式
	bool is_relation();//是不是一个关系表达式，因为关系表达式可以构成条件表达式

	bool is_expr();//是不是表达式
	bool is_item();//是不是项
	bool is_factor();//是不是因子

	bool with_item();//是不是有连接符连接的项
	bool with_factor();//是不是有连接符的因子

	bool is_cmp_operators();//是不是关系运算符
	bool is_separater();//是不是分隔符
	bool is_put_operators();//是不是赋值运算符

	void to_end();//如果对一个语句的检测在中间过程出现了问题，我们就不对这个语句进行后续的检测，这个函数可以使得word读到
	//这个语句最近的分号的位置而如果没有分号，则一直前进到分程序的末尾end标记，如果没有end，那就前进到下一个分程序的开始
	//标记procedure或是读到table的cend()处

	//vector<pair<size_t, string> >::const_iterator ing_word;//当前处理的单词
	vector<pair<size_t, string> >::const_iterator word;//当前处理的单词
	size_t line_order = 0;//行号，注意：从0开始
	size_t tmp_line = 0;//辅助行号，为了精确定位错误的位置

	set<string>cmp_operators = { ">","<","==","!=",">=","<=","<>" };//关系运算符，记录了7个
	set<char>separater = { ';',',','{','}','[',']','(',')' };//分隔符记录容器（目前记录了8个）
	set<string>put_operators = { "=","+=","-=","*=","/=","%=",">>=","<<=","&=","|=","^=" };//赋值运算符，记录了11个

	set<string>define;//存储定义语句变量的set容器，其中的pair的first成员是开始的行数位置，second成员是变量对应的标识符名

	vector<pair<size_t, vector<pair<size_t, string> > > >expr;//存储表达式的容器，外层pair的first位表示该表达式开始的位置
	//注：以上两个容器设立的目的都是为了其后的四元分析式的构建

	vector<pair<size_t, string> > wd;
public:
	Grammar_analyzer() :p(NULL), p_row(NULL) {}//默认的构造函数
	Grammar_analyzer(const Lexical_analyzer& L);//含有一个词法分析类对象的参数的构造函数

	bool is_filter();//判断当前处理的单词是不是过滤符，如果是，则递归读入下一个直到读入非过滤符再返回true，
	//同时要判断是不是回车符（是的话要变换相应的行数tmp_line，否则不变），如果不是过滤符，则就处理这个单词且返回false

	void analyse();//语法分析函数

	void show_inf()const;//信息展示函数
};

/*
1.语句列表表示为stmt_list
2.语句表示为stmt
3.define表示变量定义语句
4.assign表示赋值语句
5.condition表示条件语句
6.loop表示循环语句
7.func表示过程调用语句
8.compound表示复合语句
*/
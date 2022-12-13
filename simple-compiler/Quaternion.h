#pragma once
#include<iostream>
#include<vector>
#include<set>
#include<iterator>
#include<string>
#include<utility>
#include<memory>
using namespace std;

class Grammar_analyzer;
class cmp;

struct FourElements {
	string oper;//四元式对应的运算符
	string var1;//四元式中的一号变量
	string var2;//四元式中的二号变量
	string aim;//四元式中的目标存储值
	size_t num;//该四元式的位序
	//static size_t _number;//辅助标记该四元式的位序的静态变量：

	//默认的构造函数：
	FourElements();
	//带参数的构造函数：
	FourElements(const string& op, const string& v1, const string& v2, const string& a, const size_t& n);
	//拷贝构造函数：
	FourElements(const FourElements& F);
	//拷贝赋值运算符
	FourElements& operator=(const FourElements& F);

	//析构函数：使用合成的默认的析构函数
	~FourElements() {}
};

class Quaternion {
private:
	set<string>var;//存储已经经过定义的变量的名字的容器集合，将在涉及到变量赋值语句和比较语句时用于搜索来检测使用的变量是不是
	//已经定义过。

	vector<pair<size_t, string> >error;//用于存储使用没有定义过的变量的错误的提示信息以及出现错误对应的行数。

	vector<FourElements>fe;//存储最终四元式结果的vector容器

	shared_ptr<set<string> >define;//存储各种变量的set容器的智能共享指针

	shared_ptr<vector<pair<size_t, vector<pair<size_t, string > > > > >expr;//存储各种表达式的位置以及表达式中的成分的
	//vector容器的智能指针，其中，vector容器里存放的是pair对象，pair对象的first成员是这些表达式所在的位置，second成员是
	//表达式的内容，也是用一个vecto容器来存，其中的vector中放置的也是一个pair对象，其first成员是表达式不同成分对应的
	//种别码，second成员是表达式各成分的实际内容。

public:
	//默认的构造函数：
	Quaternion() :define(NULL), expr(NULL) {}
	//带参数的构造函数：
	Quaternion(const Grammar_analyzer& G);

	void construct();//四元分析式构造函数（核心）

	void show_inf()const;//信息展示函数.
};


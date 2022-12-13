#include"Lexical_analyzer.h"
#include"Grammar_analyzer.h"
#include"Quaternion.h"
using namespace std;

//file_name更改赋值函数：（但是要注意，每一次改变file_name，都必须要改变table里的内容，为此我们会输出一个提示内容，还会把相应的flag置为false）
void Lexical_analyzer::set_file(const string& name) {
	if (name != file_name) {
		cout << "注意：您的文件已变更，此时对应的二元表格里的内容可能与新文件的内容不符，请慎重使用" << endl;
		file_flag = false;
		file_name = name;
		table.erase(table.begin(), table.end());
		notation = "";
	}
}//注意：如果对相同的文件名里的具体内容进行变更，我们不提供这种改变操作的后续改变，所以请慎重使用

//判断类操作：（以下函数主要是辅助主操作函数的小型函数）
int Lexical_analyzer::is_keyword(const string& word)const {//判断是否是keyword中所列关键字，如果是，对应的位序+1，否则返回0
	string m;
	transform(word.begin(), word.end(), back_inserter(m), ::tolower);//将传进来的字符串全部转换为小写字母，即keyword不区分小写字母，不过可能出错
	for (vector<string>::const_iterator it = keyword.cbegin(); it != keyword.cend(); ++it) {
		if (*it == m) {
			return it - keyword.cbegin() + 1;
		}
	}
	return 0;
}

bool Lexical_analyzer::is_separater(const char& ch)const {//判断是否为分隔符标记
	set<char>::iterator it = separater.find(ch);//如果找到了，则返回指向那个元素的迭代器，如果找不到，则返回一个尾后迭代器
	if (it == separater.cend()) {//表明没有找到，即这个字符不是一个分隔符
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_lower(const char& ch)const {//判断是不是小写字母函数：
	if (ch <= 'z' && ch >= 'a') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_upper(const char& ch)const {//判断是否是大写字母函数
	if (ch <= 'Z' && ch >= 'A') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_digital(const char& ch)const {//判断传进来的字符是否是0-9之间的数字
	if (ch <= '9' && ch >= '0') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_num_operator(const char& ch)const {//判断是否是算数运算符
	set<char>::iterator it = num_operators.find(ch);//如果找到了，则返回指向那个元素的迭代器，如果找不到，则返回一个尾后迭代器
	if (it == num_operators.cend()) {//表明没有找到，即这个字符不是一个分隔符
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_cmp_operators(const string& s)const {//判断是否为比较运算符
	set<string>::iterator it = cmp_operators.find(s);//如果找到了，则返回指向那个元素的迭代器，如果找不到，则返回一个尾后迭代器
	if (it == cmp_operators.cend()) {//表明没有找到，即这个字符不是一个分隔符
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_logic_operators(const string& s)const {//判断是否为逻辑运算符
	set<string>::iterator it = logic_operators.find(s);
	if (it == logic_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_pos_operators(const string& s)const {//判断是否为位运算符
	set<string>::iterator it = pos_operators.find(s);
	if (it == pos_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_put_operators(const string& s)const {//判断是否为赋值运算符
	set<string>::iterator it = put_operators.find(s);
	if (it == put_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_operators(const char& ch)const {//判断传进来的字符串是否为由单字符组成的且属于以上五种运算符的运算符（将会用于识别单个字符的种类以及辅助判断
	//一些字符串所属类型）
	switch (ch) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '%':
	case '>':
	case '<':
	case '=':
	case '&':
	case '|':
	case '!':
	case '^'://这种情况要特别注意，一般认为只有当它出现在序列的开始（作逻辑运算符‘非’用或者是后面仅连接一个‘=’时才可作进一步判断）
		return true;
		break;
	default:
		return false;
		break;
	}
}

bool Lexical_analyzer::is_filter(const char& ch)const {//判断是否是过滤符
	set<char>::iterator it = filter.find(ch);
	if (it == filter.cend()) {
		return false;
	}
	else {
		return true;
	}
}

void Lexical_analyzer::analyse() {//用于生成file_name所对应的种类序号-对象二元序列表格（这是这个类中最关键的函数）
	ifstream input;
	if (file_flag)return;//表明此时的table、notation不需要做分析
	input.open(file_name);

	char ch = ' ';
	string s = "";

	bool point_flag = false;//用于数字检测识别时指示数字中有没有包含小数点
	bool e_flag = false;//用于数字检测识别时数字中有没有包含指数位
	bool o_flag = false;//用于标识数字中识别时数字中的指数位中的符号（为true表示在紧挨着指数号的后面加了一个正负号，为false表示没有加正负号）
	//注意：以上三个标志位在确定识别出一个数字以后要恢复其默认的false值

	bool note_left = false;//指示多行注释符左半部分‘/*’是否出现为true时则表示已经出现
	//bool note_right = false;//指示多行注释符右半部分‘*/’是否出现为true时则表示已经出现
	//目前暂时认为单行注释符不需要标志位来辅助检测

	int left_it;//表示多行注释符“/*”中的“/”与table容器中首元素的位置相差的序号数，本来想直接搞一个迭代器指向‘/’来着，但是由于边读文件边识别，所以
	//table容器后续还会变动，可能引起了 容器内部迭代器的变动，而这个字符到首元素的位置一定，抓住这一点可以顺利删除想删除的元素，规避迭代器的弊病
	//之所以要设置这两个迭代器标记那两个位置，是因为我们是边扫描文件边进行识别的，有时候可能识别不到一个完整的多行注释符，比如这种情况：“/*j vvebv*as”
	//所以为了应对这种情况，我们如果要设置左右标志位，然后把其放到table容器中，然后删除这之间的元素，从而对注释的内容进行一个忽略。

	cin.unsetf(ios::skipws);//这个操作是为了不要忽略文件中的空白字符（空格、回车符等）

	int row = 0;//标记每个单词行序的变量
	if (!input.eof()) {//这一步的目的主要是先把文档的第一个字符提取出来好先确定chars_flag的值
		if (input.get(ch)) {
			is_what(ch);
			s += ch;
		}
	}

	while (!input.eof()) {
		//if (input.get(ch)) {//这么做可以避免多一次处理最后一个字符（不过这里可能出问题）
		input.get(ch);
		if (input.eof())
			ch = NULL;
		switch (chars_flag.first) {
		case 1: {//表明读取的可能是关键字，当然也有可能是一个标识符
			/*if(input.eof()){
				int key_flag = is_keyword(s);
				if (key_flag) {
					table.push_back(make_pair(key_flag, s));
				}
				else {
					table.push_back(make_pair(400, s));
				}
				break;
			}*/
			if (is_upper(ch) || is_lower(ch) || is_digital(ch)) {
				if (s.size() >= 20) {
					table.push_back(make_pair(400, s));
					row_order.push_back(row);
					is_what(ch);
					s = "";
				}
				s += ch;
			}
			else if (ch == '.' && s == "end") {
				s += ch;
			}
			else if (ch == '_') {
				chars_flag.first = 5;
				chars_flag.second = "names";//这个字符串由可能的关键字序列变为标识符序列
				if (s.size() >= 20) {
					table.push_back(make_pair(400, s));
					row_order.push_back(row);
					is_what(ch);
					s = "";
				}
				s += ch;
			}
			else {
				int key_flag = is_keyword(s);
				if (key_flag) {
					table.push_back(make_pair(key_flag, s));//读取到了一个关键字
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				else {
					chars_flag.first = 5;
					chars_flag.second = "names";//注意：这个地方的ch有可能是运算符、分隔符、过滤符的一种，而标识符序列中不能出现这些字符，
					//所以我们要注意及时识别这些
					if (!is_lower(ch) && !is_upper(ch) && !is_digital(ch) && ch != '_') {
						table.push_back(make_pair(400, s));
						row_order.push_back(row);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						if (s.size() >= 20) {
							table.push_back(make_pair(400, s));
							row_order.push_back(row);
							is_what(ch);
							s = "";
						}
						s += ch;
					}
				}
			}
		}
			  break;
		case 2: {//表明读取的可能是分隔符
			table.push_back(make_pair(70, s));
			row_order.push_back(row);
			is_what(ch);
			s = "";
			s += ch;
		}
			  break;
		case 3: {//表明读取的可能是某种运算符
			switch (s[0]) {
			case '+':
			case '-':
			case '%': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(300, s));
					row_order.push_back(row);
					//if (!input.eof()) {
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
					//}
					/*else {
						table.push_back(make_pair(100, s));
						break;
					}*/
				}
						break;
				default: {
					table.push_back(make_pair(100, s));
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '<': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(150, s));//比较运算符‘<=’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				case '<': {
					s += ch;
					if (!input.eof()) {
						input.get(ch);
						if (ch == '=') {
							s += ch;
							table.push_back(make_pair(300, s));//赋值运算符：‘<<=’
							row_order.push_back(row);
							if (!input.eof()) {
								input.get(ch);
								is_what(ch);
								s = "";
								s += ch;
							}
							else {
								table.push_back(make_pair(150, s));
								row_order.push_back(row);
								break;
							}
						}
						else {
							table.push_back(make_pair(249, s));//位运算符‘<<’
							row_order.push_back(row);
							is_what(ch);
							s = "";
							s += ch;
						}
					}
					else {
						table.push_back(make_pair(150, s));
						row_order.push_back(row);
						break;
					}
				}
						break;
				case '>': {
					s += ch;
					table.push_back(make_pair(150, s));//比较运算符‘<>’
					row_order.push_back(row);
					if (!input.eof())
						input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					table.push_back(make_pair(150, s));//比较运算符‘<’
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '>': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(150, s));//比较运算符‘>=’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				case '>': {
					s += ch;
					input.get(ch);
					if (ch == '=') {
						s += ch;
						table.push_back(make_pair(300, s));//赋值运算符‘>>=’
						row_order.push_back(row);
						input.get(ch);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						table.push_back(make_pair(249, s));//位运算符‘>>’
						row_order.push_back(row);
						is_what(ch);
						s = "";
						s += ch;
					}
				}
						break;
				default: {
					table.push_back(make_pair(150, s));//比较运算符‘>’
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '=': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(150, s));//比较运算符或‘==’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					table.push_back(make_pair(300, s));//赋值运算符‘=’
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '&':
			case '|': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(300, s));//赋值运算符‘&=’或‘|=’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					if (s[0] == ch) {
						s += ch;
						table.push_back(make_pair(200, s));//逻辑运算符‘&&’或‘||’
						row_order.push_back(row);
						input.get(ch);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						table.push_back(make_pair(249, s));//位运算符‘&’或‘|’
						row_order.push_back(row);
						is_what(ch);
						s = "";
						s += ch;
					}
				}
				}
			}
					break;
			case '^':
			case '!': {
				switch (ch) {
				case '=': {
					s += ch;
					if (s == "^")
						table.push_back(make_pair(300, s));//赋值运算符‘^=’
					else
						table.push_back(make_pair(150, s));//比较运算符‘!=’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					if (s == "^") {
						table.push_back(make_pair(249, s));//位运算符‘^’	
					}
					else {
						table.push_back(make_pair(200, s));//逻辑运算符‘!’	
					}
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '*': {//注意，之所以将‘*’和‘/’的检测放在后面，是因为我们要想方设法识别注释号：‘/**/’和‘//’
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(300, s));//赋值运算符‘*=’
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				case '/': {//此时要注意，极有可能是注释的后半部分‘*/’
					if (note_left) {//表明已经有了一个左注释符，两个单注释符构成一个完整的多行注释（目前我们的注释中不包含中文字符，后期可能会进行优化）
						for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + left_it; it != table.cend(); ++it) {
							notation += it->second;
						}
						notation += "*/";
						notation += '\n';
						table.erase(table.begin() + left_it, table.end());//对注释的已经放到table容器的内容进行删除（删除的也包括‘/*’，后面将会对其进行在table中后插入操作）
						row_order.erase(row_order.begin() + left_it, row_order.end());
						//s += ch;
						//table.push_back(make_pair(550, "/*"));
						//table.push_back(make_pair(550, s));//不插入注释
						//row_order.push_back(row);
						note_left = false;//注意找到一个完整的注释符并将其插入后，要将相应的note_left复原

						input.get(ch);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						table.push_back(make_pair(100, s));
						row_order.push_back(row);
						is_what(ch);
						s = "";
						s += ch;
					}
				}
						break;
				default: {
					table.push_back(make_pair(100, s));//算术运算符‘*’
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '/': {
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(300, s));
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				case '*': {
					table.push_back(make_pair(100, s));//算数运算符‘/’
					row_order.push_back(row);
					left_it = table.end() - 1 - table.begin();//因为可能涉及到容器的内部的一些可能的缺陷，故此处容器出问题，即迭代器指向的是不是我们希望的
					is_what(ch);
					s = "";
					s += ch;
					note_left = true;//遇到了一个注释符的左半部分，但是此时我们不要将其合到一起（变成‘/*’），因为很有可能最终构不成一个完整的注释符
					//所以只要改变相应的标志位即可
				}
						break;
				case '/': {
					s += ch;
					//table.push_back(make_pair(550, s));//单行注释符“//”
					//row_order.push_back(row);
					notation += s;
					char tmp_ch;
					if (note_left == true) {//有可能多行注释里包含着一个单行注释，那么可以确定，//后的内容一定是注释的内容
						string s_tmp2 = "";

						size_t flag2 = notation.size() - 2;

						while (input.get(tmp_ch)) {
							notation += tmp_ch;
							if (tmp_ch == '\n') {
								++row;
							}
							else if (tmp_ch == '*') {
								s_tmp2 = "";
								s_tmp2 += tmp_ch;
							}
							else if (tmp_ch == '/' && s_tmp2 == "*") {//识别到了一个*/，且‘/**/’包着‘//’

								string tmp_notation;
								copy(notation.begin() + flag2, notation.end(), back_inserter(tmp_notation));

								notation.erase(notation.begin() + flag2, notation.end());

								for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + left_it; it != table.cend(); ++it) {
									notation += it->second;
								}
								for (string::const_iterator it = tmp_notation.cbegin(); it != tmp_notation.cend(); ++it) {
									notation += *it;
								}

								table.erase(table.begin() + left_it, table.end());//对注释的已经放到table容器的内容进行删除（删除的也包括‘/*’，后面将会对其进行在table中后插入操作）
								row_order.erase(row_order.begin() + left_it, row_order.end());
								//s += ch;
								//table.push_back(make_pair(550, "/*"));
								//table.push_back(make_pair(550, s));//不插入注释
								//row_order.push_back(row);
								note_left = false;//注意找到一个完整的注释符并将其插入后，要将相应的note_left复原
								break;
							}
							else {
								s_tmp2 = "";
							}
						}//为了记录注释中的内容，本质上不对注释进行识别	
					}
					else {
						while (input.get(tmp_ch) && tmp_ch != '\n') {
							notation += tmp_ch;
						}//为了记录注释中的内容，本质上不对注释进行识别
						++row;
					}
					notation += '\n';
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					table.push_back(make_pair(100, s));//算术运算符‘/’
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			}
		}
			  break;
		case 4: {//表明读取的是过滤符
			table.push_back(make_pair(350, s));
			row_order.push_back(row);
			if (s[0] == '\n') {
				++row;
			}
			is_what(ch);
			s = "";
			s += ch;
		}
			  break;
		case 5: {//表明读取的是标识符
			if (!is_upper(ch) && !is_lower(ch) && !is_digital(ch) && ch != '_') {
				table.push_back(make_pair(400, s));
				row_order.push_back(row);
				is_what(ch);
				s = "";
				s += ch;
			}
			else {
				if (s.size() >= 20) {
					table.push_back(make_pair(400, s));
					row_order.push_back(row);
					is_what(ch);
					s = "";
				}
				s += ch;
			}
		}
			  break;
		case 6: {//表明读取的是一个常数，此时可以允许它的第一个数为0
			/*if (input.eof()) {
				table.push_back(make_pair(450, s));
			}*/
			if (is_digital(ch)) {
				s += ch;
			}
			else {
				if (!point_flag && !e_flag && ch == '.') {//没有小数点且小数点的前面不能有指数‘E’和‘e’
					s += ch;
					point_flag = true;
				}
				else if ((point_flag || e_flag) && ch == '.') {//如果已经有了小数点或者小数点前已经有了指数位，那么不能往往数字串里再加小数点
					table.push_back(make_pair(450, s));
					row_order.push_back(row);
					point_flag = false;
					e_flag = false;
					o_flag = false;
					is_what(ch);
					s = "";
					s += ch;
				}
				else if (!e_flag && (ch == 'e' || ch == 'E')) {//在一个是数字串中，最多只能有一个指数位
					s += ch;
					e_flag = true;
				}
				else if (e_flag && (ch == 'e' || ch == 'E')) {
					table.push_back(make_pair(450, s));
					row_order.push_back(row);
					point_flag = false;
					e_flag = false;
					o_flag = false;
					is_what(ch);
					s = "";
					s += ch;
				}
				else if ((*(s.end() - 1) == 'E' || *(s.end() - 1) == 'e') && !o_flag && (ch == '+' || ch == '-')) {
					s += ch;
					o_flag = true;
				}
				else {
					table.push_back(make_pair(450, s));
					row_order.push_back(row);
					point_flag = false;
					e_flag = false;
					o_flag = false;
					is_what(ch);
					s = "";
					s += ch;
				}
			}
		}
			  break;
		default: {//表明读取的是一个其他字符
			table.push_back(make_pair(500, s));
			row_order.push_back(row);
			is_what(ch);
			s = "";
			s += ch;
		}
		}
		//}

	}
	input.close();
	file_flag = true;
}

bool Lexical_analyzer::is_blank(const char& ch)const {//判断这个字符是不是空白字符
	if (ch == ' ' || ch == '\t' || ch == '\n') {
		return true;
	}
	else {
		return false;
	}
}

void Lexical_analyzer::is_what(const char& ch) {//根据读取到的第一个字符的情况，来分别分析可能单词可能是是哪种成分
	if (is_lower(ch) || is_upper(ch)) {
		chars_flag.first = 1;
		chars_flag.second = "keywords";//表明所读取的字符串可能是一个关键字，当然也有可能是一个标识符
	}
	else if (ch == '_') {
		chars_flag.first = 5;
		chars_flag.second = "names";//表明所读取的字符可能是一个标识符
	}
	else if (is_operators(ch)) {
		chars_flag.first = 3;
		chars_flag.second = "operators";//表明所读取的字符是某种运算符
	}
	else if (is_digital(ch)) {
		chars_flag.first = 6;
		chars_flag.second = "number";
	}
	else {
		switch (ch) {
		case ';':
		case ',':
		case '{':
		case '}':
		case '[':
		case ']':
		case '(':
		case ')': {
			chars_flag.first = 2;
			chars_flag.second = "separaters";
		}
				break;
		case ' ':
		case '\t':
		case '\r':
		case '\n': {
			chars_flag.first = 4;
			chars_flag.second = "filters";
		}
				 break;
		default: {
			chars_flag.first = 7;
			chars_flag.second = "other";
		}
		}
	}
}
void Lexical_analyzer::show_inf()const {//信息展示函数：（展示的信息包括：所处理的文件的名字file_name以及该文件对应的目标表格table）
	cout << "处理的文件的名称为：" << file_name << endl;
	if (table.empty()) {
		cout << "该文件的词法分析结果为空" << endl;
		return;
	}

	cout << "对于该文件的词法分析结果为：" << endl;
	cout << '(' << table.cbegin()->first << ' ' << ',' << ' ';
	switch (table.cbegin()->first) {
	case 70:cout << "分隔符" << ' ' << ',' << ' '; break;
	case 100:cout << "算术运算符" << ' ' << ',' << ' '; break;
	case 150:cout << "比较运算符" << ' ' << ',' << ' '; break;
	case 200:cout << "逻辑运算符" << ' ' << ',' << ' '; break;
	case 249:cout << "位运算符" << ' ' << ',' << ' '; break;
	case 300:cout << "赋值运算符" << ' ' << ',' << ' '; break;
	case 350:cout << "过滤符" << ' ' << ',' << ' '; break;
	case 400:cout << "标识符" << ' ' << ',' << ' '; break;
	case 450:cout << "常数" << ' ' << ',' << ' '; break;
	case 500:cout << "其他不可识别字符" << ' ' << ',' << ' '; break;
	case 550:cout << "注释符" << ' ' << ',' << ' '; break;
	default:cout << "关键字" << ' ' << ',' << ' ';
	}
	if (table.cbegin()->first == 350) {
		switch (table.cbegin()->second[0]) {
		case '\t':cout << "tab制表符" << ')'; break;
		case '\n':cout << "回车符" << ')'; break;
		case ' ':cout << "空格" << ')'; break;
		default: cout << table.cbegin()->second << ')';
		}
	}
	else {
		cout << table.cbegin()->second << ')';
	}
	cout << '\t' << "所在行数：" << *row_order.cbegin();

	vector<int>::const_iterator  it2 = row_order.cbegin() + 1;
	for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + 1; it != table.cend(); ++it) {
		cout << endl << '(' << it->first << ' ' << ',' << ' ';
		switch (it->first) {
		case 70:cout << "分隔符" << ' ' << ',' << ' '; break;
		case 100:cout << "算术运算符" << ' ' << ',' << ' '; break;
		case 150:cout << "比较运算符" << ' ' << ',' << ' '; break;
		case 200:cout << "逻辑运算符" << ' ' << ',' << ' '; break;
		case 249:cout << "位运算符" << ' ' << ',' << ' '; break;
		case 300:cout << "赋值运算符" << ' ' << ',' << ' '; break;
		case 350: cout << "过滤符" << ' ' << ',' << ' '; break;
		case 400:cout << "标识符" << ' ' << ',' << ' '; break;
		case 450:cout << "常数" << ' ' << ',' << ' '; break;
		case 500:cout << "其他不可识别字符" << ' ' << ',' << ' '; break;
		case 550:cout << "注释符" << ' ' << ',' << ' '; break;
		default:cout << "关键字" << ' ' << ',' << ' ';
		}
		if (it->first == 350) {
			switch (it->second[0]) {
			case '\t':cout << "tab制表符" << ')'; break;
			case '\n':cout << "回车符" << ')'; break;
			case ' ':cout << "空格" << ')'; break;
			default: cout << it->second << ')';
			}
		}
		else {
			cout << it->second << ')';
		}
		cout << '\t' << "所在行数：" << *it2;
		++it2;
	}
	cout << endl << endl;

	if (notation.empty()) {
		cout << "没有注释的内容";
	}
	else {
		cout << "注释的内容为：" << endl;
		cout << notation;
	}
	cout << endl << endl;
}

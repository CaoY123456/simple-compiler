#include "Grammar_analyzer.h"
#include"Lexical_analyzer.h"
#include"Quaternion.h"
using namespace std;

Grammar_analyzer::Grammar_analyzer(const Lexical_analyzer& L) :p(make_shared<vector<pair<size_t, string> > >(L.table)),
p_row(make_shared<vector<int> >(L.row_order)) {}//含有一个词法分析类对象的参数的构造函数

bool Grammar_analyzer::is_filter() {//判断当前处理的单词是不是过滤符，如果是，则继续向后递归扫描直到扫描到非过滤符并且返回false，如果扫描完所有单词都没有扫描到
	//非过滤符，则返回false，在这个过程中要判断一下是不是读到了回车符，需要变更辅助行数标记tmp_line
	if (word == p->cend()) {//单词已经扫描完
		return true;
	}
	set<char>::const_iterator r = filter.find(word->second[0]);
	tmp_line = (*p_row)[word - p->cbegin()];//用于校准当前判断的语句所在的行数，以尽可能精确定位错误的位置
	if (r != filter.cend()) {//表明此时读到的是过滤字符，而这个函数的目的就是要读到非过滤字符，如果读不到，就
		++word;//一直读到table容器末尾
		return is_filter();
	}
	else {
		return false;//读到了非过滤字符，返回false
	}
}

void Grammar_analyzer::is_program() {//是不是一个程序
	word = p->cbegin();//初始化word迭代器
	while (word != p->cend()) {
		line_order = tmp_line;
		is_part_program();//对于一个有多个分程序的程序来说，即使前面的分程序出现了错误，也不影响后续分程序的判断
		if (word != p->cend())
			++word;
		if (is_filter()) {//此处使得word指向下一个分程序开始的标志“procedure”或是下一个非过滤字符（出错的时候）
			return;//或者指到table容器末尾
		}
	}

}

bool Grammar_analyzer::is_part_program() {//是不是一个分程序
	if (is_filter()) {//指到第一个非过滤的有效字符
		error.push_back(make_pair(line_order, "这是一个空的分程序"));//可能有错
		return false;
	}
	else if (word->second != "procedure") {//由给定的文法可知，每个程序的开头必然是“procedure”
		line_order = tmp_line;
		error.push_back(make_pair(line_order, "程序的开头标记错误（不是procedure）"));

		while (word->second != "procedure") {//当一段分程序的开头“procedure”出错，努力寻找下一段分程序的开头
			//以不影响其它分程序的判断
			++word;
			if (word == p->cend())
				break;
		}
		--word;

		return false;
	}
	else {//表明此时成功识别到了程序的开头“procedure”，接着往下识别
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back(make_pair(line_order, "分程序的开头procedure后为空"));
			return false;
		}

		else if (word->first != 400) {//当前的单词不是一个标识符
			line_order = tmp_line;
			error.push_back(make_pair(line_order, "紧跟在procedure后面的非过滤字符不是一个标识符"));

			while (word->second != "procedure") {//分程序的开始组成部分出了问题，不对这个分程序作进一步判断，但
				//不影响其他分程序的判断
				++word;
				if (word == p->cend())
					break;
			}

			--word;

			return false;
		}
		else {//当前的单词是一个标识符，种别码为400
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back(make_pair(line_order, "程序开始标志中的标识符后的内容为空"));
				return false;
			}

			else if (word->second != "begin") {
				line_order = tmp_line;
				error.push_back(make_pair(line_order, "程序的开头格式中的标识符后面的非过滤符内容不是begin"));

				while (word->second != "procedure") {
					++word;
					if (word == p->cend())
						break;
				}

				--word;

				return false;
			}
			else {//表明程序从procedure到语句列表前的格式均正确，下面开始对各项语句进行判断，这里的语句列表的
				//结束标记为‘end.’
				line_order = tmp_line;
				++word;//可能出错
				if (is_filter()) {
					error.push_back(make_pair(line_order, "分程序框架格式中的begin后的内容为空"));
					return false;
				}
				else if (!is_stmt_list()) {
					line_order = tmp_line;
					//error.push_back(make_pair(line_order, "分程序的语句列表不符合格式要求"));//暂时注释掉，可能会有错
					//++word;//需不需要加这一句
					return false;
				}
				else {//分程序的语句列表满足格式要求，且一定是以end.结尾，这是因为我们识别语句列表以end作为结束标记
					//即使如果没有end，我们也会在识别语句列表的时候加以给出
					//#++word;//可能出错
					line_order = tmp_line;
					return true;
				}
			}
		}
	}
}

bool Grammar_analyzer::is_stmt_list() {//是不是一个语句列表，只要能到这一个函数，那么此时的word一定不会是过滤符
	while (word->second != "end.") {
		line_order = tmp_line;
		is_stmt();

		if (word != p->cend())
			++word;

		if (is_filter() || word->second == "procedure") {
			error.push_back({ line_order,"分程序缺少end.结束标记" });

			--word;

			return false;
		}
	}


	//#--word;//这里与判断分程序正确后那里的那个++word相呼应
	return true;
}

bool Grammar_analyzer::is_stmt() {//是不是一个语句，涉及到是什么语句（变量定义、条件、赋值、循环、过程调用、复合语句）
	//此时的word一定不是过滤符，任何语句都是以分号结尾

	bool r;//判断的结果
	switch (word->first) {//根据当前单词的种别码判断句子的类型
	case 1: {//条件语句，if对应的种别码为1
		wd.push_back(*word);
		/*expr.push_back({ line_order,wd });
		wd.clear();*/
		++word;
		r = is_condition();
	}
		  break;
	case 3: {//循环语句，while对应的种别码为3，目前只识别while对应的循环语句
		wd.push_back(*word);
		/*expr.push_back({ line_order,wd });
		wd.clear();*/
		++word;
		r = is_loop();
	}
		  break;
	case 400: {//可能为赋值语句，因为赋值语句以标识符开头，而标识符对应的种别码为400
		//wd = "";
		//wd += word->second;
		wd.push_back(*word);
		++word;
		r = is_assign();
	}
			break;
	case 49: {//可能是复合语句，复合语句以begin开头，begin对应的种别码为49
		expr.push_back({ line_order,vector<pair<size_t, string> >(1,*word) });
		++word;
		r = is_compound();
	}
		   break;
	case 51: {//可能是函数调用语句，函数调用语句以call开头，call对应的种别码为51
		++word;
		r = is_func();
	}
		   break;
	default: {
		if (word->second == "def") {//以“def”开头的定义变量语句
			++word;
			r = is_define();
		}
		else if (word->second == "int" || word->second == "char" || word->second == "double" || word->second == "float" ||
			word->second == "string") {//识别c++默认的定义语句
			++word;
			r = is_define();
		}
		else if (word->second == "end.") {
			--word;
			r = false;//语句为空###
		}
		else {
			error.push_back({ line_order,"出现了一个不属于语句的单一成分" });
			r = false;
		}
	}
	}
	return r;//stmt（语句是一个大类，对于不同小类的判断包含了对结束标记“;”以及其他结束情况的判断）
}

bool Grammar_analyzer::is_define() {//是不是一个定义语句，调用这个函数以前，已经识别出了变量定义标志（def,int,char,double,string,float）
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"变量定义语句的类型关键字的后面的内容为空" });
		return false;
	}
	else if (word->first != 400) {//类型关键字后面跟着的不是一个标识符
		line_order = tmp_line;
		error.push_back({ line_order,"变量定义语句类型关键字后面跟着的不是一个标识符" });
		to_end();
		return false;
	}
	else {//类型关键字后面跟着的是一个标识符
		line_order = tmp_line;
		//size_t tmp_line2 = line_order;
		string tmp_word1 = word->second;//??
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"定义语句变量的后面的内容为空" });
			return false;
		}
		else if (word->second == ";") {
			line_order = tmp_line;
			//--word;
			define.insert(tmp_word1);//??
			return true;
		}
		else if (word->second != "," && is_separater()) {
			line_order = tmp_line;
			error.push_back({ line_order,"定义变量语句的分隔符使用错误" });
			//--word;
			to_end();
			return false;
		}
		else if (word->second == ",") {
			bool r;
			vector<string > v_word;
			v_word.push_back(tmp_word1);
			line_order = tmp_line;
			while (word->second == ",") {
				++word;
				if (is_filter()) {
					error.push_back({ line_order,"定义变量语句的分隔符后面的内容空白" });
					r = false;
				}
				else if (word->first != 400) {
					line_order = tmp_line;
					error.push_back({ line_order,"定义变量语句分隔符后面的内容不是标识符" });
					to_end();
					//--word;
					r = false;
				}
				else {
					line_order = tmp_line;
					v_word.push_back(word->second);
					r = true;
				}
				if (!r) {
					return r;
				}
				++word;
				if (is_filter()) {
					error.push_back({ line_order,"定义变量语句的定义的变量后的内容为空白" });
					return false;
				}
			}
			if (r == true) {
				if (word->second == ";") {
					for (vector<string >::const_iterator it = v_word.cbegin(); it != v_word.cend(); ++it) {
						define.insert(*it);
					}
					return true;
				}
				else {
					line_order = tmp_line;
					error.push_back({ line_order,"定义变量语句缺少分号或分隔符使用错误" });
					to_end();
					return false;
				}
			}
		}
		else {
			line_order = tmp_line;
			//--word;
			to_end();
			error.push_back({ line_order,"语句不以分号结尾或分隔符使用错误" });
			return false;
		}
	}
}

bool Grammar_analyzer::is_assign() {//是不是一个赋值语句
	line_order = tmp_line;
	size_t start_line = line_order;
	if (is_filter()) {
		error.push_back({ line_order,"赋值语句中标识符后面的内容为空白" });
		//wd = "";//??
		//wd.clear();//???
		return false;
	}
	else if (!is_put_operators()) {
		line_order = tmp_line;
		error.push_back({ line_order,"赋值语句中的运算符不是一个赋值运算符" });
		//--word;
		to_end();
		//wd = "";//??
		wd.clear();//??
		return false;
	}
	else {//识别到了正确的赋值运算符，接下来看下一个单词属不属于表达式
		line_order = tmp_line;
		//wd += word->second;//??
		wd.push_back(*word);
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"赋值语句赋值运算符后的内容为空白" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (!is_expr()) {
			line_order = tmp_line;
			error.push_back({ line_order,"赋值语句的赋值运算符后跟着的表达式的格式错误" });
			//--word;
			to_end();
			//wd = "";//??
			wd.clear();
			return false;
		}
		else {//识别一个语句的结束标志‘;’
			line_order = tmp_line;
			//wd += word->second;//??
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"赋值语句的表达式后的内容为空白" });
				//wd = "";//??
				//wd.clear();
				return false;
			}
			else if (word->second != ";") {
				line_order = tmp_line;
				error.push_back({ line_order,"赋值语句不是以分号‘;’结尾" });
				//--word;
				to_end();
				//wd = "";//??
				wd.clear();
				return false;
			}
			else {
				line_order = tmp_line;
				expr.push_back({ start_line,wd });
				//wd = "";//??
				wd.clear();
				return true;
			}
		}
	}
}

bool Grammar_analyzer::is_condition() {//是不是一个条件语句，注意，这里已经判断出来了if开头，这里的语句判断不需要考虑语句的
	//最后的分号结束标志，关于分号结束标志的判断在is_stmt()里
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back(make_pair(line_order, "条件语句if后的部分为空"));
		return false;
	}
	else if (word->second != "(") {
		line_order = tmp_line;
		error.push_back(make_pair(line_order, "if语句右括号出错"));
		to_end();
		return false;
	}
	else {//表明if后面识别到了一个‘(’
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back(make_pair(line_order, "条件表达式‘if(’后面为空"));
			return false;
		}
		else if (!is_cond_expr()) {//跟在‘if(’后面的不是一个条件表达式，中这个expr可能是一个式子，也有可能是由多个式子用逻辑连接运算符连接起来的
			line_order = tmp_line;
			error.push_back(make_pair(line_order, "跟在‘if(’后面的不是一个条件表达式"));
			to_end();
			return false;
		}
		else {//表明识别到了一个条件表达式，到这一步目前为止if条件语句的格式正确
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"条件表达式后的内容为空" });
				return false;
			}
			else {//此时识别出了“if(expr)”，接着识别其后是否有语句，注意，我们此时的if不能带{}
				line_order = tmp_line;
				//++word;
				if (is_filter()) {
					error.push_back({ line_order,"if语句后除了完整的条件表达式外后面的内容为空" });
					return false;
				}
				else if (!is_stmt()) {
					line_order = tmp_line;
					error.push_back({ line_order,"完整的if表达式后跟着的内容不是一个语句" });
					to_end();
					return false;
				}
				else {//识别出了if(expr)stmt语句，要确定其还有没有else，注意：我们的else不包含{}，且else中最多只有一个语句，
					//而且我们也识别不了else if，就是这么菜
					line_order = tmp_line;

					//else {
					line_order = tmp_line;
					++word;
					if (is_filter()) {
						error.push_back({ line_order,"完整的if语句后面的内容为空" });
						return false;
					}
					else if (word->second != "else") {
						line_order = tmp_line;
						//error.push_back({ line_order,"完整的if语句后面跟着的不是else" });

						--word;
						return true;
					}
					else {//表明识别出了it(expr)stmt else
						line_order = tmp_line;

						wd.push_back(*word);
						expr.push_back({ line_order,wd });
						wd.clear();

						++word;
						if (is_filter()) {
							error.push_back({ line_order,"else后面的内容为空" });
							expr.erase(expr.begin() + expr.size() - 1, expr.begin());
							return false;
						}
						else if (!is_stmt()) {
							line_order = tmp_line;
							error.push_back({ line_order,"else中的语句格式错误" });
							expr.erase(expr.begin() + expr.size() - 1, expr.begin());
							to_end();
							return false;
						}
						else {
							line_order = tmp_line;

							//--word;
							return true;//判断出了一个完整的if-else语句。
						}
					}
					//}

				}
			}

		}
	}
}

bool Grammar_analyzer::is_loop() {//是不是一个循环语句
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"while循环语句中的while的后面的内容为空白" });
		return false;
	}
	else if (word->second != "(") {
		line_order = tmp_line;
		error.push_back({ line_order,"while循环语句缺少右括号‘(’" });
		//--word;
		to_end();
		return false;
	}
	else {
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"while循环语句右括号的后面的内容为空白" });
			return false;
		}
		else if (!is_cond_expr()) {
			line_order = tmp_line;
			error.push_back({ line_order,"while循环语句条件表达式的部分不正确" });
			//--word;
			to_end();
			return false;
		}
		else {
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"while循环语句条件表达式后面的内容为空" });
				return false;
			}
			/*else if (word->second != ")") {
				line_order = tmp_line;
				error.push_back({ line_order,"while循环语句的条件表达式缺少括号" });
				//--word;
				to_end();
				return false;
			}*/
			else {
				line_order = tmp_line;
				//++word;
				if (is_filter()) {
					error.push_back({ line_order,"while循环语句完整的条件表达式后面的内容为空" });
					return false;
				}
				else if (!is_stmt()) {
					line_order = tmp_line;
					error.push_back({ line_order,"while循环语句循环体不符合语句格式" });
					//--word;
					to_end();
					return false;
				}
				else {
					line_order = tmp_line;
					return true;//while循环体的stmt语句里自带分号结束标志
					/*if (is_filter()) {
						error.push_back({ line_order,"while循环语句的循环体部分后面的内容为空白" });
						return false;
					}
					else if (word->second != ";") {
						line_order = tmp_line;
						error.push_back({ line_order,"while循环语句循环体的结束标记不是分号‘;’" });
						//--word;
						return false;
					}
					else {
						return true;
					}*/
				}
			}
		}
	}
}

bool Grammar_analyzer::is_func() {//是不是一个过程调用语句
	line_order = tmp_line;
	++word;
	if (is_filter()) {
		error.push_back({ line_order,"过程调用语句中的call关键字后的内容为空白" });
		return false;
	}
	else if (word->first != 400) {//call后面跟着的不是一个标识符
		line_order = tmp_line;
		error.push_back({ line_order,"过程调用语句的call关键字后面跟着的内容不是一个标识符" });

		to_end();//###可能出错

		return false;
	}
	else {//call后面跟着的单词是一个标识符
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"过程调用语句中的标识符后面的内容为空白" });
			//to_end();###
			return false;
		}
		else if (word->second != ";") {
			error.push_back({ line_order,"语句不是以分号‘;’结尾" });
			to_end();
			return false;
		}
		else {
			return true;
		}
	}
}

bool Grammar_analyzer::is_compound() {//是不是一个复合语句
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"复合语句的begin关键字后的内容为空白" });
		return false;
	}
	else if (!is_stmt_list_2()) {
		line_order = tmp_line;
		error.push_back({ line_order,"复合语句格式不正确或语句列表格式不正确" });
		to_end();
		return false;
	}
	else {//识别到了语句列表
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"复合语句的语句列表后面的内容为空白" });
			return false;
		}
		else if (word->second != "end") {
			line_order = tmp_line;
			error.push_back({ line_order,"复合语句不以end结尾" });

			//###可能会出错

			return false;
		}
		else {
			//++word;
			expr.push_back({ line_order,vector<pair<size_t, string> >(1,*word) });
			return true;
		}
	}
}

bool Grammar_analyzer::is_stmt_list_2() {//是不是一个语句列表，只要能到这一个函数，那么此时的word一定不会是过滤符
	while (word->second != "end") {
		line_order = tmp_line;
		bool r = is_stmt();

		if (word != p->cend()) {
			if (word->second == "end" && !r) {//表明此时的刚刚判断过的那个语句不是复合语句
				--word;
				return true;
			}//###
			++word;
		}

		if (is_filter() || word->second == "procedure" || word->second == "end.") {
			error.push_back({ line_order,"复合语句缺少end结束标记" });

			--word;

			return false;
		}
	}

	--word;
	return true;
}

bool Grammar_analyzer::is_cond_expr() {//是不是一个条件表达式
	line_order = tmp_line;
	size_t start_line = line_order;
	if (is_relation()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"条件表达式后面的内容为空" });
			return false;
		}
		else if (word->second == ")") {//以右括号‘)’作为条件表达式的结束标记
			line_order = tmp_line;
			expr.push_back({ start_line,wd });
			wd.clear();
			return true;
		}
		else if (word->second == "&&" || word->second == "||" || word->second == "and" || word->second == "or") {
			line_order = tmp_line;
			wd.push_back(*word);
			++word;
			if (is_filter()) {
				return false;
			}
			return is_cond_expr();//注意此处的递归调用
		}
		else {
			line_order = tmp_line;
			//error.push_back({ line_order,"条件表达式出现错误" });
			wd.clear();
			return false;
		}
	}
	else {
		line_order = tmp_line;
		//error.push_back({ line_order,"条件语句的条件表达式错误" });
		wd.clear();
		return false;
	}
}

bool Grammar_analyzer::is_relation() {//是不是一个关系表达式，因为关系表达式可以构成条件表达式
	line_order = tmp_line;

	size_t start_line = line_order;

	if (is_expr()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"条件表达式的后面内容为空" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (!is_cmp_operators()) {//当前单词不是关系运算符
			line_order = tmp_line;
			//wd = "";//??
			//wd.clear();
			//error.push_back({ line_order,"条件表达式的关系运算符不正确" });
			return false;
		}
		else {
			line_order = tmp_line;
			//wd += word->second;//??
			wd.push_back(*word);
			++word;
			if (is_filter()) {
				//error.push_back({ line_order,"条件表达式中关系运算符的后面内容为空" });
				//wd = "";//??
				return false;
			}
			else if (!is_expr()) {
				line_order = tmp_line;
				//error.push_back({ line_order,"条件表达式的第二个表达式错误" });
				//wd = "";//??
				//wd.clear();
				return false;
			}
			else {
				line_order = tmp_line;
				//expr.push_back({ start_line,wd });
				//wd = "";//??
				//wd.clear();
				return true;
			}
		}
	}
	else {
		line_order = tmp_line;
		//error.push_back({ line_order,"条件语句的条件表达式错误" });
		//wd = "";//??
		//wd.clear();
		return false;
	}
}

bool Grammar_analyzer::is_expr() {//是不是表达式
	line_order = tmp_line;
	if (is_item()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"表达式中项的后面内容为空" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (word->second != "+" && word->second != "-") {//我们认为此时表达式结束，即这个表达式只由一个项构成
			//line_order = tmp_line;
			//error.push_back({ line_order,"表达式的项的连接运算符不是加或减" });
			line_order = tmp_line;

			--word;//??
			return true;
		}//针对此处进行测试，即单个标识符后跟一些其他字符用作条件表达式
		else {
			bool r;

			while (word->second == "+" || word->second == "-") {
				//wd += word->second;//??
				wd.push_back(*word);
				r = with_item();
				line_order = tmp_line;
				++word;

				if (!r) {
					//error.push_back({ line_order,"含有连接运算符的表达式错误" });
					//wd = "";//??
					wd.clear();
					return false;
				}
			}
			if (is_filter()) {
				//error.push_back({ line_order,"含有连接运算符的表达式的后面的内容为空" });
				//wd = "";//??
				wd.clear();
				return false;
			}
			else if (is_cmp_operators() || word->second == ")" || word->second == ";"
				|| word->second == "&&" || word->second == "||" || word->second == "and" || word->second == "or") {
				line_order = tmp_line;


				--word;
				return true;//检测到了识别终点
			}
			else {
				line_order = tmp_line;
				//wd = "";//??
				wd.clear();
				//error.push_back({ line_order,"项的结束标记出错" });###
				return false;
			}
		}
	}
	else {
		//error.push_back({ line_order,"表达式的组成部分不是项" });
		line_order = tmp_line;
		//wd = "";//???
		return false;
	}
}

bool Grammar_analyzer::is_item() {//是不是项
	line_order = tmp_line;
	if (is_factor()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"项中的因子的后面内容为空" });
			return false;
		}
		else if (word->second != "*" && word->second != "/") {//表明这个项中只有一个因子（这里可能出错）
			//line_order = tmp_line;
			//error.push_back({ line_order,"项中的因子的连接运算符不是乘或除" });
			line_order = tmp_line;


			--word;
			return true;
		}
		else {
			bool r;
			line_order = tmp_line;
			while (word->second == "*" || word->second == "/") {
				//wd += word->second;//??
				wd.push_back(*word);
				r = with_factor();
				line_order = tmp_line;
				if (!r) {
					//error.push_back({ line_order,"含有连接运算符的项错误" });
					//wd = "";//??
					return false;
				}
				++word;
			}
			if (is_filter()) {
				//error.push_back({ line_order,"含有连接运算符的项的后面的内容为空" });
				//wd = "";//??
				return false;
			}
			else if (word->second == "+" || word->second == "-" || is_cmp_operators() || is_separater()
				|| word->second == "&&" || word->second == "||" || word->second == "and" || word->second == "or") {
				line_order = tmp_line;


				--word;
				return true;//检测到了识别终点
			}
			else {
				line_order = tmp_line;
				//error.push_back({ line_order,"项的结束标记出错" });
				//wd = "";//??
				return false;
			}
		}
	}
	else {
		//error.push_back({ line_order,"表达式的组成部分不是项" });
		line_order = tmp_line;
		//wd = "";//??
		return false;
	}
}

bool Grammar_analyzer::is_factor() {//是不是因子
	switch (word->first) {
	case 400: {//标识符
		//wd += word->second;//??
		wd.push_back(*word);
		return true;
	}
			break;
	case 450: {//常数
		//wd += word->second;//??
		wd.push_back(*word);
		return true;
	}
			break;
	default: {//其他（包括带一对括号的表达式）
		if (word->second != "(") {
			line_order = tmp_line;
			//error.push_back({ line_order,"应该是因子的地方不是因子" });
			//wd = "";//??
			return false;
		}
		else {
			//wd += word->second;//??
			wd.push_back(*word);
			++word;
			line_order = tmp_line;
			if (is_filter()) {
				//error.push_back({ line_order,"因子中的‘(’后面的内容为空" });
				//wd = "";//??
				return false;
			}
			else if (!is_expr()) {
				line_order = tmp_line;
				//error.push_back({ line_order,"对应的带括号因子中的表达式格式错误" });
				//wd = "";//??
				return false;
			}
			else {
				line_order = tmp_line;
				//wd += word->second;//??
				//wd.push_back(*word);
				++word;
				if (is_filter()) {
					//error.push_back({ line_order,"因子中的带括号的表达式的后面的内容为空" });
					//wd = "";//??
					return false;
				}
				else if (word->second != ")") {
					line_order = tmp_line;
					//error.push_back({ line_order,"因子的带括号的表达式的右侧括号变更" });
					//wd = "";//??
					return false;
				}
				else {
					line_order = tmp_line;
					//wd += word->second;//??
					wd.push_back(*word);
					return true;
				}
			}
		}
	}
	}
}

bool Grammar_analyzer::with_item() {//是不是有连接符连接的项，由is_expr调用
	line_order = tmp_line;
	if (word->second == "+" || word->second == "-") {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"表达式连接运算符后面的内容为空" });
			//wd = "";
			return false;
		}
		else if (!is_item()) {
			line_order = tmp_line;
			//wd == "";
			//error.push_back({ line_order,"表达式的连接运算符后面的项的格式不正确" });
			return false;
		}
		else {
			line_order = tmp_line;
			return true;
		}
	}
	else {
		line_order = tmp_line;
		//error.push_back({ line_order,"表达式中的连接项的运算符错误" });
		//wd = "";
		return false;
	}
}

bool Grammar_analyzer::with_factor() {//是不是有连接符的因子
	line_order = tmp_line;
	if (word->second == "*" || word->second == "/") {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"表达式连接运算符后面的内容为空" });
			//wd = "";
			return false;
		}
		else if (!is_factor()) {
			line_order = tmp_line;
			//wd = "";
			//error.push_back({ line_order,"表达式的连接运算符后面的项的格式不正确" });
			return false;
		}
		else {
			line_order = tmp_line;
			return true;
		}
	}
	else {
		line_order = tmp_line;
		//wd = "";
		//error.push_back({ line_order,"项中的连接项因子的运算符错误" });
		return false;
	}
}

void Grammar_analyzer::analyse() {//语法分析函数，每当其找出一个错误后，其还会接着找下去
	//首先从词法分析的表中读入第一个单词
	is_program();
}

bool Grammar_analyzer::is_cmp_operators() {//判断当前字符是不是关系运算符
	set<string>::const_iterator r = cmp_operators.find(word->second);
	if (r != cmp_operators.cend()) {
		return true;
	}
	else return false;
}

bool Grammar_analyzer::is_separater() {//是不是分隔符
	set<char>::const_iterator r = separater.find(word->second[0]);
	if (r != separater.cend()) {
		return true;
	}
	else {
		return false;
	}
}

bool Grammar_analyzer::is_put_operators() {//是不是赋值运算符
	set<string>::const_iterator r = put_operators.find(word->second);
	if (r != put_operators.cend()) {
		return true;
	}
	else {
		return false;
	}
}

void Grammar_analyzer::to_end() {//如果对一个语句的检测在中间过程出现了问题，我们就不对这个语句进行后续的检测，这个函数可以使得word读到
	//这个语句最近的分号的位置而如果没有分号，则一直前进到分程序的末尾end标记，如果没有end，那就前进到下一个分程序的开始
	//标记procedure或是读到table的cend()处
	while (word != p->cend()) {
		if (word->second == ";") {
			return;
		}
		else if (word->second == "end") {//会影响其后的复合语句的识别
			return;
		}
		else if (word->second == "end.") {
			--word;
			return;
		}
		else if (word->second == "procedure") {
			--word;
			return;
		}
		else {
			++word;
			if (word != p->cend() && word->second[0] == '\n') {//###可能出错
				++tmp_line;
			}
		}
	}
}
void Grammar_analyzer::show_inf()const {//信息展示函数

	if (p->empty()) {
		cout << "程序为空" << endl;
		return;
	}

	cout << "语法分析的结果为：" << endl;
	if (error.empty()) {
		cout << "该程序没有任何语法错误" << endl;
	}
	else {
		for (vector<pair<size_t, string> >::const_iterator it = error.cbegin(); it != error.cend(); ++it) {
			cout << "程序的错误如下：" << endl;
			cout << "出错的行数（仅供参考）：" << it->first << "     " << "出错的内容（仅供参考）：" << it->second << endl;
		}
	}

	if (define.empty()) {
		cout << "该程序没有任何定义语句" << endl;
	}
	else {
		cout << "定义的变量有：" << endl;
		int i = 1;
		for (set<string >::const_iterator it = define.cbegin(); it != define.cend(); ++it) {
			cout << "变量" << i << "为：" << *it << endl;
			++i;
		}
	}

	if (expr.empty()) {
		cout << "该程序没有任何表达式" << endl;
	}
	else {
		cout << "程序中的式子（赋值式、表达式等）如下：" << endl;
		for (vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it = expr.cbegin(); it != expr.cend(); ++it) {
			cout << "所在行数：" << it->first << "   " << "相应的表达式为：";
			for (vector<pair<size_t, string> >::const_iterator it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
				cout << it2->second << /*'(' << it2->first << ')' <<*/ ' ';
			}
			cout << endl;
		}
	}
}

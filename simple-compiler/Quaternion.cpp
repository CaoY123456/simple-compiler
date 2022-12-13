#include "Quaternion.h"
#include"Grammar_analyzer.h"
#include<stack>
using namespace std;

bool compare(const pair<size_t, string>& a, const pair<size_t, string>& b) {//a的位置的运算符存储的是运算符栈顶的运算符，而
	//b的位置的运算符代表的是当前扫描到的运算符，如果a的优先级高，则表明b不能放入栈中，返回false；若a的优先级更低，则表明b可以
	//放入栈中，返回true
	//注：比较运算符种别码为150，逻辑运算符种别码为200，赋值运算符种别码为300，算数运算符种别码为100，分隔运算符（包含括号）种别码为70
	//优先级比较为：分隔运算符 > 算数运算符 > 比较运算符 > 逻辑运算符 > 赋值运算符
	switch (a.first) {
	case 100:
		if (b.first > 100) {
			return false;//a的优先级更高
		}
		else {
			if (b.first == 100) {//表明是两个算数运算符（+、-、*、/、%）进行比较
				if (a.second == "+" || a.second == "-") {//如果a为+或-，
					if (b.second == "+" || b.second == "-") {//此时b不能直接放进去，false
						return false;
					}
					else {//此时b可以直接放进去，true
						return true;
					}
				}
				else if (a.second == "*" || a.second == "/") {//如果a为*或/
					if (b.second == "%") {//此时b可以直接放进去
						return true;
					}
					else {//此时b不可以直接放进去
						return false;
					}
				}
				else {//此时a为%,无论b为什么情况，其优先级肯定比a低
					return false;
				}
			}
			else {
				return true;//表明遇到了分隔符"("
			}
		}
		break;
	case 70: //a为分隔符"("，遇到右括号无条件入栈，此时要返回true
		return true;
		break;
	case 200: {//a为逻辑运算符，&&的优先级更高
		if (a.first < b.first) {//a的优先级高
			return false;
		}
		else if (a.first > b.first) {//b的优先级高
			return true;
		}
		else {
			if (a.second == "&&") {//a的优先级高
				return false;
			}
			else {
				if (b.second == "&&") {
					return true;
				}
				else {
					return false;
				}
			}
		}
	}
	default://a为比较运算符
		if (a.first <= b.first) {//b的优先级更低，此时不能放入b，false
			return false;
		}
		else {
			return true;
		}
	}

}

//size_t FourElements::_number = 1;//可能出错

//默认的构造函数：
FourElements::FourElements() :oper(""), var1(""), var2(""), aim(""), num(0) {}
//带参数的构造函数：
FourElements::FourElements(const string& op, const string& v1, const string& v2, const string& a, const size_t& n) :oper(op), var1(v1), var2(v2), aim(a), num(n) {}

//拷贝构造函数：
FourElements::FourElements(const FourElements& F) :oper(F.oper), var1(F.var1), var2(F.var2), aim(F.aim), num(F.num) {}
//拷贝赋值运算符
FourElements& FourElements::operator=(const FourElements& F) {
	oper = F.oper;
	var1 = F.var1;
	var2 = F.var2;
	aim = F.aim;

	num = F.num;

	return *this;
}

//带参数的构造函数：
Quaternion::Quaternion(const Grammar_analyzer& G) :define(make_shared<set<string> >(G.define)),
expr(make_shared<vector<pair<size_t, vector<pair<size_t, string> > > > >(G.expr)) {}

void Quaternion::construct() {//构造四元分析式的成员函数，确保其中的每个变量都是定义过的或者是依据顺序生成的中间变量
	FourElements F;

	enum end_way {
		if_sem_end, if_end_end, if_wait_end,
		else_sem_end, else_end_end, else_wait_end,
		wh_sem_end, wh_end_end, wh_wait_end
	};//语句体结尾的方式：单一语句，sem_end：以分号结尾；end_end：复合语句，以end结尾
	//wait_end：有时候if条件表达式下紧跟这一个while或if，此时不确定它们的结尾方式，待定。
	//最前面的if、else和wh则表示是if语句、else语句还是while语句的结尾

	size_t order = 1;//order为当前构造出的四元式的位序（从1开始）
	size_t tmp_order = 1;//用于标记产生的当前中间变量的位序

	stack<end_way>which_end;//哪种结尾方式的栈，用于含有条件表达式的语句中，压入对应的结尾方式
	stack<pair<size_t, end_way> >adjust_aim_end;//等待调整的跳转语句的在fe中的位序和结束方式的集合，此种跳转属于跳转到所要执行的
	//的语句体结束之后的下一条四元式语句
	stack<size_t>aim_end_order;//上述每个需要调整的语句对应的四元式跳转位于的语句在expr中的位置。为了保证当条件表达式中的“与”
	//运算符前面的那个比较表达式跳转正确，即如果这个条件表达式中有“||”，只对于位于同一个条件表达式的比较表达式的四元式之间进行跳转，
	//因为根据设计的方法，会产生不同条件表达式语句之间的错误跳转，我必须将其局限在一个条件表达式中。

	stack<size_t>adjust_or_aim;//等待调整的跳转语句的在fe中的位序的集合，针对的是遇到逻辑运算符“或”的情况，因为当或前面的表达式正确时，直接跳转到
	//语句体开始的四元式处执行，即为条件表达式的四元式结束之后
	stack<pair<size_t, end_way> >adjust_and_aim;//等待调整的跳转语句的在fe中的位序的集合，针对的是遇到逻辑运算符“与”的情况，
	//因为当或前面的表达式错误时，直接跳转到后面所遇到的第一个“或”运算符的后面的第一个四元式，如果其后面没有或运算符，则跳转到语句的结束

	stack<pair<size_t, end_way> >wh_start_aim;//用于存储while语句的最开始语句代表的四元式的位序的栈，pair的first对象即存储前面所述的事物，
	//而second成员存储while语句体的结束方式标记

	stack<bool >bg;//如果这个begin是紧跟在if、else或者是while后面的，则入栈为true，反之则为else，为了解决多个复合语句嵌套时产生的begin-end
	//匹配错误

	for (vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it = expr->cbegin(); it != expr->cend(); ++it) {//对expr表达式中存储的每个表达式进行遍历
		//目前，expr包含的表达式的种类有“关键字if、else、begin、end、while含有逻辑运算符的条件表达式、赋值形式的运算语句”
		stack<string>name;//中间运算数栈（可以为常数或已经定义的变量）
		stack<pair<size_t, string> >op;//运算符栈，只有待压入的运算符的优先级比栈顶的运算符的优先级高能将运算符压入栈，涉及到的运算符有：
		//（按优先级从高到低）左右括号>乘除号>加减号>比较运算符>逻辑运算符>赋值运算符，但赋值运算符不会与比较和逻辑运算符相遇
		//上述两个栈是为了实现合理的中间运算顺序而设置的
		stack<pair<size_t, end_way> >adjust_and_or_aim;//专门用于当前的条件语句（如果是条件语句就能派上用场，否则不行），如果“与”运算符
		//后还跟着或运算符，则进行相应跳转语句的调整

		switch (it->second[0].first) {//每个表达式的第一个字符可以区分出该表达式的类型
		case 1: {//if类型的条件语句，涉及到比较运算符、逻辑运算符、以及+-*/的处理
			//it往前看一个，判断该if语句体的结尾类型，以实现相关语句的正确跳转
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//临时存储向前看的迭代器
			switch (it2->second[0].first) {
			case 3:
			case 1: {//表明if条件表达式其后紧跟着的是一个if条件语句或while条件语句
				which_end.push(if_wait_end);//结尾方式待定，不确定
			}
				  break;
			case 49: {//表明if条件表达式其后紧跟着的是一个复合语句，以end结尾
				which_end.push(if_end_end);
			}
				   break;
			case 400: {//表明if条件表达式其后跟着的是一个单一的语句，以分号结尾，但是分号没有被我们读进来
				which_end.push(if_sem_end);
			}
					break;
			}

			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin() + 1; k != it->second.cend(); ++k) {//对表达式的每个成分进行遍历
				switch (k->first) {
				case 450://遇到的是常数
				case 400: {//遇到的运算数
					name.push(k->second);

					if (k->first == 400) {//如果是一个变量，则要检测其有没有被定义过
						if (define->find(k->second) == define->cend()) {//表明此时使用的变量未定义过，记录变量未定义错误，
							//不影响四元式的构建
							error.push_back({ it->first,k->second });//记录该变量所在的行数和该变量的具体的名字
						}
					}

				}
						break;
				case 70: {//遇到的是左右括号，左括号无条件进栈，右括号则一直弹出运算符栈进行运算直到遇到第一个左括号为止
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						vector<pair<size_t, string> >::const_iterator k2 = k + 1;
						if (k2 != it->second.cend()) {
							if (k2->first == 200) {//表明括号中可能含有比较运算符和逻辑运算符，且括号外紧跟着一个逻辑运算符，此时要将k自增，使其
								++k;//指向那个逻辑运算符，如果不是这种情况的话则k不需要自增
							}
						}
						while (op.top().first != 70) {//在条件表达式的左右括号中可能出现逻辑运算符、比较运算符、算数运算符，但是逻辑运算符不放入
							switch (op.top().first) {//运算符栈op，所以此时只有可能出现比较运算符和算数运算符
							case 150: {//当前运算符栈顶的运算符为比较运算符
								F.oper = "j";
								F.oper += op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.num = order++;

								F.aim = "wait";

								fe.push_back(F);
								//上面为带比较运算符的表达式正确时的跳转情况，下面还要进行如果该带比较运算符的条件表达式错误
								//的情况的四元式的跳转情况的构建

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//处理到这种情况（运算符栈顶为比较运算符），肯定当前遇到的是比比较运算符优先级低的逻辑运算符,
								//所以要考虑遇到的是"&&"还是"||"
								if (k->second == "&&" || k->second == "and") {//当前待放入的为与运算符
									fe[fe.size() - 2].aim = to_string(order);
									//“与运算符”前的比较表达式不成立则跳到下一个比较表达式的开始
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//暂时不写，用wait替代

									//“与运算符”前的比较表达式不成立则跳出if的语句体部分，跳到外面或else的语句体中
								}
								else {//当前待放入的为或运算符
									adjust_or_aim.push(fe.size() - 2);
									//暂时不写，用wait替代

									//“或运算符”前的比较表达式成立则跳到if的语句体开始部分，
									fe[fe.size() - 1].aim = to_string(order);
									//“或运算符”前的比较表达式不成立则跳到下一个比较表达式的开始

									//可能这个“或”运算符前面还有若干个与运算符，我们要在此时调整与运算符不符合表达式的跳转情况
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}
							}
									break;
							case 100: {//当前运算符栈顶的运算符为算数运算符
								F.oper = op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.aim = "T";
								F.aim += to_string(tmp_order++);

								F.num = order++;
								fe.push_back(F);

								name.push(F.aim);
							}
									break;
							}

							op.pop();

						}
						op.pop();
					}
				}
					   break;
				default: {
					if (op.empty()) {
						op.push(*k);
					}
					else {
						while (!compare(op.top(), *k)) {//if条件表达式中可能出现的其他运算符有算数运算符、比较运算符
							//逻辑运算符
							switch (op.top().first) {
							case 150: {//可能出现比较运算符，此时当前遇到的必然是一个逻辑运算符，注意，逻辑运算符仅是控制语句的跳转，我们不对其
								F.oper = "j";//进行入栈处理
								F.oper += op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.num = order++;

								F.aim = "wait";

								fe.push_back(F);
								//上面为带比较运算符的表达式正确时的跳转情况，下面还要进行如果该带比较运算符的条件表达式错误
								//的情况的四元式的跳转情况的构建

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//处理到这种情况（运算符栈顶为比较运算符），肯定当前遇到的是比比较运算符优先级低的逻辑运算符,
								//所以要考虑遇到的是"&&"还是"||"
								if (k->second == "&&" || k->second == "and") {//当前待放入的为与运算符
									fe[fe.size() - 2].aim = to_string(order);
									//“与运算符”前的比较表达式不成立则跳到下一个比较表达式的开始
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//暂时不写，用wait替代

									//“与运算符”前的比较表达式不成立则跳出if的语句体部分，跳到外面或else的语句体中
								}
								else {//当前待放入的为或运算符
									adjust_or_aim.push(fe.size() - 2);
									//暂时不写，用wait替代

									//“或运算符”前的比较表达式成立则跳到if的语句体开始部分，
									fe[fe.size() - 1].aim = to_string(order);
									//“或运算符”前的比较表达式不成立则跳到下一个比较表达式的开始

									//可能这个“或”运算符前面还有若干个与运算符，我们要在此时调整与运算符不符合表达式的跳转情况
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}

							}
									break;

							case 100: {//可能出现算数运算符
								F.oper = op.top().second;
								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.aim = "T";
								F.aim += to_string(tmp_order++);

								F.num = order++;
								fe.push_back(F);
								name.push(F.aim);

							}
									break;
							}

							op.pop();
							if (op.empty()) {
								break;
							}
						}
						if (k->first != 200)
							op.push(*k);

					}
				}
				}
			}

			while (!op.empty()) {//此时会出现处理比较运算符和算数运算符的情况
				switch (op.top().first) {//可能会遇到算数运算符、比较运算符、逻辑运算符
				case 150: {//比较运算符
					F.oper = "j";
					F.oper += op.top().second;

					if (!name.empty()) {
						F.var2 = name.top();
						name.pop();
					}
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.num = order++;

					F.aim = to_string(order + 1);

					fe.push_back(F);

					//上面为比较表达式符合四元式的跳转情况，下面开始写若不符合四元式的跳转情况
					F.oper = "j";
					F.var2 = " ";
					F.var1 = " ";
					F.num = order++;

					F.aim = "wait";

					fe.push_back(F);
					adjust_aim_end.push({ fe.size() - 1,which_end.top() });
					aim_end_order.push(it - expr->cbegin());
				}
						break;
				case 100: {//此时的运算符为算数运算符
					F.oper = op.top().second;
					if (!name.empty()) {
						F.var2 = name.top();
						name.pop();
					}
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.aim = "T";
					F.aim += to_string(tmp_order++);

					F.num = order++;
					fe.push_back(F);
					name.push(F.aim);
				}
						break;
				}

				op.pop();
			}

			while (!adjust_or_aim.empty()) {//对可能遇到的条件表达式中出现逻辑运算符“或”的跳转情况进行调整，是在这个条件表达式完成初步四元式
				//创建以后就可以进行的
				fe[adjust_or_aim.top()].aim = to_string(order);
				adjust_or_aim.pop();
			}

		}
			  break;










		case 2: {//else类型的的条件语句的后半截，一定前面已经有一个完整的if语句，仅当执行完else语句前面的if语句体中的内容时，
			//根据if-else语句的规则，则需要跳过else的全部语句体执行，此处就是为了构造那个跳过else全部语句体的四元式的

			//先判断else语句体的结束标记是什么：
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//临时存储向前看的迭代器
			switch (it2->second[0].first) {
			case 3:
			case 1: {//表明else的语句体跟着的是一个if条件语句或while条件语句
				which_end.push(else_wait_end);//结尾方式待定，不确定
			}
				  break;
			case 49: {//表明if条件表达式其后紧跟着的是一个复合语句，以end结尾
				which_end.push(else_end_end);
			}
				   break;
			case 400: {//表明if条件表达式其后跟着的是一个单一的语句，以分号结尾，但是分号没有被我们读进来
				which_end.push(else_sem_end);
			}
					break;
			}

			F.oper = "j";
			F.var2 = " ";
			F.var1 = " ";
			F.num = order++;

			F.aim = "wait";

			fe.push_back(F);
			adjust_aim_end.push({ fe.size() - 1,which_end.top() });
			aim_end_order.push(it - expr->cbegin());
		}
			  break;








		case 3: {//while类型的条件语句，涉及到比较运算符、逻辑运算符、以及+-*/的处理
			//it往前看一个，判断该if语句体的结尾类型，以实现相关语句的正确跳转
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//临时存储向前看的迭代器
			switch (it2->second[0].first) {
			case 3:
			case 1: {//表明while条件表达式其后紧跟着的是一个if条件语句或while条件语句
				which_end.push(wh_wait_end);//结尾方式待定，不确定
			}
				  break;
			case 49: {//表明while条件表达式其后紧跟着的是一个复合语句，以end结尾
				which_end.push(wh_end_end);
			}
				   break;
			case 400: {//表明while条件表达式其后跟着的是一个单一的语句，以分号结尾，但是分号没有被我们读进来
				which_end.push(wh_sem_end);
			}
					break;
			}

			wh_start_aim.push({ order,which_end.top() });//记录while语句开始时四元式的位序，以方便调整四元式的跳转语句以实现循环

			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin() + 1; k != it->second.cend(); ++k) {//对表达式的每个成分进行遍历
				switch (k->first) {
				case 450://遇到的是常数
				case 400: {//遇到的运算数
					name.push(k->second);

					if (k->first == 400) {//如果是一个变量，则要检测其有没有被定义过
						if (define->find(k->second) == define->cend()) {//表明此时使用的变量未定义过，记录变量未定义错误，
							//不影响四元式的构建
							error.push_back({ it->first,k->second });//记录该变量所在的行数和该变量的具体的名字
						}
					}

				}
						break;
				case 70: {//遇到的是左右括号，左括号无条件进栈，右括号则一直弹出运算符栈进行运算直到遇到第一个左括号为止
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						vector<pair<size_t, string> >::const_iterator k2 = k + 1;
						if (k2 != it->second.cend()) {
							if (k2->first == 200) {//表明括号中可能含有比较运算符和逻辑运算符，且括号外紧跟着一个逻辑运算符，此时要将k自增，使其
								++k;//指向那个逻辑运算符，如果不是这种情况的话则k不需要自增
							}
						}
						while (op.top().first != 70) {//在条件表达式的左右括号中可能出现逻辑运算符、比较运算符、算数运算符，但是逻辑运算符不放入
							switch (op.top().first) {//运算符栈op，所以此时只有可能出现比较运算符和算数运算符
							case 150: {//当前运算符栈顶的运算符为比较运算符
								F.oper = "j";
								F.oper += op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.num = order++;

								F.aim = "wait";

								fe.push_back(F);
								//上面为带比较运算符的表达式正确时的跳转情况，下面还要进行如果该带比较运算符的条件表达式错误
								//的情况的四元式的跳转情况的构建

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//处理到这种情况（运算符栈顶为比较运算符），肯定当前遇到的是比比较运算符优先级低的逻辑运算符,
								//所以要考虑遇到的是"&&"还是"||"
								if (k->second == "&&" || k->second == "and") {//当前待放入的为与运算符
									fe[fe.size() - 2].aim = to_string(order);
									//“与运算符”前的比较表达式不成立则跳到下一个比较表达式的开始
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//暂时不写，用wait替代

									//“与运算符”前的比较表达式不成立则跳出if的语句体部分，跳到外面或else的语句体中
								}
								else {//当前待放入的为或运算符
									adjust_or_aim.push(fe.size() - 2);
									//暂时不写，用wait替代

									//“或运算符”前的比较表达式成立则跳到if的语句体开始部分，
									fe[fe.size() - 1].aim = to_string(order);
									//“或运算符”前的比较表达式不成立则跳到下一个比较表达式的开始

									//可能这个“或”运算符前面还有若干个与运算符，我们要在此时调整与运算符不符合表达式的跳转情况
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}
							}
									break;
							case 100: {//当前运算符栈顶的运算符为算数运算符
								F.oper = op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.aim = "T";
								F.aim += to_string(tmp_order++);

								F.num = order++;
								fe.push_back(F);

								name.push(F.aim);
							}
									break;
							}

							op.pop();

						}
						op.pop();
					}
				}
					   break;
				default: {
					if (op.empty()) {
						op.push(*k);
					}
					else {
						while (!compare(op.top(), *k)) {//while条件表达式中可能出现的其他运算符有算数运算符、比较运算符
							//逻辑运算符
							switch (op.top().first) {
							case 150: {//可能出现比较运算符，此时当前遇到的必然是一个逻辑运算符，注意，逻辑运算符仅是控制语句的跳转，我们不对其
								F.oper = "j";//进行入栈处理
								F.oper += op.top().second;

								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.num = order++;

								F.aim = "wait";

								fe.push_back(F);
								//上面为带比较运算符的表达式正确时的跳转情况，下面还要进行如果该带比较运算符的条件表达式错误
								//的情况的四元式的跳转情况的构建

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//处理到这种情况（运算符栈顶为比较运算符），肯定当前遇到的是比比较运算符优先级低的逻辑运算符,
								//所以要考虑遇到的是"&&"还是"||"
								if (k->second == "&&" || k->second == "and") {//当前待放入的为与运算符
									fe[fe.size() - 2].aim = to_string(order);
									//“与运算符”前的比较表达式不成立则跳到下一个比较表达式的开始
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//暂时不写，用wait替代

									//“与运算符”前的比较表达式不成立则跳出if的语句体部分，跳到外面或else的语句体中
								}
								else {//当前待放入的为或运算符
									adjust_or_aim.push(fe.size() - 2);
									//暂时不写，用wait替代

									//“或运算符”前的比较表达式成立则跳到if的语句体开始部分，
									fe[fe.size() - 1].aim = to_string(order);
									//“或运算符”前的比较表达式不成立则跳到下一个比较表达式的开始

									//可能这个“或”运算符前面还有若干个与运算符，我们要在此时调整与运算符不符合表达式的跳转情况
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}

							}
									break;

							case 100: {//可能出现算数运算符
								F.oper = op.top().second;
								if (!name.empty()) {
									F.var2 = name.top();
									name.pop();
								}
								if (!name.empty()) {
									F.var1 = name.top();
									name.pop();
								}
								F.aim = "T";
								F.aim += to_string(tmp_order++);

								F.num = order++;
								fe.push_back(F);
								name.push(F.aim);

							}
									break;
							}

							op.pop();
							if (op.empty()) {
								break;
							}
						}
						if (k->first != 200)
							op.push(*k);

					}
				}
				}
			}

			while (!op.empty()) {//此时会出现处理比较运算符和算数运算符的情况
				switch (op.top().first) {//可能会遇到算数运算符、比较运算符、逻辑运算符
				case 150: {//比较运算符
					F.oper = "j";
					F.oper += op.top().second;

					if (!name.empty()) {
						F.var2 = name.top();
						name.pop();
					}
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.num = order++;

					F.aim = to_string(order + 1);

					fe.push_back(F);

					//上面为比较表达式符合四元式的跳转情况，下面开始写若不符合四元式的跳转情况
					F.oper = "j";
					F.var2 = " ";
					F.var1 = " ";
					F.num = order++;

					F.aim = "wait";

					fe.push_back(F);
					adjust_aim_end.push({ fe.size() - 1,which_end.top() });
					aim_end_order.push(it - expr->cbegin());
				}
						break;
				case 100: {//此时的运算符为算数运算符
					F.oper = op.top().second;
					if (!name.empty()) {
						F.var2 = name.top();
						name.pop();
					}
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.aim = "T";
					F.aim += to_string(tmp_order++);

					F.num = order++;
					fe.push_back(F);
					name.push(F.aim);
				}
						break;
				}

				op.pop();
			}

			while (!adjust_or_aim.empty()) {//对可能遇到的条件表达式中出现逻辑运算符“或”的跳转情况进行调整，是在这个条件表达式完成初步四元式
				//创建以后就可以进行的
				fe[adjust_or_aim.top()].aim = to_string(order);
				adjust_or_aim.pop();
			}
		}
			  break;








		case 400: {//赋值运算语句，仅涉及+-*/的处理，所有语句的最终落脚点
			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin(); k != it->second.cend(); ++k) {//对表达式的每个成分进行遍历
				switch (k->first) {
				case 450://遇到的是常数
				case 400: {//遇到的运算数
					name.push(k->second);

					if (k->first == 400) {//如果是一个变量，则要检测其有没有被定义过
						if (define->find(k->second) == define->cend()) {//表明此时使用的变量未定义过，记录变量未定义错误，
							//不影响四元式的构建
							error.push_back({ it->first,k->second });//记录该变量所在的行数和该变量的具体的名字
						}
					}

				}
						break;
				case 70: {//遇到的是左右括号，左括号无条件进栈，右括号则一直弹出运算符栈进行运算直到遇到第一个左括号为止
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						while (op.top().first != 70) {//赋值运算式中的括号中的运算符只有算数运算符，不涉及其他运算符
							F.oper = op.top().second;

							if (!name.empty()) {
								F.var2 = name.top();
								name.pop();
							}
							if (!name.empty()) {
								F.var1 = name.top();
								name.pop();
							}
							F.aim = "T";
							F.aim += to_string(tmp_order++);

							F.num = order++;
							fe.push_back(F);

							name.push(F.aim);

							op.pop();

							if (op.empty()) {
								break;
							}
						}
						op.pop();
					}
				}
					   break;
				default: {
					if (op.empty()) {
						op.push(*k);
					}
					else {
						while (!compare(op.top(), *k)) {//赋值运算式中只有赋值运算符和算数运算符，又因为赋值运算符的优先级
							//最低，所以不会存在此时处理赋值运算符的情况
							pair<size_t, string>ing_op = op.top();//提取当前运算符栈顶的运算符
							op.pop();
							F.oper = ing_op.second;

							if (!name.empty()) {
								F.var2 = name.top();
								name.pop();
							}
							if (!name.empty()) {
								F.var1 = name.top();
								name.pop();
							}
							F.aim = "T";
							F.aim += to_string(tmp_order++);

							F.num = order++;
							fe.push_back(F);
							name.push(F.aim);

							if (op.empty()) {
								break;
							}
						}
						op.push(*k);
					}
				}
				}
			}

			while (!op.empty()) {//此时会出现处理赋值运算符和算数运算符的情况
				F.oper = op.top().second;

				switch (op.top().first) {
				case 300: {//此时的运算符为赋值运算符
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.var2 = " ";
					if (!name.empty()) {
						F.aim = name.top();
						name.pop();
					}
					F.num = order++;

					fe.push_back(F);

				}
						break;
				case 100: {//此时的运算符为算数运算符
					if (!name.empty()) {
						F.var2 = name.top();
						name.pop();
					}
					if (!name.empty()) {
						F.var1 = name.top();
						name.pop();
					}
					F.aim = "T";
					F.aim += to_string(tmp_order++);

					F.num = order++;
					fe.push_back(F);
					name.push(F.aim);
				}
						break;
				}

				op.pop();
			}

			if (!which_end.empty()) {
				switch (which_end.top()) {
				case if_sem_end: {//if中的结尾标记
					vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == if_sem_end) {
						if (it3 != expr->cend()) {
							if (it3->second[0].second == "else") {
								fe[adjust_aim_end.top().first].aim = to_string(order + 1);//当if语句后面还有一个else语句时，要跳过if语句体应该
							}
							else {
								fe[adjust_aim_end.top().first].aim = to_string(order);
							}
						}
						else {
							fe[adjust_aim_end.top().first].aim = to_string(order);
						}
						adjust_aim_end.pop();
						aim_end_order.pop();
					}

					while (!adjust_and_aim.empty() && adjust_and_aim.top().second == if_sem_end) {
						if (it3 != expr->cend()) {
							if (it3->second[0].second == "else") {
								fe[adjust_and_aim.top().first].aim = to_string(order + 1);//当if语句后面还有一个else语句时，要跳过if语句体应该
							}
							else {
								fe[adjust_and_aim.top().first].aim = to_string(order);
							}
						}
						else {
							fe[adjust_and_aim.top().first].aim = to_string(order);
						}
						adjust_and_aim.pop();
					}
					which_end.pop();
				}
							   break;
				case else_sem_end: {//else中的结尾标记
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == else_sem_end) {//用于调整if-else语句if语句体结束时的跳转情况
						fe[adjust_aim_end.top().first].aim = to_string(order);
						adjust_aim_end.pop();
						aim_end_order.pop();
					}

					/*while (adjust_and_aim.empty() && adjust_and_aim.top().second == else_sem_end) {
						fe[adjust_and_aim.top().first].aim = to_string(order);//应该用不到
					}*/
					which_end.pop();
				}
								 break;
				case wh_sem_end: {//while语句中的结尾标记
					//用于调整while的条件表达式不满足的情况下跳过while语句体执行的情况以及返回开始执行的情况（目前只写了不满足跳过执行的情况）
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == wh_sem_end) {//用于调整if-else语句if语句体结束时的跳转情况
						fe[adjust_aim_end.top().first].aim = to_string(order + 1);
						adjust_aim_end.pop();
						aim_end_order.pop();

					}
					while (!adjust_and_aim.empty() && adjust_and_aim.top().second == which_end.top()) {//对其条件表达式中的&&最后一批没有在后面遇上
						//“或”运算符的与运算符不满足条件条件时的跳转情况的调整
						fe[adjust_and_aim.top().first].aim = to_string(order + 1);
						adjust_and_aim.pop();
					}
					which_end.pop();

					//while语句体结束之后所添加的跳转语句以实现循环
					F.oper = "j";
					F.var2 = " ";
					F.var1 = " ";
					F.num = order++;

					F.aim = to_string(wh_start_aim.top().first);
					wh_start_aim.pop();
					fe.push_back(F);

				}
							   break;
				}
			}


			while ((!which_end.empty()) && (which_end.top() == if_wait_end || which_end.top() == else_wait_end || which_end.top() == wh_wait_end)) {
				int i = 1;
				while ((!adjust_aim_end.empty()) &&
					adjust_aim_end.top().second == which_end.top()) {
					switch (adjust_aim_end.top().second) {
					case if_wait_end: {
						vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
						if (it3 != expr->cend()) {
							if (it3->second[0].second == "else") {
								fe[adjust_aim_end.top().first].aim = to_string(order + 1);
							}
							else {
								fe[adjust_aim_end.top().first].aim = to_string(order);
							}
						}
						else {
							fe[adjust_aim_end.top().first].aim = to_string(order);
						}
					}
									break;
					case else_wait_end: {
						fe[adjust_aim_end.top().first].aim = to_string(order);
					}
									  break;
					case wh_wait_end: {
						fe[adjust_aim_end.top().first].aim = to_string(order + i);//????
						++i;
					}
									break;
					}
					adjust_aim_end.pop();
					aim_end_order.pop();
				}

				//以下是对含有"与"运算符的条件表达式的跳转调整
				i = 1;
				while (!adjust_and_aim.empty() &&
					adjust_and_aim.top().second == which_end.top()) {//对其条件表达式中的&&最后一批没有在后面遇上
					//“或”运算符的与运算符不满足条件条件时的跳转情况的调整
					switch (adjust_and_aim.top().second) {
					case if_wait_end: {
						vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
						if (it3 != expr->cend()) {
							if (it3->second[0].second == "else") {
								fe[adjust_and_aim.top().first].aim = to_string(order + 1);
							}
							else {
								fe[adjust_and_aim.top().first].aim = to_string(order);
							}
						}
						else {
							fe[adjust_and_aim.top().first].aim = to_string(order);
						}
					}
									break;
									/*case else_wait_end: {
										fe[adjust_and_aim.top().first].aim = to_string(order);
									}
													  break;*///不一定有用
					case wh_wait_end: {
						fe[adjust_and_aim.top().first].aim = to_string(order + i);//????
						++i;
					}
									break;
					}
					//fe[adjust_and_aim.top().first].aim = to_string(order);

					adjust_and_aim.pop();
				}

				if (which_end.top() == wh_wait_end) {
					//while语句体结束之后所添加的跳转语句以实现循环
					F.oper = "j";
					F.var2 = " ";
					F.var1 = " ";
					F.num = order++;

					F.aim = to_string(wh_start_aim.top().first);
					wh_start_aim.pop();
					fe.push_back(F);
				}
				which_end.pop();//可能会出错
			}

		}
				break;







		case 49: {//复合语句的开头begin标记，即将开始处理复合语句，复合语句里可以包含一系列语句，比较棘手
			if (it == expr->cbegin()) {
				bg.push(false);
			}
			else {
				vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it - 1;//看一看begin前面的那个元素是什么？
				switch (it2->second[0].first) {
				case 1:
				case 2:
				case 3: {//如果begin的前面是if、else、while，true入栈
					bg.push(true);
				}
					  break;
				default:
					bg.push(false);
				}
			}
			continue;//估计要涉及到一些标志位的控制
		}
			   break;







		case 50: {//复合语句的结尾end标记，复合语句的结束标记，涉及到一些跳转语句的目的地的调整
			if (!bg.empty() && bg.top()) {
				if (!which_end.empty()) {
					switch (which_end.top()) {
					case if_end_end: {//if中的结尾标记
						vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
						size_t ing_order;
						if (!aim_end_order.empty()) {
							ing_order = aim_end_order.top();
						}
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == if_end_end) {
							if (it3 != expr->cend()) {
								if (it3->second[0].second == "else") {
									fe[adjust_aim_end.top().first].aim = to_string(order + 1);//当if语句后面还有一个else语句时，要跳过if语句体应该
									//多跳一个四元分析式
								}
								else {
									fe[adjust_aim_end.top().first].aim = to_string(order);
								}
							}
							else {
								fe[adjust_aim_end.top().first].aim = to_string(order);
							}
							adjust_aim_end.pop();
							aim_end_order.pop();

							if (aim_end_order.empty()) {
								break;
							}
							else {
								if (aim_end_order.top() != ing_order) {
									break;
								}
							}
						}

						while (!adjust_and_aim.empty() && adjust_and_aim.top().second == if_end_end) {
							if (it3 != expr->cend()) {
								if (it3->second[0].second == "else") {
									fe[adjust_and_aim.top().first].aim = to_string(order + 1);//当if语句后面还有一个else语句时，要跳过if语句体应该
								}
								else {
									fe[adjust_and_aim.top().first].aim = to_string(order);
								}
							}
							else {
								fe[adjust_and_aim.top().first].aim = to_string(order);
							}
							adjust_and_aim.pop();
						}
						which_end.pop();
					}
								   break;
					case else_end_end: {//else中的结尾标记
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == else_end_end) {//用于调整if-else语句if语句体结束时的跳转情况
							fe[adjust_aim_end.top().first].aim = to_string(order);
							adjust_aim_end.pop();
							aim_end_order.pop();
						}
						which_end.pop();
					}
									 break;
					case wh_end_end: {//while语句中的结尾标记
						//用于调整while的条件表达式不满足的情况下跳过while语句体执行的情况以及返回开始执行的情况（目前只写了不满足跳过执行的情况）
						size_t ing_order;
						if (!aim_end_order.empty()) {
							ing_order = aim_end_order.top();
						}
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == wh_end_end) {//用于调整if-else语句if语句体结束时的跳转情况
							fe[adjust_aim_end.top().first].aim = to_string(order + 1);
							adjust_aim_end.pop();
							aim_end_order.pop();

							if (aim_end_order.empty()) {
								break;
							}
							else {
								if (aim_end_order.top() != ing_order) {
									break;
								}
							}
						}

						while (!adjust_and_aim.empty() && adjust_and_aim.top().second == wh_end_end) {//对其条件表达式中的&&最后一批没有在后面遇上
							//“或”运算符的与运算符不满足条件条件时的跳转情况的调整
							fe[adjust_and_aim.top().first].aim = to_string(order + 1);
							adjust_and_aim.pop();
						}
						which_end.pop();

						//while语句体结束之后所添加的跳转语句以实现循环
						F.oper = "j";
						F.var2 = " ";
						F.var1 = " ";
						F.num = order++;

						F.aim = to_string(wh_start_aim.top().first);
						wh_start_aim.pop();
						fe.push_back(F);

					}
								   break;
					}
				}

				while ((!which_end.empty()) && (which_end.top() == if_wait_end || which_end.top() == else_wait_end || which_end.top() == wh_wait_end)) {
					int i = 1;
					while ((!adjust_aim_end.empty()) &&
						adjust_aim_end.top().second == which_end.top()) {
						switch (adjust_aim_end.top().second) {
						case if_wait_end: {
							vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
							if (it3 != expr->cend()) {
								if (it3->second[0].second == "else") {
									fe[adjust_aim_end.top().first].aim = to_string(order + 1);
								}
								else {
									fe[adjust_aim_end.top().first].aim = to_string(order);
								}
							}
							else {
								fe[adjust_aim_end.top().first].aim = to_string(order);
							}
						}
										break;
						case else_wait_end: {
							fe[adjust_aim_end.top().first].aim = to_string(order);
						}
										  break;
						case wh_wait_end: {
							fe[adjust_aim_end.top().first].aim = to_string(order + i);
							++i;
						}
										break;
						}
						adjust_aim_end.pop();
						aim_end_order.pop();
					}

					//以下是对含有"与"运算符的条件表达式的跳转调整
					i = 1;
					while (!adjust_and_aim.empty() &&
						adjust_and_aim.top().second == which_end.top()) {//对其条件表达式中的&&最后一批没有在后面遇上
						//“或”运算符的与运算符不满足条件条件时的跳转情况的调整
						switch (adjust_and_aim.top().second) {
						case if_wait_end: {
							vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
							if (it3 != expr->cend()) {
								if (it3->second[0].second == "else") {
									fe[adjust_and_aim.top().first].aim = to_string(order + 1);
								}
								else {
									fe[adjust_and_aim.top().first].aim = to_string(order);
								}
							}
							else {
								fe[adjust_and_aim.top().first].aim = to_string(order);
							}
						}
										break;
										/*case else_wait_end: {
											fe[adjust_and_aim.top().first].aim = to_string(order);e
										}
														  break;*///不一定有用
						case wh_wait_end: {
							fe[adjust_and_aim.top().first].aim = to_string(order + i);//???
							++i;
						}
										break;
						}
						//fe[adjust_and_aim.top().first].aim = to_string(order);

						adjust_and_aim.pop();
					}

					if (which_end.top() == wh_wait_end) {
						//while语句体结束之后所添加的跳转语句以实现循环
						F.oper = "j";
						F.var2 = " ";
						F.var1 = " ";
						F.num = order++;

						F.aim = to_string(wh_start_aim.top().first);
						wh_start_aim.pop();
						fe.push_back(F);
					}
					which_end.pop();//可能会出错
				}
			}
			bg.pop();


		}
			   break;

		}
	}

	//最后的收尾工作：
	F.oper = "ret";
	F.var2 = " ";
	F.var1 = " ";
	F.num = order;
	F.aim = "0";

	fe.push_back(F);
}

void Quaternion::show_inf()const {//信息展示函数
	if (fe.empty()) {
		cout << "未生成任何四元式" << endl;
	}
	else {
		cout << "生成的四元式如下：" << endl;
		for (vector<FourElements>::const_iterator it = fe.cbegin(); it != fe.cend(); ++it) {
			cout << it->num << '.' << '(' << it->oper << ',' << it->var1 << ',' << it->var2 << ',' << it->aim << ')' << endl;
		}
	}

	if (error.empty()) {//如果没有错误定义变量类的错误
		cout << "使用的变量均已定义" << endl;
	}
	else {
		for (vector<pair<size_t, string> >::const_iterator it = error.cbegin(); it != error.cend(); ++it) {
			cout << "出错的位置：" << it->first << '\t' << "未定义的变量：" << it->second << endl;
		}
	}

}
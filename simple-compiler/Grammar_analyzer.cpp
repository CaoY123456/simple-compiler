#include "Grammar_analyzer.h"
#include"Lexical_analyzer.h"
#include"Quaternion.h"
using namespace std;

Grammar_analyzer::Grammar_analyzer(const Lexical_analyzer& L) :p(make_shared<vector<pair<size_t, string> > >(L.table)),
p_row(make_shared<vector<int> >(L.row_order)) {}//����һ���ʷ����������Ĳ����Ĺ��캯��

bool Grammar_analyzer::is_filter() {//�жϵ�ǰ����ĵ����ǲ��ǹ��˷�������ǣ���������ݹ�ɨ��ֱ��ɨ�赽�ǹ��˷����ҷ���false�����ɨ�������е��ʶ�û��ɨ�赽
	//�ǹ��˷����򷵻�false�������������Ҫ�ж�һ���ǲ��Ƕ����˻س�������Ҫ��������������tmp_line
	if (word == p->cend()) {//�����Ѿ�ɨ����
		return true;
	}
	set<char>::const_iterator r = filter.find(word->second[0]);
	tmp_line = (*p_row)[word - p->cbegin()];//����У׼��ǰ�жϵ�������ڵ��������Ծ����ܾ�ȷ��λ�����λ��
	if (r != filter.cend()) {//������ʱ�������ǹ����ַ��������������Ŀ�ľ���Ҫ�����ǹ����ַ����������������
		++word;//һֱ����table����ĩβ
		return is_filter();
	}
	else {
		return false;//�����˷ǹ����ַ�������false
	}
}

void Grammar_analyzer::is_program() {//�ǲ���һ������
	word = p->cbegin();//��ʼ��word������
	while (word != p->cend()) {
		line_order = tmp_line;
		is_part_program();//����һ���ж���ֳ���ĳ�����˵����ʹǰ��ķֳ�������˴���Ҳ��Ӱ������ֳ�����ж�
		if (word != p->cend())
			++word;
		if (is_filter()) {//�˴�ʹ��wordָ����һ���ֳ���ʼ�ı�־��procedure��������һ���ǹ����ַ��������ʱ��
			return;//����ָ��table����ĩβ
		}
	}

}

bool Grammar_analyzer::is_part_program() {//�ǲ���һ���ֳ���
	if (is_filter()) {//ָ����һ���ǹ��˵���Ч�ַ�
		error.push_back(make_pair(line_order, "����һ���յķֳ���"));//�����д�
		return false;
	}
	else if (word->second != "procedure") {//�ɸ������ķ���֪��ÿ������Ŀ�ͷ��Ȼ�ǡ�procedure��
		line_order = tmp_line;
		error.push_back(make_pair(line_order, "����Ŀ�ͷ��Ǵ��󣨲���procedure��"));

		while (word->second != "procedure") {//��һ�ηֳ���Ŀ�ͷ��procedure������Ŭ��Ѱ����һ�ηֳ���Ŀ�ͷ
			//�Բ�Ӱ�������ֳ�����ж�
			++word;
			if (word == p->cend())
				break;
		}
		--word;

		return false;
	}
	else {//������ʱ�ɹ�ʶ���˳���Ŀ�ͷ��procedure������������ʶ��
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back(make_pair(line_order, "�ֳ���Ŀ�ͷprocedure��Ϊ��"));
			return false;
		}

		else if (word->first != 400) {//��ǰ�ĵ��ʲ���һ����ʶ��
			line_order = tmp_line;
			error.push_back(make_pair(line_order, "������procedure����ķǹ����ַ�����һ����ʶ��"));

			while (word->second != "procedure") {//�ֳ���Ŀ�ʼ��ɲ��ֳ������⣬��������ֳ�������һ���жϣ���
				//��Ӱ�������ֳ�����ж�
				++word;
				if (word == p->cend())
					break;
			}

			--word;

			return false;
		}
		else {//��ǰ�ĵ�����һ����ʶ�����ֱ���Ϊ400
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back(make_pair(line_order, "����ʼ��־�еı�ʶ���������Ϊ��"));
				return false;
			}

			else if (word->second != "begin") {
				line_order = tmp_line;
				error.push_back(make_pair(line_order, "����Ŀ�ͷ��ʽ�еı�ʶ������ķǹ��˷����ݲ���begin"));

				while (word->second != "procedure") {
					++word;
					if (word == p->cend())
						break;
				}

				--word;

				return false;
			}
			else {//���������procedure������б�ǰ�ĸ�ʽ����ȷ�����濪ʼ�Ը����������жϣ����������б��
				//�������Ϊ��end.��
				line_order = tmp_line;
				++word;//���ܳ���
				if (is_filter()) {
					error.push_back(make_pair(line_order, "�ֳ����ܸ�ʽ�е�begin�������Ϊ��"));
					return false;
				}
				else if (!is_stmt_list()) {
					line_order = tmp_line;
					//error.push_back(make_pair(line_order, "�ֳ��������б����ϸ�ʽҪ��"));//��ʱע�͵������ܻ��д�
					//++word;//�費��Ҫ����һ��
					return false;
				}
				else {//�ֳ��������б������ʽҪ����һ������end.��β��������Ϊ����ʶ������б���end��Ϊ�������
					//��ʹ���û��end������Ҳ����ʶ������б��ʱ����Ը���
					//#++word;//���ܳ���
					line_order = tmp_line;
					return true;
				}
			}
		}
	}
}

bool Grammar_analyzer::is_stmt_list() {//�ǲ���һ������б�ֻҪ�ܵ���һ����������ô��ʱ��wordһ�������ǹ��˷�
	while (word->second != "end.") {
		line_order = tmp_line;
		is_stmt();

		if (word != p->cend())
			++word;

		if (is_filter() || word->second == "procedure") {
			error.push_back({ line_order,"�ֳ���ȱ��end.�������" });

			--word;

			return false;
		}
	}


	//#--word;//�������жϷֳ�����ȷ��������Ǹ�++word���Ӧ
	return true;
}

bool Grammar_analyzer::is_stmt() {//�ǲ���һ����䣬�漰����ʲô��䣨�������塢��������ֵ��ѭ�������̵��á�������䣩
	//��ʱ��wordһ�����ǹ��˷����κ���䶼���ԷֺŽ�β

	bool r;//�жϵĽ��
	switch (word->first) {//���ݵ�ǰ���ʵ��ֱ����жϾ��ӵ�����
	case 1: {//������䣬if��Ӧ���ֱ���Ϊ1
		wd.push_back(*word);
		/*expr.push_back({ line_order,wd });
		wd.clear();*/
		++word;
		r = is_condition();
	}
		  break;
	case 3: {//ѭ����䣬while��Ӧ���ֱ���Ϊ3��Ŀǰֻʶ��while��Ӧ��ѭ�����
		wd.push_back(*word);
		/*expr.push_back({ line_order,wd });
		wd.clear();*/
		++word;
		r = is_loop();
	}
		  break;
	case 400: {//����Ϊ��ֵ��䣬��Ϊ��ֵ����Ա�ʶ����ͷ������ʶ����Ӧ���ֱ���Ϊ400
		//wd = "";
		//wd += word->second;
		wd.push_back(*word);
		++word;
		r = is_assign();
	}
			break;
	case 49: {//�����Ǹ�����䣬���������begin��ͷ��begin��Ӧ���ֱ���Ϊ49
		expr.push_back({ line_order,vector<pair<size_t, string> >(1,*word) });
		++word;
		r = is_compound();
	}
		   break;
	case 51: {//�����Ǻ���������䣬�������������call��ͷ��call��Ӧ���ֱ���Ϊ51
		++word;
		r = is_func();
	}
		   break;
	default: {
		if (word->second == "def") {//�ԡ�def����ͷ�Ķ���������
			++word;
			r = is_define();
		}
		else if (word->second == "int" || word->second == "char" || word->second == "double" || word->second == "float" ||
			word->second == "string") {//ʶ��c++Ĭ�ϵĶ������
			++word;
			r = is_define();
		}
		else if (word->second == "end.") {
			--word;
			r = false;//���Ϊ��###
		}
		else {
			error.push_back({ line_order,"������һ�����������ĵ�һ�ɷ�" });
			r = false;
		}
	}
	}
	return r;//stmt�������һ�����࣬���ڲ�ͬС����жϰ����˶Խ�����ǡ�;���Լ���������������жϣ�
}

bool Grammar_analyzer::is_define() {//�ǲ���һ��������䣬�������������ǰ���Ѿ�ʶ����˱��������־��def,int,char,double,string,float��
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"���������������͹ؼ��ֵĺ��������Ϊ��" });
		return false;
	}
	else if (word->first != 400) {//���͹ؼ��ֺ�����ŵĲ���һ����ʶ��
		line_order = tmp_line;
		error.push_back({ line_order,"��������������͹ؼ��ֺ�����ŵĲ���һ����ʶ��" });
		to_end();
		return false;
	}
	else {//���͹ؼ��ֺ�����ŵ���һ����ʶ��
		line_order = tmp_line;
		//size_t tmp_line2 = line_order;
		string tmp_word1 = word->second;//??
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"�����������ĺ��������Ϊ��" });
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
			error.push_back({ line_order,"����������ķָ���ʹ�ô���" });
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
					error.push_back({ line_order,"����������ķָ�����������ݿհ�" });
					r = false;
				}
				else if (word->first != 400) {
					line_order = tmp_line;
					error.push_back({ line_order,"����������ָ�����������ݲ��Ǳ�ʶ��" });
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
					error.push_back({ line_order,"����������Ķ���ı����������Ϊ�հ�" });
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
					error.push_back({ line_order,"����������ȱ�ٷֺŻ�ָ���ʹ�ô���" });
					to_end();
					return false;
				}
			}
		}
		else {
			line_order = tmp_line;
			//--word;
			to_end();
			error.push_back({ line_order,"��䲻�ԷֺŽ�β��ָ���ʹ�ô���" });
			return false;
		}
	}
}

bool Grammar_analyzer::is_assign() {//�ǲ���һ����ֵ���
	line_order = tmp_line;
	size_t start_line = line_order;
	if (is_filter()) {
		error.push_back({ line_order,"��ֵ����б�ʶ�����������Ϊ�հ�" });
		//wd = "";//??
		//wd.clear();//???
		return false;
	}
	else if (!is_put_operators()) {
		line_order = tmp_line;
		error.push_back({ line_order,"��ֵ����е����������һ����ֵ�����" });
		//--word;
		to_end();
		//wd = "";//??
		wd.clear();//??
		return false;
	}
	else {//ʶ������ȷ�ĸ�ֵ�����������������һ�������������ڱ��ʽ
		line_order = tmp_line;
		//wd += word->second;//??
		wd.push_back(*word);
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"��ֵ��丳ֵ������������Ϊ�հ�" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (!is_expr()) {
			line_order = tmp_line;
			error.push_back({ line_order,"��ֵ���ĸ�ֵ���������ŵı��ʽ�ĸ�ʽ����" });
			//--word;
			to_end();
			//wd = "";//??
			wd.clear();
			return false;
		}
		else {//ʶ��һ�����Ľ�����־��;��
			line_order = tmp_line;
			//wd += word->second;//??
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"��ֵ���ı��ʽ�������Ϊ�հ�" });
				//wd = "";//??
				//wd.clear();
				return false;
			}
			else if (word->second != ";") {
				line_order = tmp_line;
				error.push_back({ line_order,"��ֵ��䲻���Էֺš�;����β" });
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

bool Grammar_analyzer::is_condition() {//�ǲ���һ��������䣬ע�⣬�����Ѿ��жϳ�����if��ͷ�����������жϲ���Ҫ��������
	//���ķֺŽ�����־�����ڷֺŽ�����־���ж���is_stmt()��
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back(make_pair(line_order, "�������if��Ĳ���Ϊ��"));
		return false;
	}
	else if (word->second != "(") {
		line_order = tmp_line;
		error.push_back(make_pair(line_order, "if��������ų���"));
		to_end();
		return false;
	}
	else {//����if����ʶ����һ����(��
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back(make_pair(line_order, "�������ʽ��if(������Ϊ��"));
			return false;
		}
		else if (!is_cond_expr()) {//���ڡ�if(������Ĳ���һ���������ʽ�������expr������һ��ʽ�ӣ�Ҳ�п������ɶ��ʽ�����߼��������������������
			line_order = tmp_line;
			error.push_back(make_pair(line_order, "���ڡ�if(������Ĳ���һ���������ʽ"));
			to_end();
			return false;
		}
		else {//����ʶ����һ���������ʽ������һ��ĿǰΪֹif�������ĸ�ʽ��ȷ
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"�������ʽ�������Ϊ��" });
				return false;
			}
			else {//��ʱʶ����ˡ�if(expr)��������ʶ������Ƿ�����䣬ע�⣬���Ǵ�ʱ��if���ܴ�{}
				line_order = tmp_line;
				//++word;
				if (is_filter()) {
					error.push_back({ line_order,"if��������������������ʽ����������Ϊ��" });
					return false;
				}
				else if (!is_stmt()) {
					line_order = tmp_line;
					error.push_back({ line_order,"������if���ʽ����ŵ����ݲ���һ�����" });
					to_end();
					return false;
				}
				else {//ʶ�����if(expr)stmt��䣬Ҫȷ���仹��û��else��ע�⣺���ǵ�else������{}����else�����ֻ��һ����䣬
					//��������Ҳʶ����else if��������ô��
					line_order = tmp_line;

					//else {
					line_order = tmp_line;
					++word;
					if (is_filter()) {
						error.push_back({ line_order,"������if�����������Ϊ��" });
						return false;
					}
					else if (word->second != "else") {
						line_order = tmp_line;
						//error.push_back({ line_order,"������if��������ŵĲ���else" });

						--word;
						return true;
					}
					else {//����ʶ�����it(expr)stmt else
						line_order = tmp_line;

						wd.push_back(*word);
						expr.push_back({ line_order,wd });
						wd.clear();

						++word;
						if (is_filter()) {
							error.push_back({ line_order,"else���������Ϊ��" });
							expr.erase(expr.begin() + expr.size() - 1, expr.begin());
							return false;
						}
						else if (!is_stmt()) {
							line_order = tmp_line;
							error.push_back({ line_order,"else�е�����ʽ����" });
							expr.erase(expr.begin() + expr.size() - 1, expr.begin());
							to_end();
							return false;
						}
						else {
							line_order = tmp_line;

							//--word;
							return true;//�жϳ���һ��������if-else��䡣
						}
					}
					//}

				}
			}

		}
	}
}

bool Grammar_analyzer::is_loop() {//�ǲ���һ��ѭ�����
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"whileѭ������е�while�ĺ��������Ϊ�հ�" });
		return false;
	}
	else if (word->second != "(") {
		line_order = tmp_line;
		error.push_back({ line_order,"whileѭ�����ȱ�������š�(��" });
		//--word;
		to_end();
		return false;
	}
	else {
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"whileѭ����������ŵĺ��������Ϊ�հ�" });
			return false;
		}
		else if (!is_cond_expr()) {
			line_order = tmp_line;
			error.push_back({ line_order,"whileѭ������������ʽ�Ĳ��ֲ���ȷ" });
			//--word;
			to_end();
			return false;
		}
		else {
			line_order = tmp_line;
			++word;
			if (is_filter()) {
				error.push_back({ line_order,"whileѭ������������ʽ���������Ϊ��" });
				return false;
			}
			/*else if (word->second != ")") {
				line_order = tmp_line;
				error.push_back({ line_order,"whileѭ�������������ʽȱ������" });
				//--word;
				to_end();
				return false;
			}*/
			else {
				line_order = tmp_line;
				//++word;
				if (is_filter()) {
					error.push_back({ line_order,"whileѭ������������������ʽ���������Ϊ��" });
					return false;
				}
				else if (!is_stmt()) {
					line_order = tmp_line;
					error.push_back({ line_order,"whileѭ�����ѭ���岻��������ʽ" });
					//--word;
					to_end();
					return false;
				}
				else {
					line_order = tmp_line;
					return true;//whileѭ�����stmt������Դ��ֺŽ�����־
					/*if (is_filter()) {
						error.push_back({ line_order,"whileѭ������ѭ���岿�ֺ��������Ϊ�հ�" });
						return false;
					}
					else if (word->second != ";") {
						line_order = tmp_line;
						error.push_back({ line_order,"whileѭ�����ѭ����Ľ�����ǲ��Ƿֺš�;��" });
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

bool Grammar_analyzer::is_func() {//�ǲ���һ�����̵������
	line_order = tmp_line;
	++word;
	if (is_filter()) {
		error.push_back({ line_order,"���̵�������е�call�ؼ��ֺ������Ϊ�հ�" });
		return false;
	}
	else if (word->first != 400) {//call������ŵĲ���һ����ʶ��
		line_order = tmp_line;
		error.push_back({ line_order,"���̵�������call�ؼ��ֺ�����ŵ����ݲ���һ����ʶ��" });

		to_end();//###���ܳ���

		return false;
	}
	else {//call������ŵĵ�����һ����ʶ��
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"���̵�������еı�ʶ�����������Ϊ�հ�" });
			//to_end();###
			return false;
		}
		else if (word->second != ";") {
			error.push_back({ line_order,"��䲻���Էֺš�;����β" });
			to_end();
			return false;
		}
		else {
			return true;
		}
	}
}

bool Grammar_analyzer::is_compound() {//�ǲ���һ���������
	line_order = tmp_line;
	if (is_filter()) {
		error.push_back({ line_order,"��������begin�ؼ��ֺ������Ϊ�հ�" });
		return false;
	}
	else if (!is_stmt_list_2()) {
		line_order = tmp_line;
		error.push_back({ line_order,"��������ʽ����ȷ������б��ʽ����ȷ" });
		to_end();
		return false;
	}
	else {//ʶ��������б�
		line_order = tmp_line;
		++word;
		if (is_filter()) {
			error.push_back({ line_order,"������������б���������Ϊ�հ�" });
			return false;
		}
		else if (word->second != "end") {
			line_order = tmp_line;
			error.push_back({ line_order,"������䲻��end��β" });

			//###���ܻ����

			return false;
		}
		else {
			//++word;
			expr.push_back({ line_order,vector<pair<size_t, string> >(1,*word) });
			return true;
		}
	}
}

bool Grammar_analyzer::is_stmt_list_2() {//�ǲ���һ������б�ֻҪ�ܵ���һ����������ô��ʱ��wordһ�������ǹ��˷�
	while (word->second != "end") {
		line_order = tmp_line;
		bool r = is_stmt();

		if (word != p->cend()) {
			if (word->second == "end" && !r) {//������ʱ�ĸո��жϹ����Ǹ���䲻�Ǹ������
				--word;
				return true;
			}//###
			++word;
		}

		if (is_filter() || word->second == "procedure" || word->second == "end.") {
			error.push_back({ line_order,"�������ȱ��end�������" });

			--word;

			return false;
		}
	}

	--word;
	return true;
}

bool Grammar_analyzer::is_cond_expr() {//�ǲ���һ���������ʽ
	line_order = tmp_line;
	size_t start_line = line_order;
	if (is_relation()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"�������ʽ���������Ϊ��" });
			return false;
		}
		else if (word->second == ")") {//�������š�)����Ϊ�������ʽ�Ľ������
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
			return is_cond_expr();//ע��˴��ĵݹ����
		}
		else {
			line_order = tmp_line;
			//error.push_back({ line_order,"�������ʽ���ִ���" });
			wd.clear();
			return false;
		}
	}
	else {
		line_order = tmp_line;
		//error.push_back({ line_order,"���������������ʽ����" });
		wd.clear();
		return false;
	}
}

bool Grammar_analyzer::is_relation() {//�ǲ���һ����ϵ���ʽ����Ϊ��ϵ���ʽ���Թ����������ʽ
	line_order = tmp_line;

	size_t start_line = line_order;

	if (is_expr()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"�������ʽ�ĺ�������Ϊ��" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (!is_cmp_operators()) {//��ǰ���ʲ��ǹ�ϵ�����
			line_order = tmp_line;
			//wd = "";//??
			//wd.clear();
			//error.push_back({ line_order,"�������ʽ�Ĺ�ϵ���������ȷ" });
			return false;
		}
		else {
			line_order = tmp_line;
			//wd += word->second;//??
			wd.push_back(*word);
			++word;
			if (is_filter()) {
				//error.push_back({ line_order,"�������ʽ�й�ϵ������ĺ�������Ϊ��" });
				//wd = "";//??
				return false;
			}
			else if (!is_expr()) {
				line_order = tmp_line;
				//error.push_back({ line_order,"�������ʽ�ĵڶ������ʽ����" });
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
		//error.push_back({ line_order,"���������������ʽ����" });
		//wd = "";//??
		//wd.clear();
		return false;
	}
}

bool Grammar_analyzer::is_expr() {//�ǲ��Ǳ��ʽ
	line_order = tmp_line;
	if (is_item()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"���ʽ����ĺ�������Ϊ��" });
			//wd = "";//??
			//wd.clear();
			return false;
		}
		else if (word->second != "+" && word->second != "-") {//������Ϊ��ʱ���ʽ��������������ʽֻ��һ�����
			//line_order = tmp_line;
			//error.push_back({ line_order,"���ʽ�����������������Ǽӻ��" });
			line_order = tmp_line;

			--word;//??
			return true;
		}//��Դ˴����в��ԣ���������ʶ�����һЩ�����ַ������������ʽ
		else {
			bool r;

			while (word->second == "+" || word->second == "-") {
				//wd += word->second;//??
				wd.push_back(*word);
				r = with_item();
				line_order = tmp_line;
				++word;

				if (!r) {
					//error.push_back({ line_order,"��������������ı��ʽ����" });
					//wd = "";//??
					wd.clear();
					return false;
				}
			}
			if (is_filter()) {
				//error.push_back({ line_order,"��������������ı��ʽ�ĺ��������Ϊ��" });
				//wd = "";//??
				wd.clear();
				return false;
			}
			else if (is_cmp_operators() || word->second == ")" || word->second == ";"
				|| word->second == "&&" || word->second == "||" || word->second == "and" || word->second == "or") {
				line_order = tmp_line;


				--word;
				return true;//��⵽��ʶ���յ�
			}
			else {
				line_order = tmp_line;
				//wd = "";//??
				wd.clear();
				//error.push_back({ line_order,"��Ľ�����ǳ���" });###
				return false;
			}
		}
	}
	else {
		//error.push_back({ line_order,"���ʽ����ɲ��ֲ�����" });
		line_order = tmp_line;
		//wd = "";//???
		return false;
	}
}

bool Grammar_analyzer::is_item() {//�ǲ�����
	line_order = tmp_line;
	if (is_factor()) {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"���е����ӵĺ�������Ϊ��" });
			return false;
		}
		else if (word->second != "*" && word->second != "/") {//�����������ֻ��һ�����ӣ�������ܳ���
			//line_order = tmp_line;
			//error.push_back({ line_order,"���е����ӵ�������������ǳ˻��" });
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
					//error.push_back({ line_order,"��������������������" });
					//wd = "";//??
					return false;
				}
				++word;
			}
			if (is_filter()) {
				//error.push_back({ line_order,"�����������������ĺ��������Ϊ��" });
				//wd = "";//??
				return false;
			}
			else if (word->second == "+" || word->second == "-" || is_cmp_operators() || is_separater()
				|| word->second == "&&" || word->second == "||" || word->second == "and" || word->second == "or") {
				line_order = tmp_line;


				--word;
				return true;//��⵽��ʶ���յ�
			}
			else {
				line_order = tmp_line;
				//error.push_back({ line_order,"��Ľ�����ǳ���" });
				//wd = "";//??
				return false;
			}
		}
	}
	else {
		//error.push_back({ line_order,"���ʽ����ɲ��ֲ�����" });
		line_order = tmp_line;
		//wd = "";//??
		return false;
	}
}

bool Grammar_analyzer::is_factor() {//�ǲ�������
	switch (word->first) {
	case 400: {//��ʶ��
		//wd += word->second;//??
		wd.push_back(*word);
		return true;
	}
			break;
	case 450: {//����
		//wd += word->second;//??
		wd.push_back(*word);
		return true;
	}
			break;
	default: {//������������һ�����ŵı��ʽ��
		if (word->second != "(") {
			line_order = tmp_line;
			//error.push_back({ line_order,"Ӧ�������ӵĵط���������" });
			//wd = "";//??
			return false;
		}
		else {
			//wd += word->second;//??
			wd.push_back(*word);
			++word;
			line_order = tmp_line;
			if (is_filter()) {
				//error.push_back({ line_order,"�����еġ�(�����������Ϊ��" });
				//wd = "";//??
				return false;
			}
			else if (!is_expr()) {
				line_order = tmp_line;
				//error.push_back({ line_order,"��Ӧ�Ĵ����������еı��ʽ��ʽ����" });
				//wd = "";//??
				return false;
			}
			else {
				line_order = tmp_line;
				//wd += word->second;//??
				//wd.push_back(*word);
				++word;
				if (is_filter()) {
					//error.push_back({ line_order,"�����еĴ����ŵı��ʽ�ĺ��������Ϊ��" });
					//wd = "";//??
					return false;
				}
				else if (word->second != ")") {
					line_order = tmp_line;
					//error.push_back({ line_order,"���ӵĴ����ŵı��ʽ���Ҳ����ű��" });
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

bool Grammar_analyzer::with_item() {//�ǲ��������ӷ����ӵ����is_expr����
	line_order = tmp_line;
	if (word->second == "+" || word->second == "-") {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"���ʽ������������������Ϊ��" });
			//wd = "";
			return false;
		}
		else if (!is_item()) {
			line_order = tmp_line;
			//wd == "";
			//error.push_back({ line_order,"���ʽ������������������ĸ�ʽ����ȷ" });
			return false;
		}
		else {
			line_order = tmp_line;
			return true;
		}
	}
	else {
		line_order = tmp_line;
		//error.push_back({ line_order,"���ʽ�е�����������������" });
		//wd = "";
		return false;
	}
}

bool Grammar_analyzer::with_factor() {//�ǲ��������ӷ�������
	line_order = tmp_line;
	if (word->second == "*" || word->second == "/") {
		++word;
		if (is_filter()) {
			//error.push_back({ line_order,"���ʽ������������������Ϊ��" });
			//wd = "";
			return false;
		}
		else if (!is_factor()) {
			line_order = tmp_line;
			//wd = "";
			//error.push_back({ line_order,"���ʽ������������������ĸ�ʽ����ȷ" });
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
		//error.push_back({ line_order,"���е����������ӵ����������" });
		return false;
	}
}

void Grammar_analyzer::analyse() {//�﷨����������ÿ�����ҳ�һ��������仹���������ȥ
	//���ȴӴʷ������ı��ж����һ������
	is_program();
}

bool Grammar_analyzer::is_cmp_operators() {//�жϵ�ǰ�ַ��ǲ��ǹ�ϵ�����
	set<string>::const_iterator r = cmp_operators.find(word->second);
	if (r != cmp_operators.cend()) {
		return true;
	}
	else return false;
}

bool Grammar_analyzer::is_separater() {//�ǲ��Ƿָ���
	set<char>::const_iterator r = separater.find(word->second[0]);
	if (r != separater.cend()) {
		return true;
	}
	else {
		return false;
	}
}

bool Grammar_analyzer::is_put_operators() {//�ǲ��Ǹ�ֵ�����
	set<string>::const_iterator r = put_operators.find(word->second);
	if (r != put_operators.cend()) {
		return true;
	}
	else {
		return false;
	}
}

void Grammar_analyzer::to_end() {//�����һ�����ļ�����м���̳��������⣬���ǾͲ�����������к����ļ�⣬�����������ʹ��word����
	//����������ķֺŵ�λ�ö����û�зֺţ���һֱǰ�����ֳ����ĩβend��ǣ����û��end���Ǿ�ǰ������һ���ֳ���Ŀ�ʼ
	//���procedure���Ƕ���table��cend()��
	while (word != p->cend()) {
		if (word->second == ";") {
			return;
		}
		else if (word->second == "end") {//��Ӱ�����ĸ�������ʶ��
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
			if (word != p->cend() && word->second[0] == '\n') {//###���ܳ���
				++tmp_line;
			}
		}
	}
}
void Grammar_analyzer::show_inf()const {//��Ϣչʾ����

	if (p->empty()) {
		cout << "����Ϊ��" << endl;
		return;
	}

	cout << "�﷨�����Ľ��Ϊ��" << endl;
	if (error.empty()) {
		cout << "�ó���û���κ��﷨����" << endl;
	}
	else {
		for (vector<pair<size_t, string> >::const_iterator it = error.cbegin(); it != error.cend(); ++it) {
			cout << "����Ĵ������£�" << endl;
			cout << "����������������ο�����" << it->first << "     " << "��������ݣ������ο�����" << it->second << endl;
		}
	}

	if (define.empty()) {
		cout << "�ó���û���κζ������" << endl;
	}
	else {
		cout << "����ı����У�" << endl;
		int i = 1;
		for (set<string >::const_iterator it = define.cbegin(); it != define.cend(); ++it) {
			cout << "����" << i << "Ϊ��" << *it << endl;
			++i;
		}
	}

	if (expr.empty()) {
		cout << "�ó���û���κα��ʽ" << endl;
	}
	else {
		cout << "�����е�ʽ�ӣ���ֵʽ�����ʽ�ȣ����£�" << endl;
		for (vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it = expr.cbegin(); it != expr.cend(); ++it) {
			cout << "����������" << it->first << "   " << "��Ӧ�ı��ʽΪ��";
			for (vector<pair<size_t, string> >::const_iterator it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
				cout << it2->second << /*'(' << it2->first << ')' <<*/ ' ';
			}
			cout << endl;
		}
	}
}

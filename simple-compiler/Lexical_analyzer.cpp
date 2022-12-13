#include"Lexical_analyzer.h"
#include"Grammar_analyzer.h"
#include"Quaternion.h"
using namespace std;

//file_name���ĸ�ֵ������������Ҫע�⣬ÿһ�θı�file_name��������Ҫ�ı�table������ݣ�Ϊ�����ǻ����һ����ʾ���ݣ��������Ӧ��flag��Ϊfalse��
void Lexical_analyzer::set_file(const string& name) {
	if (name != file_name) {
		cout << "ע�⣺�����ļ��ѱ������ʱ��Ӧ�Ķ�Ԫ���������ݿ��������ļ������ݲ�����������ʹ��" << endl;
		file_flag = false;
		file_name = name;
		table.erase(table.begin(), table.end());
		notation = "";
	}
}//ע�⣺�������ͬ���ļ�����ľ������ݽ��б�������ǲ��ṩ���ָı�����ĺ����ı䣬����������ʹ��

//�ж�������������º�����Ҫ�Ǹ���������������С�ͺ�����
int Lexical_analyzer::is_keyword(const string& word)const {//�ж��Ƿ���keyword�����йؼ��֣�����ǣ���Ӧ��λ��+1�����򷵻�0
	string m;
	transform(word.begin(), word.end(), back_inserter(m), ::tolower);//�����������ַ���ȫ��ת��ΪСд��ĸ����keyword������Сд��ĸ���������ܳ���
	for (vector<string>::const_iterator it = keyword.cbegin(); it != keyword.cend(); ++it) {
		if (*it == m) {
			return it - keyword.cbegin() + 1;
		}
	}
	return 0;
}

bool Lexical_analyzer::is_separater(const char& ch)const {//�ж��Ƿ�Ϊ�ָ������
	set<char>::iterator it = separater.find(ch);//����ҵ��ˣ��򷵻�ָ���Ǹ�Ԫ�صĵ�����������Ҳ������򷵻�һ��β�������
	if (it == separater.cend()) {//����û���ҵ���������ַ�����һ���ָ���
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_lower(const char& ch)const {//�ж��ǲ���Сд��ĸ������
	if (ch <= 'z' && ch >= 'a') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_upper(const char& ch)const {//�ж��Ƿ��Ǵ�д��ĸ����
	if (ch <= 'Z' && ch >= 'A') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_digital(const char& ch)const {//�жϴ��������ַ��Ƿ���0-9֮�������
	if (ch <= '9' && ch >= '0') {
		return true;
	}
	else return false;
}

bool Lexical_analyzer::is_num_operator(const char& ch)const {//�ж��Ƿ������������
	set<char>::iterator it = num_operators.find(ch);//����ҵ��ˣ��򷵻�ָ���Ǹ�Ԫ�صĵ�����������Ҳ������򷵻�һ��β�������
	if (it == num_operators.cend()) {//����û���ҵ���������ַ�����һ���ָ���
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_cmp_operators(const string& s)const {//�ж��Ƿ�Ϊ�Ƚ������
	set<string>::iterator it = cmp_operators.find(s);//����ҵ��ˣ��򷵻�ָ���Ǹ�Ԫ�صĵ�����������Ҳ������򷵻�һ��β�������
	if (it == cmp_operators.cend()) {//����û���ҵ���������ַ�����һ���ָ���
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_logic_operators(const string& s)const {//�ж��Ƿ�Ϊ�߼������
	set<string>::iterator it = logic_operators.find(s);
	if (it == logic_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_pos_operators(const string& s)const {//�ж��Ƿ�Ϊλ�����
	set<string>::iterator it = pos_operators.find(s);
	if (it == pos_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}
bool Lexical_analyzer::is_put_operators(const string& s)const {//�ж��Ƿ�Ϊ��ֵ�����
	set<string>::iterator it = put_operators.find(s);
	if (it == put_operators.cend()) {
		return false;
	}
	else {
		return true;
	}
}

bool Lexical_analyzer::is_operators(const char& ch)const {//�жϴ��������ַ����Ƿ�Ϊ�ɵ��ַ���ɵ�������������������������������������ʶ�𵥸��ַ��������Լ������ж�
	//һЩ�ַ����������ͣ�
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
	case '^'://�������Ҫ�ر�ע�⣬һ����Ϊֻ�е������������еĿ�ʼ�����߼���������ǡ��û����Ǻ��������һ����=��ʱ�ſ�����һ���жϣ�
		return true;
		break;
	default:
		return false;
		break;
	}
}

bool Lexical_analyzer::is_filter(const char& ch)const {//�ж��Ƿ��ǹ��˷�
	set<char>::iterator it = filter.find(ch);
	if (it == filter.cend()) {
		return false;
	}
	else {
		return true;
	}
}

void Lexical_analyzer::analyse() {//��������file_name����Ӧ���������-�����Ԫ���б���������������ؼ��ĺ�����
	ifstream input;
	if (file_flag)return;//������ʱ��table��notation����Ҫ������
	input.open(file_name);

	char ch = ' ';
	string s = "";

	bool point_flag = false;//�������ּ��ʶ��ʱָʾ��������û�а���С����
	bool e_flag = false;//�������ּ��ʶ��ʱ��������û�а���ָ��λ
	bool o_flag = false;//���ڱ�ʶ������ʶ��ʱ�����е�ָ��λ�еķ��ţ�Ϊtrue��ʾ�ڽ�����ָ���ŵĺ������һ�������ţ�Ϊfalse��ʾû�м������ţ�
	//ע�⣺����������־λ��ȷ��ʶ���һ�������Ժ�Ҫ�ָ���Ĭ�ϵ�falseֵ

	bool note_left = false;//ָʾ����ע�ͷ���벿�֡�/*���Ƿ����Ϊtrueʱ���ʾ�Ѿ�����
	//bool note_right = false;//ָʾ����ע�ͷ��Ұ벿�֡�*/���Ƿ����Ϊtrueʱ���ʾ�Ѿ�����
	//Ŀǰ��ʱ��Ϊ����ע�ͷ�����Ҫ��־λ���������

	int left_it;//��ʾ����ע�ͷ���/*���еġ�/����table��������Ԫ�ص�λ�������������������ֱ�Ӹ�һ��������ָ��/�����ţ��������ڱ߶��ļ���ʶ������
	//table������������䶯������������ �����ڲ��������ı䶯��������ַ�����Ԫ�ص�λ��һ����ץס��һ�����˳��ɾ����ɾ����Ԫ�أ���ܵ������ıײ�
	//֮����Ҫ�������������������������λ�ã�����Ϊ�����Ǳ�ɨ���ļ��߽���ʶ��ģ���ʱ�����ʶ�𲻵�һ�������Ķ���ע�ͷ������������������/*j vvebv*as��
	//����Ϊ��Ӧ������������������Ҫ�������ұ�־λ��Ȼ�����ŵ�table�����У�Ȼ��ɾ����֮���Ԫ�أ��Ӷ���ע�͵����ݽ���һ�����ԡ�

	cin.unsetf(ios::skipws);//���������Ϊ�˲�Ҫ�����ļ��еĿհ��ַ����ո񡢻س����ȣ�

	int row = 0;//���ÿ����������ı���
	if (!input.eof()) {//��һ����Ŀ����Ҫ���Ȱ��ĵ��ĵ�һ���ַ���ȡ��������ȷ��chars_flag��ֵ
		if (input.get(ch)) {
			is_what(ch);
			s += ch;
		}
	}

	while (!input.eof()) {
		//if (input.get(ch)) {//��ô�����Ա����һ�δ������һ���ַ�������������ܳ����⣩
		input.get(ch);
		if (input.eof())
			ch = NULL;
		switch (chars_flag.first) {
		case 1: {//������ȡ�Ŀ����ǹؼ��֣���ȻҲ�п�����һ����ʶ��
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
				chars_flag.second = "names";//����ַ����ɿ��ܵĹؼ������б�Ϊ��ʶ������
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
					table.push_back(make_pair(key_flag, s));//��ȡ����һ���ؼ���
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				else {
					chars_flag.first = 5;
					chars_flag.second = "names";//ע�⣺����ط���ch�п�������������ָ��������˷���һ�֣�����ʶ�������в��ܳ�����Щ�ַ���
					//��������Ҫע�⼰ʱʶ����Щ
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
		case 2: {//������ȡ�Ŀ����Ƿָ���
			table.push_back(make_pair(70, s));
			row_order.push_back(row);
			is_what(ch);
			s = "";
			s += ch;
		}
			  break;
		case 3: {//������ȡ�Ŀ�����ĳ�������
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
					table.push_back(make_pair(150, s));//�Ƚ��������<=��
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
							table.push_back(make_pair(300, s));//��ֵ���������<<=��
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
							table.push_back(make_pair(249, s));//λ�������<<��
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
					table.push_back(make_pair(150, s));//�Ƚ��������<>��
					row_order.push_back(row);
					if (!input.eof())
						input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					table.push_back(make_pair(150, s));//�Ƚ��������<��
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
					table.push_back(make_pair(150, s));//�Ƚ��������>=��
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
						table.push_back(make_pair(300, s));//��ֵ�������>>=��
						row_order.push_back(row);
						input.get(ch);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						table.push_back(make_pair(249, s));//λ�������>>��
						row_order.push_back(row);
						is_what(ch);
						s = "";
						s += ch;
					}
				}
						break;
				default: {
					table.push_back(make_pair(150, s));//�Ƚ��������>��
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
					table.push_back(make_pair(150, s));//�Ƚ��������==��
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					table.push_back(make_pair(300, s));//��ֵ�������=��
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
					table.push_back(make_pair(300, s));//��ֵ�������&=����|=��
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
						table.push_back(make_pair(200, s));//�߼��������&&����||��
						row_order.push_back(row);
						input.get(ch);
						is_what(ch);
						s = "";
						s += ch;
					}
					else {
						table.push_back(make_pair(249, s));//λ�������&����|��
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
						table.push_back(make_pair(300, s));//��ֵ�������^=��
					else
						table.push_back(make_pair(150, s));//�Ƚ��������!=��
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				default: {
					if (s == "^") {
						table.push_back(make_pair(249, s));//λ�������^��	
					}
					else {
						table.push_back(make_pair(200, s));//�߼��������!��	
					}
					row_order.push_back(row);
					is_what(ch);
					s = "";
					s += ch;
				}
				}
			}
					break;
			case '*': {//ע�⣬֮���Խ���*���͡�/���ļ����ں��棬����Ϊ����Ҫ�뷽�跨ʶ��ע�ͺţ���/**/���͡�//��
				switch (ch) {
				case '=': {
					s += ch;
					table.push_back(make_pair(300, s));//��ֵ�������*=��
					row_order.push_back(row);
					input.get(ch);
					is_what(ch);
					s = "";
					s += ch;
				}
						break;
				case '/': {//��ʱҪע�⣬���п�����ע�͵ĺ�벿�֡�*/��
					if (note_left) {//�����Ѿ�����һ����ע�ͷ���������ע�ͷ�����һ�������Ķ���ע�ͣ�Ŀǰ���ǵ�ע���в����������ַ������ڿ��ܻ�����Ż���
						for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + left_it; it != table.cend(); ++it) {
							notation += it->second;
						}
						notation += "*/";
						notation += '\n';
						table.erase(table.begin() + left_it, table.end());//��ע�͵��Ѿ��ŵ�table���������ݽ���ɾ����ɾ����Ҳ������/*�������潫����������table�к���������
						row_order.erase(row_order.begin() + left_it, row_order.end());
						//s += ch;
						//table.push_back(make_pair(550, "/*"));
						//table.push_back(make_pair(550, s));//������ע��
						//row_order.push_back(row);
						note_left = false;//ע���ҵ�һ��������ע�ͷ�����������Ҫ����Ӧ��note_left��ԭ

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
					table.push_back(make_pair(100, s));//�����������*��
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
					table.push_back(make_pair(100, s));//�����������/��
					row_order.push_back(row);
					left_it = table.end() - 1 - table.begin();//��Ϊ�����漰���������ڲ���һЩ���ܵ�ȱ�ݣ��ʴ˴����������⣬��������ָ����ǲ�������ϣ����
					is_what(ch);
					s = "";
					s += ch;
					note_left = true;//������һ��ע�ͷ�����벿�֣����Ǵ�ʱ���ǲ�Ҫ����ϵ�һ�𣨱�ɡ�/*��������Ϊ���п������չ�����һ��������ע�ͷ�
					//����ֻҪ�ı���Ӧ�ı�־λ����
				}
						break;
				case '/': {
					s += ch;
					//table.push_back(make_pair(550, s));//����ע�ͷ���//��
					//row_order.push_back(row);
					notation += s;
					char tmp_ch;
					if (note_left == true) {//�п��ܶ���ע���������һ������ע�ͣ���ô����ȷ����//�������һ����ע�͵�����
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
							else if (tmp_ch == '/' && s_tmp2 == "*") {//ʶ����һ��*/���ҡ�/**/�����š�//��

								string tmp_notation;
								copy(notation.begin() + flag2, notation.end(), back_inserter(tmp_notation));

								notation.erase(notation.begin() + flag2, notation.end());

								for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + left_it; it != table.cend(); ++it) {
									notation += it->second;
								}
								for (string::const_iterator it = tmp_notation.cbegin(); it != tmp_notation.cend(); ++it) {
									notation += *it;
								}

								table.erase(table.begin() + left_it, table.end());//��ע�͵��Ѿ��ŵ�table���������ݽ���ɾ����ɾ����Ҳ������/*�������潫����������table�к���������
								row_order.erase(row_order.begin() + left_it, row_order.end());
								//s += ch;
								//table.push_back(make_pair(550, "/*"));
								//table.push_back(make_pair(550, s));//������ע��
								//row_order.push_back(row);
								note_left = false;//ע���ҵ�һ��������ע�ͷ�����������Ҫ����Ӧ��note_left��ԭ
								break;
							}
							else {
								s_tmp2 = "";
							}
						}//Ϊ�˼�¼ע���е����ݣ������ϲ���ע�ͽ���ʶ��	
					}
					else {
						while (input.get(tmp_ch) && tmp_ch != '\n') {
							notation += tmp_ch;
						}//Ϊ�˼�¼ע���е����ݣ������ϲ���ע�ͽ���ʶ��
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
					table.push_back(make_pair(100, s));//�����������/��
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
		case 4: {//������ȡ���ǹ��˷�
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
		case 5: {//������ȡ���Ǳ�ʶ��
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
		case 6: {//������ȡ����һ����������ʱ�����������ĵ�һ����Ϊ0
			/*if (input.eof()) {
				table.push_back(make_pair(450, s));
			}*/
			if (is_digital(ch)) {
				s += ch;
			}
			else {
				if (!point_flag && !e_flag && ch == '.') {//û��С������С�����ǰ�治����ָ����E���͡�e��
					s += ch;
					point_flag = true;
				}
				else if ((point_flag || e_flag) && ch == '.') {//����Ѿ�����С�������С����ǰ�Ѿ�����ָ��λ����ô�����������ִ����ټ�С����
					table.push_back(make_pair(450, s));
					row_order.push_back(row);
					point_flag = false;
					e_flag = false;
					o_flag = false;
					is_what(ch);
					s = "";
					s += ch;
				}
				else if (!e_flag && (ch == 'e' || ch == 'E')) {//��һ�������ִ��У����ֻ����һ��ָ��λ
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
		default: {//������ȡ����һ�������ַ�
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

bool Lexical_analyzer::is_blank(const char& ch)const {//�ж�����ַ��ǲ��ǿհ��ַ�
	if (ch == ' ' || ch == '\t' || ch == '\n') {
		return true;
	}
	else {
		return false;
	}
}

void Lexical_analyzer::is_what(const char& ch) {//���ݶ�ȡ���ĵ�һ���ַ�����������ֱ�������ܵ��ʿ����������ֳɷ�
	if (is_lower(ch) || is_upper(ch)) {
		chars_flag.first = 1;
		chars_flag.second = "keywords";//��������ȡ���ַ���������һ���ؼ��֣���ȻҲ�п�����һ����ʶ��
	}
	else if (ch == '_') {
		chars_flag.first = 5;
		chars_flag.second = "names";//��������ȡ���ַ�������һ����ʶ��
	}
	else if (is_operators(ch)) {
		chars_flag.first = 3;
		chars_flag.second = "operators";//��������ȡ���ַ���ĳ�������
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
void Lexical_analyzer::show_inf()const {//��Ϣչʾ��������չʾ����Ϣ��������������ļ�������file_name�Լ����ļ���Ӧ��Ŀ����table��
	cout << "������ļ�������Ϊ��" << file_name << endl;
	if (table.empty()) {
		cout << "���ļ��Ĵʷ��������Ϊ��" << endl;
		return;
	}

	cout << "���ڸ��ļ��Ĵʷ��������Ϊ��" << endl;
	cout << '(' << table.cbegin()->first << ' ' << ',' << ' ';
	switch (table.cbegin()->first) {
	case 70:cout << "�ָ���" << ' ' << ',' << ' '; break;
	case 100:cout << "���������" << ' ' << ',' << ' '; break;
	case 150:cout << "�Ƚ������" << ' ' << ',' << ' '; break;
	case 200:cout << "�߼������" << ' ' << ',' << ' '; break;
	case 249:cout << "λ�����" << ' ' << ',' << ' '; break;
	case 300:cout << "��ֵ�����" << ' ' << ',' << ' '; break;
	case 350:cout << "���˷�" << ' ' << ',' << ' '; break;
	case 400:cout << "��ʶ��" << ' ' << ',' << ' '; break;
	case 450:cout << "����" << ' ' << ',' << ' '; break;
	case 500:cout << "��������ʶ���ַ�" << ' ' << ',' << ' '; break;
	case 550:cout << "ע�ͷ�" << ' ' << ',' << ' '; break;
	default:cout << "�ؼ���" << ' ' << ',' << ' ';
	}
	if (table.cbegin()->first == 350) {
		switch (table.cbegin()->second[0]) {
		case '\t':cout << "tab�Ʊ��" << ')'; break;
		case '\n':cout << "�س���" << ')'; break;
		case ' ':cout << "�ո�" << ')'; break;
		default: cout << table.cbegin()->second << ')';
		}
	}
	else {
		cout << table.cbegin()->second << ')';
	}
	cout << '\t' << "����������" << *row_order.cbegin();

	vector<int>::const_iterator  it2 = row_order.cbegin() + 1;
	for (vector<pair<size_t, string> >::const_iterator it = table.cbegin() + 1; it != table.cend(); ++it) {
		cout << endl << '(' << it->first << ' ' << ',' << ' ';
		switch (it->first) {
		case 70:cout << "�ָ���" << ' ' << ',' << ' '; break;
		case 100:cout << "���������" << ' ' << ',' << ' '; break;
		case 150:cout << "�Ƚ������" << ' ' << ',' << ' '; break;
		case 200:cout << "�߼������" << ' ' << ',' << ' '; break;
		case 249:cout << "λ�����" << ' ' << ',' << ' '; break;
		case 300:cout << "��ֵ�����" << ' ' << ',' << ' '; break;
		case 350: cout << "���˷�" << ' ' << ',' << ' '; break;
		case 400:cout << "��ʶ��" << ' ' << ',' << ' '; break;
		case 450:cout << "����" << ' ' << ',' << ' '; break;
		case 500:cout << "��������ʶ���ַ�" << ' ' << ',' << ' '; break;
		case 550:cout << "ע�ͷ�" << ' ' << ',' << ' '; break;
		default:cout << "�ؼ���" << ' ' << ',' << ' ';
		}
		if (it->first == 350) {
			switch (it->second[0]) {
			case '\t':cout << "tab�Ʊ��" << ')'; break;
			case '\n':cout << "�س���" << ')'; break;
			case ' ':cout << "�ո�" << ')'; break;
			default: cout << it->second << ')';
			}
		}
		else {
			cout << it->second << ')';
		}
		cout << '\t' << "����������" << *it2;
		++it2;
	}
	cout << endl << endl;

	if (notation.empty()) {
		cout << "û��ע�͵�����";
	}
	else {
		cout << "ע�͵�����Ϊ��" << endl;
		cout << notation;
	}
	cout << endl << endl;
}

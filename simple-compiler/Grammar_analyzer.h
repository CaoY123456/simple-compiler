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
	friend class Quaternion;//ʹ����Ԫ����ʽ�����ʹ���﷨�������private��Ա
private:
	shared_ptr<vector<pair<size_t, string> > >p;//���ڹ���Lexical_analyzer�����table�е�����
	shared_ptr<vector<int> >p_row;//���ڹ���Lexical_analyzer�е�row_order������

	vector<pair<size_t, string> >error;//Ϊ�����ڼ�¼��������λ�ü������ԭ��

	set<char>filter = { ' ','\t','\n','\r' };//���˷���¼������Ŀǰ��¼��4����

	bool is_enter();//�ǲ���������һ���س�����������ˣ���ô���ٸ���

	void is_program();//�ǲ���һ������
	bool is_part_program();//�ǲ���һ���ֳ���
	bool is_stmt_list();//�ǲ���һ������б����������б���ר�ŷ��ڷֳ�����ģ�

	bool is_stmt_list_2();//�ǲ��Ǹ�������������б�

	bool is_stmt();//�ǲ���һ�����

	bool is_define();//�ǲ���һ���������
	bool is_assign();//�ǲ���һ����ֵ���
	bool is_condition();//�ǲ���һ���������
	bool is_loop();//�ǲ���һ��ѭ�����
	bool is_func();//�ǲ���һ�����̵������
	bool is_compound();//�ǲ���һ���������

	bool is_cond_expr();//�ǲ���һ���������ʽ
	bool is_relation();//�ǲ���һ����ϵ���ʽ����Ϊ��ϵ���ʽ���Թ����������ʽ

	bool is_expr();//�ǲ��Ǳ��ʽ
	bool is_item();//�ǲ�����
	bool is_factor();//�ǲ�������

	bool with_item();//�ǲ��������ӷ����ӵ���
	bool with_factor();//�ǲ��������ӷ�������

	bool is_cmp_operators();//�ǲ��ǹ�ϵ�����
	bool is_separater();//�ǲ��Ƿָ���
	bool is_put_operators();//�ǲ��Ǹ�ֵ�����

	void to_end();//�����һ�����ļ�����м���̳��������⣬���ǾͲ�����������к����ļ�⣬�����������ʹ��word����
	//����������ķֺŵ�λ�ö����û�зֺţ���һֱǰ�����ֳ����ĩβend��ǣ����û��end���Ǿ�ǰ������һ���ֳ���Ŀ�ʼ
	//���procedure���Ƕ���table��cend()��

	//vector<pair<size_t, string> >::const_iterator ing_word;//��ǰ����ĵ���
	vector<pair<size_t, string> >::const_iterator word;//��ǰ����ĵ���
	size_t line_order = 0;//�кţ�ע�⣺��0��ʼ
	size_t tmp_line = 0;//�����кţ�Ϊ�˾�ȷ��λ�����λ��

	set<string>cmp_operators = { ">","<","==","!=",">=","<=","<>" };//��ϵ���������¼��7��
	set<char>separater = { ';',',','{','}','[',']','(',')' };//�ָ�����¼������Ŀǰ��¼��8����
	set<string>put_operators = { "=","+=","-=","*=","/=","%=",">>=","<<=","&=","|=","^=" };//��ֵ���������¼��11��

	set<string>define;//�洢������������set���������е�pair��first��Ա�ǿ�ʼ������λ�ã�second��Ա�Ǳ�����Ӧ�ı�ʶ����

	vector<pair<size_t, vector<pair<size_t, string> > > >expr;//�洢���ʽ�����������pair��firstλ��ʾ�ñ��ʽ��ʼ��λ��
	//ע��������������������Ŀ�Ķ���Ϊ��������Ԫ����ʽ�Ĺ���

	vector<pair<size_t, string> > wd;
public:
	Grammar_analyzer() :p(NULL), p_row(NULL) {}//Ĭ�ϵĹ��캯��
	Grammar_analyzer(const Lexical_analyzer& L);//����һ���ʷ����������Ĳ����Ĺ��캯��

	bool is_filter();//�жϵ�ǰ����ĵ����ǲ��ǹ��˷�������ǣ���ݹ������һ��ֱ������ǹ��˷��ٷ���true��
	//ͬʱҪ�ж��ǲ��ǻس������ǵĻ�Ҫ�任��Ӧ������tmp_line�����򲻱䣩��������ǹ��˷�����ʹ�����������ҷ���false

	void analyse();//�﷨��������

	void show_inf()const;//��Ϣչʾ����
};

/*
1.����б��ʾΪstmt_list
2.����ʾΪstmt
3.define��ʾ�����������
4.assign��ʾ��ֵ���
5.condition��ʾ�������
6.loop��ʾѭ�����
7.func��ʾ���̵������
8.compound��ʾ�������
*/
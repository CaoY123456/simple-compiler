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

//����������Ŀ�ģ����Ǹ���һ���ļ������ܸ��������ļ��Ĵʷ��������ݣ���ȡ�Ĳ����Ǳ߶���ߴ����ж�
class Lexical_analyzer {
	friend class Grammar_analyzer;//ʹ���﷨���������ʹ�ôʷ��������private��Ա
private:
	string file_name;//��ʾ������ļ�������
	vector<pair<size_t, string> >table;//�������ɵ�����-�����Ԫ���б�񣬲����������ɲ������ڹ��캯��������������create_table��Ա�������Ѿ�ȴȷ����file_name��
	//��һ��file_name������һ���ļ�����Ӧһ��Ŀ���Ԫ���б��

	vector<int>row_order;//���table�����е�ÿ�����ʶ�Ӧ�����򣨴�0��ʼ��

	string notation;//ר�����ڴ洢ע���е�����
	bool file_flag = false;//��table��Ӧ���ļ���file_name��������ļ�ʱ�����ֵΪtrue����֮��Ϊfalse

	pair<int, string> chars_flag;//���ڱ�ǵ�ǰ���ڼ��ִ�ж�ʲô�����ļ�⣺
	//1������ǹؼ��֣�����ֵΪ��(1,keywords)
	//2������Ƿָ�������ֵΪ��(2,separaters)
	//3������Ǹ������������ֵΪ��(3,operators)
	//4������ǹ��˷�����ֵΪ��(4,filters)
	//5������Ǳ�ʶ������ֵΪ��(5,names)
	//6������ǳ�������ֵΪ��(6,number)
	//7���������ͣ���ֵΪ��(7,other)

	vector<string>keyword = { "if","else","while","signed","throw","union","this",
		"int","char","double","unsigned","const","goto","virtual","for","float",
	"break","auto","class","operator","case","do","long","typedef","static","friend",
	"template","default","new","void","register","extern","return","enum","inline",
	"try","short","continue","sizeof","switch","private","protected","asm","catch",
	"delete","public","volatile","struct","begin","end","call","string","procedure","def","end.","and","or" };//�ؼ�������������ȷ��һ������⵽��ĳһ���ַ����ǲ����������涨����¼���Ĺؼ��֣�Ŀǰ��¼��55����

	//�ù�������set�洢�������нϿ�Ĳ���Ч��
	set<char>separater = { ';',',','{','}','[',']','(',')' };//�ָ�����¼������Ŀǰ��¼��8����

	set<char>num_operators = { '+','-','*','/','%' };//�����������¼������Ŀǰ��¼��5����

	set<string>cmp_operators = { ">","<","==","!=",">=","<=","<>" };//�Ƚ����������¼��7��

	set<string>logic_operators = { "&&","||","!" };//�߼����������¼��3��

	set<string>pos_operators = { "&","|","^",">>","<<" };//λ���������¼��5��

	set<string>put_operators = { "=","+=","-=","*=","/=","%=",">>=","<<=","&=","|=","^=" };//��ֵ���������¼��11��

	set<char>filter = { ' ','\t','\n','\r' };//���˷���¼������Ŀǰ��¼��4����

	//���ǽ�������ͬ�ַ��������ַ������ֱ����������¹涨��
	//1���ؼ��ֵ��ֱ���ֱ�Ϊ1-48�е�����һ�������������keyword������ÿһ���ؼ��ֶ�Ӧ��λ����ͬ
	//2���ָ������ֱ������ӦΪ70��
	//3��������������ֱ������ӦΪ100��
	//4���Ƚ���������ֱ������ӦΪ150��
	//5���߼���������ֱ������ӦΪ200��
	//6��λ��������ֱ������ӦΪ249��
	//7����ֵ��������ֱ������ӦΪ300��
	//8�����˷����ֱ����ӦΪ350��
	//9�����еı�ʶ�����ֱ����ӦΪ400��
	//10��������Ӧ���ֱ���Ϊ450
	//11����������ʶ���ַ����ֱ����ӦΪ500
	//12��ע�ͷ���/**/����//�����Ϊ550

public:
	//���캯����
	//1��Ĭ�ϵĿյĹ��캯����
	Lexical_analyzer() :file_name("") {}
	//2����һ����ʾҪ��ȡ�ļ����Ĳ����Ĺ��캯����
	Lexical_analyzer(const string& name) :file_name(name), file_flag(false) {}

	//Ĭ�ϵ������������ǿյģ�����Ҫ��Ҳ����
	~Lexical_analyzer() {}

	//file_name���ĸ�ֵ������������Ҫע�⣬ÿһ�θı�file_name��������Ҫ�ı�table������ݣ�Ϊ�����ǻ����һ����ʾ���ݣ��������Ӧ��flag��Ϊfalse��
	void set_file(const string& name);

	//�ж�������������º�����Ҫ�Ǹ���������������С�ͺ�����
	int is_keyword(const string& word)const;//�ж��Ƿ��ǹؼ��֣�����ǣ��򷵻ض�Ӧ��λ���1��������ǣ��򷵻�0

	bool is_separater(const char& ch)const;//�ж��Ƿ�Ϊ�ָ������

	bool is_lower(const char& ch)const;//�ж��ǲ���Сд��ĸ������
	bool is_upper(const char& ch)const;//�ж��Ƿ��Ǵ�д��ĸ����
	bool is_digital(const char& ch)const;//�ж��ַ��Ƿ�Ϊ0-9������

	bool is_num_operator(const char& ch)const;//�ж��Ƿ������������
	bool is_cmp_operators(const string& s)const;//�ж��Ƿ�Ϊ�Ƚ������
	bool is_logic_operators(const string& s)const;//�ж��Ƿ�Ϊ�߼������
	bool is_pos_operators(const string& s)const;//�ж��Ƿ�Ϊλ�����
	bool is_put_operators(const string& s)const;//�ж��Ƿ�Ϊ��ֵ�����

	bool is_operators(const char& s)const;//�жϴ��������ַ����Ƿ�Ϊ�ɵ��ַ���ɵ�������������������������������������ʶ�𵥸��ַ��������Լ������ж�
	//һЩ�ַ����������ͣ�

	bool is_filter(const char& ch)const;//�ж��Ƿ��ǹ��˷�

	void analyse();//��������file_name����Ӧ���������-�����Ԫ���б��

	bool is_blank(const char& ch)const;//�ж�����ַ��ǲ��ǿհ��ַ�

	void is_what(const char& ch);//���ݶ�ȡ���ĵ�һ���ַ�����������ֱ�������ܵ��ʿ����������ֳɷ�

	void show_inf()const;//��Ϣչʾ��������չʾ����Ϣ��������������ļ�������file_name�Լ����ļ���Ӧ��Ŀ����table���ļ��е�ע������notation��
};

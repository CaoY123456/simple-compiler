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
	string oper;//��Ԫʽ��Ӧ�������
	string var1;//��Ԫʽ�е�һ�ű���
	string var2;//��Ԫʽ�еĶ��ű���
	string aim;//��Ԫʽ�е�Ŀ��洢ֵ
	size_t num;//����Ԫʽ��λ��
	//static size_t _number;//������Ǹ���Ԫʽ��λ��ľ�̬������

	//Ĭ�ϵĹ��캯����
	FourElements();
	//�������Ĺ��캯����
	FourElements(const string& op, const string& v1, const string& v2, const string& a, const size_t& n);
	//�������캯����
	FourElements(const FourElements& F);
	//������ֵ�����
	FourElements& operator=(const FourElements& F);

	//����������ʹ�úϳɵ�Ĭ�ϵ���������
	~FourElements() {}
};

class Quaternion {
private:
	set<string>var;//�洢�Ѿ���������ı��������ֵ��������ϣ������漰��������ֵ���ͱȽ����ʱ�������������ʹ�õı����ǲ���
	//�Ѿ��������

	vector<pair<size_t, string> >error;//���ڴ洢ʹ��û�ж�����ı����Ĵ������ʾ��Ϣ�Լ����ִ����Ӧ��������

	vector<FourElements>fe;//�洢������Ԫʽ�����vector����

	shared_ptr<set<string> >define;//�洢���ֱ�����set���������ܹ���ָ��

	shared_ptr<vector<pair<size_t, vector<pair<size_t, string > > > > >expr;//�洢���ֱ��ʽ��λ���Լ����ʽ�еĳɷֵ�
	//vector����������ָ�룬���У�vector�������ŵ���pair����pair�����first��Ա����Щ���ʽ���ڵ�λ�ã�second��Ա��
	//���ʽ�����ݣ�Ҳ����һ��vecto�������棬���е�vector�з��õ�Ҳ��һ��pair������first��Ա�Ǳ��ʽ��ͬ�ɷֶ�Ӧ��
	//�ֱ��룬second��Ա�Ǳ��ʽ���ɷֵ�ʵ�����ݡ�

public:
	//Ĭ�ϵĹ��캯����
	Quaternion() :define(NULL), expr(NULL) {}
	//�������Ĺ��캯����
	Quaternion(const Grammar_analyzer& G);

	void construct();//��Ԫ����ʽ���캯�������ģ�

	void show_inf()const;//��Ϣչʾ����.
};


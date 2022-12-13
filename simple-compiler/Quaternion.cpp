#include "Quaternion.h"
#include"Grammar_analyzer.h"
#include<stack>
using namespace std;

bool compare(const pair<size_t, string>& a, const pair<size_t, string>& b) {//a��λ�õ�������洢���������ջ�������������
	//b��λ�õ������������ǵ�ǰɨ�赽������������a�����ȼ��ߣ������b���ܷ���ջ�У�����false����a�����ȼ����ͣ������b����
	//����ջ�У�����true
	//ע���Ƚ�������ֱ���Ϊ150���߼�������ֱ���Ϊ200����ֵ������ֱ���Ϊ300������������ֱ���Ϊ100���ָ���������������ţ��ֱ���Ϊ70
	//���ȼ��Ƚ�Ϊ���ָ������ > ��������� > �Ƚ������ > �߼������ > ��ֵ�����
	switch (a.first) {
	case 100:
		if (b.first > 100) {
			return false;//a�����ȼ�����
		}
		else {
			if (b.first == 100) {//���������������������+��-��*��/��%�����бȽ�
				if (a.second == "+" || a.second == "-") {//���aΪ+��-��
					if (b.second == "+" || b.second == "-") {//��ʱb����ֱ�ӷŽ�ȥ��false
						return false;
					}
					else {//��ʱb����ֱ�ӷŽ�ȥ��true
						return true;
					}
				}
				else if (a.second == "*" || a.second == "/") {//���aΪ*��/
					if (b.second == "%") {//��ʱb����ֱ�ӷŽ�ȥ
						return true;
					}
					else {//��ʱb������ֱ�ӷŽ�ȥ
						return false;
					}
				}
				else {//��ʱaΪ%,����bΪʲô����������ȼ��϶���a��
					return false;
				}
			}
			else {
				return true;//���������˷ָ���"("
			}
		}
		break;
	case 70: //aΪ�ָ���"("��������������������ջ����ʱҪ����true
		return true;
		break;
	case 200: {//aΪ�߼��������&&�����ȼ�����
		if (a.first < b.first) {//a�����ȼ���
			return false;
		}
		else if (a.first > b.first) {//b�����ȼ���
			return true;
		}
		else {
			if (a.second == "&&") {//a�����ȼ���
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
	default://aΪ�Ƚ������
		if (a.first <= b.first) {//b�����ȼ����ͣ���ʱ���ܷ���b��false
			return false;
		}
		else {
			return true;
		}
	}

}

//size_t FourElements::_number = 1;//���ܳ���

//Ĭ�ϵĹ��캯����
FourElements::FourElements() :oper(""), var1(""), var2(""), aim(""), num(0) {}
//�������Ĺ��캯����
FourElements::FourElements(const string& op, const string& v1, const string& v2, const string& a, const size_t& n) :oper(op), var1(v1), var2(v2), aim(a), num(n) {}

//�������캯����
FourElements::FourElements(const FourElements& F) :oper(F.oper), var1(F.var1), var2(F.var2), aim(F.aim), num(F.num) {}
//������ֵ�����
FourElements& FourElements::operator=(const FourElements& F) {
	oper = F.oper;
	var1 = F.var1;
	var2 = F.var2;
	aim = F.aim;

	num = F.num;

	return *this;
}

//�������Ĺ��캯����
Quaternion::Quaternion(const Grammar_analyzer& G) :define(make_shared<set<string> >(G.define)),
expr(make_shared<vector<pair<size_t, vector<pair<size_t, string> > > > >(G.expr)) {}

void Quaternion::construct() {//������Ԫ����ʽ�ĳ�Ա������ȷ�����е�ÿ���������Ƕ�����Ļ���������˳�����ɵ��м����
	FourElements F;

	enum end_way {
		if_sem_end, if_end_end, if_wait_end,
		else_sem_end, else_end_end, else_wait_end,
		wh_sem_end, wh_end_end, wh_wait_end
	};//������β�ķ�ʽ����һ��䣬sem_end���ԷֺŽ�β��end_end��������䣬��end��β
	//wait_end����ʱ��if�������ʽ�½�����һ��while��if����ʱ��ȷ�����ǵĽ�β��ʽ��������
	//��ǰ���if��else��wh���ʾ��if��䡢else��仹��while���Ľ�β

	size_t order = 1;//orderΪ��ǰ���������Ԫʽ��λ�򣨴�1��ʼ��
	size_t tmp_order = 1;//���ڱ�ǲ����ĵ�ǰ�м������λ��

	stack<end_way>which_end;//���ֽ�β��ʽ��ջ�����ں����������ʽ������У�ѹ���Ӧ�Ľ�β��ʽ
	stack<pair<size_t, end_way> >adjust_aim_end;//�ȴ���������ת������fe�е�λ��ͽ�����ʽ�ļ��ϣ�������ת������ת����Ҫִ�е�
	//����������֮�����һ����Ԫʽ���
	stack<size_t>aim_end_order;//����ÿ����Ҫ����������Ӧ����Ԫʽ��תλ�ڵ������expr�е�λ�á�Ϊ�˱�֤���������ʽ�еġ��롱
	//�����ǰ����Ǹ��Ƚϱ��ʽ��ת��ȷ�����������������ʽ���С�||����ֻ����λ��ͬһ���������ʽ�ıȽϱ��ʽ����Ԫʽ֮�������ת��
	//��Ϊ������Ƶķ������������ͬ�������ʽ���֮��Ĵ�����ת���ұ��뽫�������һ���������ʽ�С�

	stack<size_t>adjust_or_aim;//�ȴ���������ת������fe�е�λ��ļ��ϣ���Ե��������߼���������򡱵��������Ϊ����ǰ��ı��ʽ��ȷʱ��ֱ����ת��
	//����忪ʼ����Ԫʽ��ִ�У���Ϊ�������ʽ����Ԫʽ����֮��
	stack<pair<size_t, end_way> >adjust_and_aim;//�ȴ���������ת������fe�е�λ��ļ��ϣ���Ե��������߼���������롱�������
	//��Ϊ����ǰ��ı��ʽ����ʱ��ֱ����ת�������������ĵ�һ������������ĺ���ĵ�һ����Ԫʽ����������û�л������������ת�����Ľ���

	stack<pair<size_t, end_way> >wh_start_aim;//���ڴ洢while�����ʼ���������Ԫʽ��λ���ջ��pair��first���󼴴洢ǰ�����������
	//��second��Ա�洢while�����Ľ�����ʽ���

	stack<bool >bg;//������begin�ǽ�����if��else������while����ģ�����ջΪtrue����֮��Ϊelse��Ϊ�˽������������Ƕ��ʱ������begin-end
	//ƥ�����

	for (vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it = expr->cbegin(); it != expr->cend(); ++it) {//��expr���ʽ�д洢��ÿ�����ʽ���б���
		//Ŀǰ��expr�����ı��ʽ�������С��ؼ���if��else��begin��end��while�����߼���������������ʽ����ֵ��ʽ��������䡱
		stack<string>name;//�м�������ջ������Ϊ�������Ѿ�����ı�����
		stack<pair<size_t, string> >op;//�����ջ��ֻ�д�ѹ�������������ȼ���ջ��������������ȼ����ܽ������ѹ��ջ���漰����������У�
		//�������ȼ��Ӹߵ��ͣ���������>�˳���>�Ӽ���>�Ƚ������>�߼������>��ֵ�����������ֵ�����������ȽϺ��߼����������
		//��������ջ��Ϊ��ʵ�ֺ�����м�����˳������õ�
		stack<pair<size_t, end_way> >adjust_and_or_aim;//ר�����ڵ�ǰ��������䣨��������������������ó��������У���������롱�����
		//�󻹸��Ż���������������Ӧ��ת���ĵ���

		switch (it->second[0].first) {//ÿ�����ʽ�ĵ�һ���ַ��������ֳ��ñ��ʽ������
		case 1: {//if���͵�������䣬�漰���Ƚ���������߼���������Լ�+-*/�Ĵ���
			//it��ǰ��һ�����жϸ�if�����Ľ�β���ͣ���ʵ�����������ȷ��ת
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//��ʱ�洢��ǰ���ĵ�����
			switch (it2->second[0].first) {
			case 3:
			case 1: {//����if�������ʽ�������ŵ���һ��if��������while�������
				which_end.push(if_wait_end);//��β��ʽ��������ȷ��
			}
				  break;
			case 49: {//����if�������ʽ�������ŵ���һ��������䣬��end��β
				which_end.push(if_end_end);
			}
				   break;
			case 400: {//����if�������ʽ�����ŵ���һ����һ����䣬�ԷֺŽ�β�����Ƿֺ�û�б����Ƕ�����
				which_end.push(if_sem_end);
			}
					break;
			}

			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin() + 1; k != it->second.cend(); ++k) {//�Ա��ʽ��ÿ���ɷֽ��б���
				switch (k->first) {
				case 450://�������ǳ���
				case 400: {//������������
					name.push(k->second);

					if (k->first == 400) {//�����һ����������Ҫ�������û�б������
						if (define->find(k->second) == define->cend()) {//������ʱʹ�õı���δ���������¼����δ�������
							//��Ӱ����Ԫʽ�Ĺ���
							error.push_back({ it->first,k->second });//��¼�ñ������ڵ������͸ñ����ľ��������
						}
					}

				}
						break;
				case 70: {//���������������ţ���������������ջ����������һֱ���������ջ��������ֱ��������һ��������Ϊֹ
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						vector<pair<size_t, string> >::const_iterator k2 = k + 1;
						if (k2 != it->second.cend()) {
							if (k2->first == 200) {//���������п��ܺ��бȽ���������߼���������������������һ���߼����������ʱҪ��k������ʹ��
								++k;//ָ���Ǹ��߼�����������������������Ļ���k����Ҫ����
							}
						}
						while (op.top().first != 70) {//���������ʽ�����������п��ܳ����߼���������Ƚ������������������������߼������������
							switch (op.top().first) {//�����ջop�����Դ�ʱֻ�п��ܳ��ֱȽ�����������������
							case 150: {//��ǰ�����ջ���������Ϊ�Ƚ������
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
								//����Ϊ���Ƚ�������ı��ʽ��ȷʱ����ת��������滹Ҫ��������ô��Ƚ���������������ʽ����
								//���������Ԫʽ����ת����Ĺ���

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//������������������ջ��Ϊ�Ƚ�����������϶���ǰ�������ǱȱȽ���������ȼ��͵��߼������,
								//����Ҫ������������"&&"����"||"
								if (k->second == "&&" || k->second == "and") {//��ǰ�������Ϊ�������
									fe[fe.size() - 2].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ������������if������岿�֣����������else���������
								}
								else {//��ǰ�������Ϊ�������
									adjust_or_aim.push(fe.size() - 2);
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ����������if������忪ʼ���֣�
									fe[fe.size() - 1].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ

									//����������������ǰ�滹�����ɸ��������������Ҫ�ڴ�ʱ����������������ϱ��ʽ����ת���
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}
							}
									break;
							case 100: {//��ǰ�����ջ���������Ϊ���������
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
						while (!compare(op.top(), *k)) {//if�������ʽ�п��ܳ��ֵ������������������������Ƚ������
							//�߼������
							switch (op.top().first) {
							case 150: {//���ܳ��ֱȽ����������ʱ��ǰ�����ı�Ȼ��һ���߼��������ע�⣬�߼���������ǿ���������ת�����ǲ�����
								F.oper = "j";//������ջ����
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
								//����Ϊ���Ƚ�������ı��ʽ��ȷʱ����ת��������滹Ҫ��������ô��Ƚ���������������ʽ����
								//���������Ԫʽ����ת����Ĺ���

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//������������������ջ��Ϊ�Ƚ�����������϶���ǰ�������ǱȱȽ���������ȼ��͵��߼������,
								//����Ҫ������������"&&"����"||"
								if (k->second == "&&" || k->second == "and") {//��ǰ�������Ϊ�������
									fe[fe.size() - 2].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ������������if������岿�֣����������else���������
								}
								else {//��ǰ�������Ϊ�������
									adjust_or_aim.push(fe.size() - 2);
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ����������if������忪ʼ���֣�
									fe[fe.size() - 1].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ

									//����������������ǰ�滹�����ɸ��������������Ҫ�ڴ�ʱ����������������ϱ��ʽ����ת���
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}

							}
									break;

							case 100: {//���ܳ������������
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

			while (!op.empty()) {//��ʱ����ִ���Ƚ����������������������
				switch (op.top().first) {//���ܻ�����������������Ƚ���������߼������
				case 150: {//�Ƚ������
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

					//����Ϊ�Ƚϱ��ʽ������Ԫʽ����ת��������濪ʼд����������Ԫʽ����ת���
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
				case 100: {//��ʱ�������Ϊ���������
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

			while (!adjust_or_aim.empty()) {//�Կ����������������ʽ�г����߼���������򡱵���ת������е�������������������ʽ��ɳ�����Ԫʽ
				//�����Ժ�Ϳ��Խ��е�
				fe[adjust_or_aim.top()].aim = to_string(order);
				adjust_or_aim.pop();
			}

		}
			  break;










		case 2: {//else���͵ĵ��������ĺ��أ�һ��ǰ���Ѿ���һ��������if��䣬����ִ����else���ǰ���if������е�����ʱ��
			//����if-else���Ĺ�������Ҫ����else��ȫ�������ִ�У��˴�����Ϊ�˹����Ǹ�����elseȫ����������Ԫʽ��

			//���ж�else�����Ľ��������ʲô��
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//��ʱ�洢��ǰ���ĵ�����
			switch (it2->second[0].first) {
			case 3:
			case 1: {//����else���������ŵ���һ��if��������while�������
				which_end.push(else_wait_end);//��β��ʽ��������ȷ��
			}
				  break;
			case 49: {//����if�������ʽ�������ŵ���һ��������䣬��end��β
				which_end.push(else_end_end);
			}
				   break;
			case 400: {//����if�������ʽ�����ŵ���һ����һ����䣬�ԷֺŽ�β�����Ƿֺ�û�б����Ƕ�����
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








		case 3: {//while���͵�������䣬�漰���Ƚ���������߼���������Լ�+-*/�Ĵ���
			//it��ǰ��һ�����жϸ�if�����Ľ�β���ͣ���ʵ�����������ȷ��ת
			vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it + 1;//��ʱ�洢��ǰ���ĵ�����
			switch (it2->second[0].first) {
			case 3:
			case 1: {//����while�������ʽ�������ŵ���һ��if��������while�������
				which_end.push(wh_wait_end);//��β��ʽ��������ȷ��
			}
				  break;
			case 49: {//����while�������ʽ�������ŵ���һ��������䣬��end��β
				which_end.push(wh_end_end);
			}
				   break;
			case 400: {//����while�������ʽ�����ŵ���һ����һ����䣬�ԷֺŽ�β�����Ƿֺ�û�б����Ƕ�����
				which_end.push(wh_sem_end);
			}
					break;
			}

			wh_start_aim.push({ order,which_end.top() });//��¼while��俪ʼʱ��Ԫʽ��λ���Է��������Ԫʽ����ת�����ʵ��ѭ��

			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin() + 1; k != it->second.cend(); ++k) {//�Ա��ʽ��ÿ���ɷֽ��б���
				switch (k->first) {
				case 450://�������ǳ���
				case 400: {//������������
					name.push(k->second);

					if (k->first == 400) {//�����һ����������Ҫ�������û�б������
						if (define->find(k->second) == define->cend()) {//������ʱʹ�õı���δ���������¼����δ�������
							//��Ӱ����Ԫʽ�Ĺ���
							error.push_back({ it->first,k->second });//��¼�ñ������ڵ������͸ñ����ľ��������
						}
					}

				}
						break;
				case 70: {//���������������ţ���������������ջ����������һֱ���������ջ��������ֱ��������һ��������Ϊֹ
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						vector<pair<size_t, string> >::const_iterator k2 = k + 1;
						if (k2 != it->second.cend()) {
							if (k2->first == 200) {//���������п��ܺ��бȽ���������߼���������������������һ���߼����������ʱҪ��k������ʹ��
								++k;//ָ���Ǹ��߼�����������������������Ļ���k����Ҫ����
							}
						}
						while (op.top().first != 70) {//���������ʽ�����������п��ܳ����߼���������Ƚ������������������������߼������������
							switch (op.top().first) {//�����ջop�����Դ�ʱֻ�п��ܳ��ֱȽ�����������������
							case 150: {//��ǰ�����ջ���������Ϊ�Ƚ������
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
								//����Ϊ���Ƚ�������ı��ʽ��ȷʱ����ת��������滹Ҫ��������ô��Ƚ���������������ʽ����
								//���������Ԫʽ����ת����Ĺ���

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//������������������ջ��Ϊ�Ƚ�����������϶���ǰ�������ǱȱȽ���������ȼ��͵��߼������,
								//����Ҫ������������"&&"����"||"
								if (k->second == "&&" || k->second == "and") {//��ǰ�������Ϊ�������
									fe[fe.size() - 2].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ������������if������岿�֣����������else���������
								}
								else {//��ǰ�������Ϊ�������
									adjust_or_aim.push(fe.size() - 2);
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ����������if������忪ʼ���֣�
									fe[fe.size() - 1].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ

									//����������������ǰ�滹�����ɸ��������������Ҫ�ڴ�ʱ����������������ϱ��ʽ����ת���
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}
							}
									break;
							case 100: {//��ǰ�����ջ���������Ϊ���������
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
						while (!compare(op.top(), *k)) {//while�������ʽ�п��ܳ��ֵ������������������������Ƚ������
							//�߼������
							switch (op.top().first) {
							case 150: {//���ܳ��ֱȽ����������ʱ��ǰ�����ı�Ȼ��һ���߼��������ע�⣬�߼���������ǿ���������ת�����ǲ�����
								F.oper = "j";//������ջ����
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
								//����Ϊ���Ƚ�������ı��ʽ��ȷʱ����ת��������滹Ҫ��������ô��Ƚ���������������ʽ����
								//���������Ԫʽ����ת����Ĺ���

								F.oper = "j";
								F.var2 = " ";
								F.var1 = " ";
								F.num = order++;

								F.aim = "wait";
								fe.push_back(F);
								//������������������ջ��Ϊ�Ƚ�����������϶���ǰ�������ǱȱȽ���������ȼ��͵��߼������,
								//����Ҫ������������"&&"����"||"
								if (k->second == "&&" || k->second == "and") {//��ǰ�������Ϊ�������
									fe[fe.size() - 2].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ
									//adjust_aim_end.push({ fe.size() - 1,which_end.top() });
									adjust_and_or_aim.push({ fe.size() - 1,which_end.top() });
									adjust_and_aim.push({ fe.size() - 1,which_end.top() });
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ������������if������岿�֣����������else���������
								}
								else {//��ǰ�������Ϊ�������
									adjust_or_aim.push(fe.size() - 2);
									//��ʱ��д����wait���

									//�����������ǰ�ıȽϱ��ʽ����������if������忪ʼ���֣�
									fe[fe.size() - 1].aim = to_string(order);
									//�����������ǰ�ıȽϱ��ʽ��������������һ���Ƚϱ��ʽ�Ŀ�ʼ

									//����������������ǰ�滹�����ɸ��������������Ҫ�ڴ�ʱ����������������ϱ��ʽ����ת���
									while (!adjust_and_or_aim.empty()) {
										fe[adjust_and_or_aim.top().first].aim = to_string(order);
										adjust_and_or_aim.pop();
										adjust_and_aim.pop();
									}
								}

							}
									break;

							case 100: {//���ܳ������������
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

			while (!op.empty()) {//��ʱ����ִ���Ƚ����������������������
				switch (op.top().first) {//���ܻ�����������������Ƚ���������߼������
				case 150: {//�Ƚ������
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

					//����Ϊ�Ƚϱ��ʽ������Ԫʽ����ת��������濪ʼд����������Ԫʽ����ת���
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
				case 100: {//��ʱ�������Ϊ���������
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

			while (!adjust_or_aim.empty()) {//�Կ����������������ʽ�г����߼���������򡱵���ת������е�������������������ʽ��ɳ�����Ԫʽ
				//�����Ժ�Ϳ��Խ��е�
				fe[adjust_or_aim.top()].aim = to_string(order);
				adjust_or_aim.pop();
			}
		}
			  break;








		case 400: {//��ֵ������䣬���漰+-*/�Ĵ�����������������ŵ�
			for (vector<pair<size_t, string> >::const_iterator k = it->second.cbegin(); k != it->second.cend(); ++k) {//�Ա��ʽ��ÿ���ɷֽ��б���
				switch (k->first) {
				case 450://�������ǳ���
				case 400: {//������������
					name.push(k->second);

					if (k->first == 400) {//�����һ����������Ҫ�������û�б������
						if (define->find(k->second) == define->cend()) {//������ʱʹ�õı���δ���������¼����δ�������
							//��Ӱ����Ԫʽ�Ĺ���
							error.push_back({ it->first,k->second });//��¼�ñ������ڵ������͸ñ����ľ��������
						}
					}

				}
						break;
				case 70: {//���������������ţ���������������ջ����������һֱ���������ջ��������ֱ��������һ��������Ϊֹ
					if (k->second == "(") {
						op.push(*k);
					}
					else {
						while (op.top().first != 70) {//��ֵ����ʽ�е������е������ֻ����������������漰���������
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
						while (!compare(op.top(), *k)) {//��ֵ����ʽ��ֻ�и�ֵ����������������������Ϊ��ֵ����������ȼ�
							//��ͣ����Բ�����ڴ�ʱ����ֵ����������
							pair<size_t, string>ing_op = op.top();//��ȡ��ǰ�����ջ���������
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

			while (!op.empty()) {//��ʱ����ִ���ֵ���������������������
				F.oper = op.top().second;

				switch (op.top().first) {
				case 300: {//��ʱ�������Ϊ��ֵ�����
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
				case 100: {//��ʱ�������Ϊ���������
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
				case if_sem_end: {//if�еĽ�β���
					vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == if_sem_end) {
						if (it3 != expr->cend()) {
							if (it3->second[0].second == "else") {
								fe[adjust_aim_end.top().first].aim = to_string(order + 1);//��if�����滹��һ��else���ʱ��Ҫ����if�����Ӧ��
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
								fe[adjust_and_aim.top().first].aim = to_string(order + 1);//��if�����滹��һ��else���ʱ��Ҫ����if�����Ӧ��
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
				case else_sem_end: {//else�еĽ�β���
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == else_sem_end) {//���ڵ���if-else���if��������ʱ����ת���
						fe[adjust_aim_end.top().first].aim = to_string(order);
						adjust_aim_end.pop();
						aim_end_order.pop();
					}

					/*while (adjust_and_aim.empty() && adjust_and_aim.top().second == else_sem_end) {
						fe[adjust_and_aim.top().first].aim = to_string(order);//Ӧ���ò���
					}*/
					which_end.pop();
				}
								 break;
				case wh_sem_end: {//while����еĽ�β���
					//���ڵ���while���������ʽ����������������while�����ִ�е�����Լ����ؿ�ʼִ�е������Ŀǰֻд�˲���������ִ�е������
					while (!adjust_aim_end.empty() && adjust_aim_end.top().second == wh_sem_end) {//���ڵ���if-else���if��������ʱ����ת���
						fe[adjust_aim_end.top().first].aim = to_string(order + 1);
						adjust_aim_end.pop();
						aim_end_order.pop();

					}
					while (!adjust_and_aim.empty() && adjust_and_aim.top().second == which_end.top()) {//�����������ʽ�е�&&���һ��û���ں�������
						//��������������������������������ʱ����ת����ĵ���
						fe[adjust_and_aim.top().first].aim = to_string(order + 1);
						adjust_and_aim.pop();
					}
					which_end.pop();

					//while��������֮������ӵ���ת�����ʵ��ѭ��
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

				//�����ǶԺ���"��"��������������ʽ����ת����
				i = 1;
				while (!adjust_and_aim.empty() &&
					adjust_and_aim.top().second == which_end.top()) {//�����������ʽ�е�&&���һ��û���ں�������
					//��������������������������������ʱ����ת����ĵ���
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
													  break;*///��һ������
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
					//while��������֮������ӵ���ת�����ʵ��ѭ��
					F.oper = "j";
					F.var2 = " ";
					F.var1 = " ";
					F.num = order++;

					F.aim = to_string(wh_start_aim.top().first);
					wh_start_aim.pop();
					fe.push_back(F);
				}
				which_end.pop();//���ܻ����
			}

		}
				break;







		case 49: {//�������Ŀ�ͷbegin��ǣ�������ʼ��������䣬�����������԰���һϵ����䣬�Ƚϼ���
			if (it == expr->cbegin()) {
				bg.push(false);
			}
			else {
				vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it2 = it - 1;//��һ��beginǰ����Ǹ�Ԫ����ʲô��
				switch (it2->second[0].first) {
				case 1:
				case 2:
				case 3: {//���begin��ǰ����if��else��while��true��ջ
					bg.push(true);
				}
					  break;
				default:
					bg.push(false);
				}
			}
			continue;//����Ҫ�漰��һЩ��־λ�Ŀ���
		}
			   break;







		case 50: {//�������Ľ�βend��ǣ��������Ľ�����ǣ��漰��һЩ��ת����Ŀ�ĵصĵ���
			if (!bg.empty() && bg.top()) {
				if (!which_end.empty()) {
					switch (which_end.top()) {
					case if_end_end: {//if�еĽ�β���
						vector<pair<size_t, vector<pair<size_t, string> > > >::const_iterator it3 = it + 1;
						size_t ing_order;
						if (!aim_end_order.empty()) {
							ing_order = aim_end_order.top();
						}
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == if_end_end) {
							if (it3 != expr->cend()) {
								if (it3->second[0].second == "else") {
									fe[adjust_aim_end.top().first].aim = to_string(order + 1);//��if�����滹��һ��else���ʱ��Ҫ����if�����Ӧ��
									//����һ����Ԫ����ʽ
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
									fe[adjust_and_aim.top().first].aim = to_string(order + 1);//��if�����滹��һ��else���ʱ��Ҫ����if�����Ӧ��
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
					case else_end_end: {//else�еĽ�β���
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == else_end_end) {//���ڵ���if-else���if��������ʱ����ת���
							fe[adjust_aim_end.top().first].aim = to_string(order);
							adjust_aim_end.pop();
							aim_end_order.pop();
						}
						which_end.pop();
					}
									 break;
					case wh_end_end: {//while����еĽ�β���
						//���ڵ���while���������ʽ����������������while�����ִ�е�����Լ����ؿ�ʼִ�е������Ŀǰֻд�˲���������ִ�е������
						size_t ing_order;
						if (!aim_end_order.empty()) {
							ing_order = aim_end_order.top();
						}
						while (!adjust_aim_end.empty() && adjust_aim_end.top().second == wh_end_end) {//���ڵ���if-else���if��������ʱ����ת���
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

						while (!adjust_and_aim.empty() && adjust_and_aim.top().second == wh_end_end) {//�����������ʽ�е�&&���һ��û���ں�������
							//��������������������������������ʱ����ת����ĵ���
							fe[adjust_and_aim.top().first].aim = to_string(order + 1);
							adjust_and_aim.pop();
						}
						which_end.pop();

						//while��������֮������ӵ���ת�����ʵ��ѭ��
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

					//�����ǶԺ���"��"��������������ʽ����ת����
					i = 1;
					while (!adjust_and_aim.empty() &&
						adjust_and_aim.top().second == which_end.top()) {//�����������ʽ�е�&&���һ��û���ں�������
						//��������������������������������ʱ����ת����ĵ���
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
														  break;*///��һ������
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
						//while��������֮������ӵ���ת�����ʵ��ѭ��
						F.oper = "j";
						F.var2 = " ";
						F.var1 = " ";
						F.num = order++;

						F.aim = to_string(wh_start_aim.top().first);
						wh_start_aim.pop();
						fe.push_back(F);
					}
					which_end.pop();//���ܻ����
				}
			}
			bg.pop();


		}
			   break;

		}
	}

	//������β������
	F.oper = "ret";
	F.var2 = " ";
	F.var1 = " ";
	F.num = order;
	F.aim = "0";

	fe.push_back(F);
}

void Quaternion::show_inf()const {//��Ϣչʾ����
	if (fe.empty()) {
		cout << "δ�����κ���Ԫʽ" << endl;
	}
	else {
		cout << "���ɵ���Ԫʽ���£�" << endl;
		for (vector<FourElements>::const_iterator it = fe.cbegin(); it != fe.cend(); ++it) {
			cout << it->num << '.' << '(' << it->oper << ',' << it->var1 << ',' << it->var2 << ',' << it->aim << ')' << endl;
		}
	}

	if (error.empty()) {//���û�д����������Ĵ���
		cout << "ʹ�õı������Ѷ���" << endl;
	}
	else {
		for (vector<pair<size_t, string> >::const_iterator it = error.cbegin(); it != error.cend(); ++it) {
			cout << "�����λ�ã�" << it->first << '\t' << "δ����ı�����" << it->second << endl;
		}
	}

}
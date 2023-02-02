#include "Node.h"
#include "Search.h"

// ���캯��
Node::Node(float priorProb, float cPuct, Node* parent)
{
	this->Q = 0.0f;
	//this->U = 0.0;
	this->N = 0;
	this->P = priorProb;
	this->cPuct = cPuct;
	this->parent = parent;
}

// ��������
Node::~Node()
{
	for (auto it : children)
		delete it.second;
}

// ѡ��÷���ߵĽڵ�
// ����std::pair<int, Node*>(maxAction, maxNode)
std::pair<int, Node*> Node::select()
{
	std::shared_lock<std::shared_mutex> lk(nodeLock);

	int maxAction = -1;
	float score = -10000;
	Node* maxNode = nullptr;

	for (std::map<int, Node*>::const_iterator it = children.begin(); it != children.end(); it++)
	{
		float newScore = it->second->getScore();
		if (newScore > score)
		{
			maxAction = it->first;
			maxNode = it->second;
			score = newScore;
		}
	}
	// ����������ʧ
	if (maxNode)
		maxNode->virtualLoss++;

	//for (std::map<int, Node*>::iterator it = children.begin();
	//	it != children.end(); it++)
	//{
	//	if (it->first == maxAction)
	//	{
	//		it->second->virtualLoss++;
	//	}
	//}
	return std::pair<int, Node*>(maxAction, maxNode);
}

// ��չ�ڵ�
// ����std::map<int, double>
void Node::expand(const std::map<int, float>& actionProb)
{
	// �ⲿ����
	if (children.empty())
	{
		for (std::map<int, float>::const_iterator it = actionProb.begin();
			it != actionProb.end(); it++)
		{
			Node* childNode = new Node(it->second, cPuct, this);
			children.insert(std::pair<int, Node*>(it->first, childNode));
		}
	}
}

// ���� Q��N
void Node::update(float value)
{
	// �����������������ڲ�������sleep
	std::lock_guard<SpinLock> lk(spinLock);

	virtualLoss--;
	Q = (N * Q + value) / (N + 1);
	N++;
}

// ���򴫲�
void Node::backup(float value)
{
	if (parent)
	{
		parent->backup(-value);
	}
	update(value);
}

// ��ȡ�÷�
// ����score
float Node::getScore()
{
	std::lock_guard<SpinLock> lk(spinLock);

	if (parent == nullptr)
		return 0;

	float U = cPuct * P * sqrtf(parent->N) / (N + 1.0f + virtualLoss * cVirtualLoss);

	float QQ = N ? Q : -(P * 2.0f - 1.0f); //std::max(-1.0f, std::min(1.0f, -parent->Q + cVirtualQDelta));
	int NN = std::max<int>(N, 1);
	float virtualQ = (QQ * NN + std::max(QQ, -parent->Q) * virtualLoss * cVirtualLoss) / (NN + virtualLoss * cVirtualLoss);
	// ע�� Q�Ƕ���ʤ��
	return U - virtualQ;
}

float Node::getUpQ() const
{
	return (-Q * N + (P * 2.0f - 1.0f)) / (N + 1);
}

// �Ƿ�ΪҶ�ӽڵ�
bool Node::isLeafNode(bool lock)
{
	if (lock) nodeLock.lock_shared();
	bool flag = children.empty();
	if (lock) nodeLock.unlock_shared();
	return flag;
}





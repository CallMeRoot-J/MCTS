#pragma once
#include <torch/torch.h>
#include <torch/script.h>
#include<queue>
#include<atomic>
#include<future>
#include<vector>
#include "Node.h"
#include "Board.h"
#define NOMINMAX
#define numThread 64
#include <windows.h>
#undef NOMINMAX

struct policyValue { //forward���صĽ��
	torch::Tensor policy;
	torch::Tensor value;

};
struct stateForward {  //Ԥ������
	torch::Tensor state;
	std::promise<policyValue> promise;
};

class Search
{
private:
	torch::DeviceType deviceType;
	float cPuct = 1.0f;
	//float cVirtualLoss = 0.0f;
	int BatchSize = 32; //(1+numThread) / 2;
	unsigned SearchTime = 5;
	unsigned serverWaitTimeUs = 1000;
	std::atomic<int> finished;
	std::atomic<int> is_end; //0δ������1����
	std::mutex QueueLock;
	std::queue<std::pair< torch::Tensor, std::promise<std::pair<torch::Tensor, torch::Tensor> > > > MessageQueue;
	torch::jit::script::Module model;
	Node* root;
	std::thread nnserver;
public:
	int nodeCount = 0;
	float winRate = 0.0f;
	std::condition_variable asySearchForward; // �����߳��������߳��첽
	//int count = 0;
	Search(float cPuct, unsigned ms);
	~Search();
	void getRank(Board* board);
	int getCount() { return nodeCount; }
	float getWinRate() { return winRate; }
	void calculation(const Board* board);
	int getAction(const Board* board);
	std::vector<float> getPi(std::vector<float>& visits, float T);
	void resetRoot();
	void nn_evaluate();
	std::pair<torch::Tensor, float> predict_commit(std::unique_ptr<Board>& board);

	/*
	* ���·���ֻ��ʵ��search�������õ��Ĺ��߷���
	*/
	// ��������������Ϊ�±���Ƭ��һ������
	std::vector<float> getElementsByVector(
		std::vector<float> mainVector,
		std::vector<int> indexVector
	);
	// ���������ո��������ĸ��ʷֲ����г���
	int unequalProbSample(
		std::vector<int> mainVector,
		std::vector<float> probVector
	);
	// ���Ͷ�����Ϣ
};


//Ishan Goel 19CS30052
//Yashica Patodia 19CS10049

#include <bits/stdc++.h>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <sys/shm.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
#define MAX_DEPENDENT_JOBS 10
#define MAX_NODES_IN_TREE 5000
#define MAX_RANGE_JOBID 1e8
#define MAX_JOB_TIME 250

typedef struct node
{
	long long int job_id;
	int node_index;
	int parent_index;
	int comp_time;
	pthread_mutex_t mLockNode;
	int dep_jobs[MAX_DEPENDENT_JOBS];
	int status;
	int no_dep_jobs;
	node()
	{
		job_id = 0;
		node_index = 0;
		parent_index = 0;
		comp_time = 0;
		if (pthread_mutex_init(&mLockNode, NULL) != 0) {
			printf("\n mutex init has failed for node\n");
		}
		for (int i = 0; i < MAX_DEPENDENT_JOBS; i++)
			dep_jobs[i] = 0;
		status = 0;
		no_dep_jobs = 0;
	}
} node;
typedef struct _shared_memory
{
	node tree[MAX_NODES_IN_TREE];
	int no_of_nodes;
	int curr_nodes;
	pthread_mutex_t mLockTree;
	_shared_memory()
	{
		if (pthread_mutex_init(&mLockTree, NULL) != 0) {
			printf("\n mutex init has failed for tree\n");
		}
		for (int i = 0; i < MAX_NODES_IN_TREE; i++)
		{
			tree[i] = node();
		}
		no_of_nodes = 0;
		curr_nodes = 0;
	}

} SharedMemory;
SharedMemory *shm;

void print_node(node nn)
{
	cout << "job_id: " << nn.job_id << " node_index: " << nn.node_index << " parent_index: " << nn.parent_index << " comp_time: " << nn.comp_time << " status: " << nn.status << " dependent_jobs: " << nn.no_dep_jobs << endl;
}

void initialiseTree() {

	int randoNoOfNodes = rand() % 201 + 300;
	for (int i = 0; i < randoNoOfNodes; i++)
	{
		shm->no_of_nodes++;

		shm->tree[shm->no_of_nodes].job_id = (1ll * (long long int)rand() % (long long int) MAX_RANGE_JOBID) + 1;
		shm->tree[shm->no_of_nodes].node_index = shm->no_of_nodes;
		shm->tree[shm->no_of_nodes].comp_time = rand() % MAX_JOB_TIME + 1;
		int par_index;
		while (1) {
			par_index = rand() % (shm->no_of_nodes);
			if (shm->tree[par_index].no_dep_jobs < MAX_DEPENDENT_JOBS)	break;
		}
		shm->tree[shm->no_of_nodes].parent_index = par_index;
		shm->tree[par_index].dep_jobs[shm->tree[par_index].no_dep_jobs] = shm->tree[shm->no_of_nodes].node_index;
		shm->tree[par_index].no_dep_jobs++;
	}
	shm->no_of_nodes++;
	shm->curr_nodes = shm->no_of_nodes;
	cout << randoNoOfNodes << endl;
	for (int i = 0; i < randoNoOfNodes + 2; i++) {
		print_node(shm->tree[i]);
	}
	cout << "Tree initialised with " << shm->no_of_nodes << " nodes\n";
}

void *producer(void *arg) {
	int id = *((int *)arg);
	cout << "Producer created -> Producer Id: " << id << "\n";
	int timeLeft = rand() % 10001 + 10000;

	while (timeLeft > 0)
	{
		if (shm->no_of_nodes > MAX_NODES_IN_TREE - 1) {
			cout << "ERROR::Exceeding max nodes in a tree::EXITING\n";
			exit(1);
		}
		pthread_mutex_lock(&shm->mLockTree);
		shm->tree[shm->no_of_nodes].job_id = (1ll * (long long int)rand() % (long long int) MAX_RANGE_JOBID) + 1;
		shm->tree[shm->no_of_nodes].node_index = shm->no_of_nodes;
		shm->tree[shm->no_of_nodes].comp_time = rand() % MAX_JOB_TIME + 1;
		int par_index;
		while (1) {
			par_index = rand() % (shm->no_of_nodes);
			if ((shm->tree[par_index].no_dep_jobs < MAX_DEPENDENT_JOBS) && (shm->tree[par_index].status == 0)) {
				break;
			}
			else {
				;
			}
		}
		shm->tree[shm->no_of_nodes].parent_index = par_index;
		shm->tree[par_index].dep_jobs[shm->tree[par_index].no_dep_jobs] = shm->tree[shm->no_of_nodes].node_index;
		shm->tree[par_index].no_dep_jobs++;

		cout << "Producer Job Created -> ";
		print_node(shm->tree[shm->no_of_nodes]);

		shm->no_of_nodes++;
		shm->curr_nodes++;

		pthread_mutex_unlock(&shm->mLockTree);


		int waitTime = rand() % 500 + 1;
		usleep(waitTime * 1000);
		timeLeft -= waitTime;
	}

	cout << "\nProducer " << id << " terminating \n\n";
	pthread_exit(0);
}

void *consumer(void *arg) {
	int id = *((int *)arg);
	cout << "Consumer created -> Consumer Id: " << id << "\n";
	while (1)
	{
		// cout << "Jobs Remaining : " << shm->curr_nodes << endl;
		if (shm->curr_nodes <= 1 || shm->tree[0].no_dep_jobs < 1) break;

		int s = 0;
		pthread_mutex_lock(&shm->mLockTree);
		stack<int> stack;
		stack.push(0);

		while (!stack.empty())
		{
			s = stack.top();
			stack.pop();

			if (shm->tree[s].status > 0)	continue;

			int flag = 0;
			for (int i = 0; i < shm->tree[s].no_dep_jobs; ++i) {
				if (shm->tree[s].dep_jobs[i] > 0) {
					flag = 1;
				}
			}
			if (!flag && shm->tree[s].status == 0) {
				break;
			}

			for (int i = 0; i < shm->tree[s].no_dep_jobs; ++i) {
				if (shm->tree[s].dep_jobs[i] != -1) {
					stack.push(shm->tree[s].dep_jobs[i]);
				}
			}
		}
		if (stack.empty()) {
			cout << "EMPTY STACK\n";
			pthread_mutex_unlock(&shm->mLockTree);
			break;
		}

		shm->tree[s].status++;
		shm->curr_nodes--;
		pthread_mutex_unlock(&shm->mLockTree);

		cout << "Consumer Running Job -> ";
		print_node(shm->tree[s]);

		usleep((shm->tree[s].comp_time) * 1000);
		shm->tree[s].status++;
		for (int i = 0; i < shm->tree[shm->tree[s].parent_index].no_dep_jobs; ++i)
		{
			if (shm->tree[shm->tree[s].parent_index].dep_jobs[i] == s) {
				shm->tree[shm->tree[s].parent_index].dep_jobs[i] = -1;
				break;
			}
		}

	}

	cout << "\nConsumer " << id << " terminating \n\n";
	pthread_exit(0);
}

int main()
{
	int noProducers, noConsumers;
	cout << "Enter Number of Producer : ";
	cin >> noProducers;
	cout << "Enter Number of Consumers : ";
	cin >> noConsumers;

	int segid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666 | IPC_CREAT); // creating shared memory using shmget
	if (segid < 0)
	{
		cout << "Failed to allocate shared memory!\n";
		exit(1);
	}

	shm = (SharedMemory *)shmat(segid, NULL, 0); // attaching  shared memory using shmat
	if (shm == (SharedMemory *) - 1)
	{
		cout << "Failed to attach using shmat" << endl;
		exit(1);
	}

	srand(time(0));
	initialiseTree();

	pid_t pid = fork();
	if (pid < 0)
		cout << "ERROR::PROCESS B CREATION FAILED";
	else if (pid == 0)		//Processs B
	{
		cout << "PROCESS B CREATED\n";
		shm = (SharedMemory *)shmat(segid, NULL, 0); // attaching  shared memory using shmat
		if (shm == (SharedMemory *) - 1)
		{
			cout << "Failed to attach using shmat" << endl;
			exit(1);
		}

		srand(time(0));
		pthread_t ptids_cons[noConsumers];
		pthread_attr_t attr;
		pthread_attr_init (&attr);

		for (int i = 0; i < noConsumers; i++) {
			int *arg = (int *) malloc(sizeof(*arg));
			*arg = i + 1;
			pthread_create(&ptids_cons[i], &attr, consumer, arg);
		}
		for (int i = 0; i < noConsumers; i++) {
			pthread_join(ptids_cons[i], NULL);
		}
		cout << "\nALL CONSUMERS THREADS TERMINATED\n\n";

	} else {	//Process A
		pthread_t ptids_prod[noProducers];
		pthread_attr_t attr;
		pthread_attr_init (&attr);

		for (int i = 0; i < noProducers; i++) {
			int *arg = (int *) malloc(sizeof(*arg));
			*arg = i + 1;
			pthread_create(&ptids_prod[i], &attr, producer, arg);
		}
		for (int i = 0; i < noProducers; i++) {
			pthread_join(ptids_prod[i], NULL);
		}
		cout << "\nALL PRODUCER THREADS TERMINATED\n\n";

		while (shm->curr_nodes >= 2) {
			;
		}
	}
}
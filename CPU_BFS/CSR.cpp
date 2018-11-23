#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <strstream>
#include <stdlib.h>

using namespace std;

struct graph
{
	int node_num;
	int edge_num;
	vector<int> all_node;
	vector<int> begin_array_node;
};

graph readgraph(char *path_)
{	
	graph g;
	g.edge_num = 0;
	ifstream ReadFile;
	string temp;
	int vertex;
	ReadFile.open(path_,ios::in);
	while(getline(ReadFile,temp,'\n'))
	{	
		g.edge_num++;
		stringstream line(temp);
		while(line >> vertex)
		{	
			g.all_node.push_back(vertex);
		}
	}
	ReadFile.close();
	for(int i = 0; i != g.all_node.size(); i=i+2)
	{
		g.begin_array_node.push_back(g.all_node[i]);
	}
	vector<int>::iterator set_ = std::unique(g.begin_array_node.begin(),g.begin_array_node.end());
	g.begin_array_node.resize(distance(g.begin_array_node.begin(),set_));
	sort(g.all_node.begin(),g.all_node.end());
	vector<int>::iterator result = std::unique(g.all_node.begin(),g.all_node.end());
	g.all_node.resize(distance(g.all_node.begin(),result));
	g.node_num = g.all_node.size();
	return g;
}


void CSR(char *path, int *beg_end, int *csr, graph g, int beg_end_size)
{
	ifstream ReadFile;
	string temp;
	int vertex;
	int count = 0;
	ReadFile.open(path,ios::in);
	vector<int> temp2;
	while(getline(ReadFile,temp,'\n'))
	{
		stringstream input(temp);
		while(input>>vertex)
        temp2.push_back(vertex);
    	if(beg_end[temp2[0]] == -1)	
    		beg_end[temp2[0]] = 1;
    	else
    		beg_end[temp2[0]] = beg_end[temp2[0]] + 1;
    	csr[count] = temp2[1];
      	temp2.clear();
    	count++;
	}
	int temp3 = beg_end[0];
	for(int i = 1; i != beg_end_size; i++)
	{
		if(beg_end[i] != -1)
		{
			temp3 = temp3 + beg_end[i];
			beg_end[i] = temp3;
		}
	}
}


int main()
{	
	char * path_ = "/Users/rzhan/Desktop/soc.txt";
	graph g = readgraph(path_);
	int beg_end_size = g.all_node[g.all_node.size()-1] + 1;
	int *beg_end = (int *)malloc(beg_end_size);
	for(int i = 0; i != beg_end_size; i++)
	{
		beg_end[i] = -1;
	}
	int *csr = (int *)malloc(g.edge_num);
	CSR(path_, beg_end, csr, g, beg_end_size);
	cout << "number of nodes: " << g.node_num << "\n" << "number of edges: " << g.edge_num << endl;
}




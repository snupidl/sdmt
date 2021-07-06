/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
 */
#ifndef HUFFMAN_H_
#define HUFFMAN_H_


#include <string>
#include <queue>
#include <vector>
#include <bitset>
using namespace std;

struct huffman_node
{
	int id; 
	int freq;
	int node_array_size;
	string code;	
	huffman_node* left;
	huffman_node* right;
	huffman_node()
	{
		left = right = NULL;
	}
};
typedef huffman_node* node_ptr;

class huffman
{
protected:
	node_ptr* node_array;						
	int* in_value;
	int dim, c_dim;
	node_ptr child, parent, root;
	int node_array_size;
	class compare
	{
	public:
		bool operator()(const node_ptr& c1, const node_ptr& c2) const
		{
			return c1->freq > c2->freq;
		}
	};
	priority_queue<node_ptr, vector<node_ptr>, compare> pq;
	void create_node_array(int);
	void traverse(node_ptr, string);
	int binary_to_decimal(string);
	string decimal_to_binary(int);
	inline void build_tree(string&, int);

public:
	huffman(int*, int);
	void create_pq();
	void create_huffman_tree();
	void calculate_huffman_codes();
	int coding_save();
	int decoding_save();
	void recreate_huffman_tree();
};

void huffman::create_node_array(int value)
{
	for (int j = 0; j < node_array_size; j++)
	{
		if(node_array[j]->id == value){
		node_array[j]->freq++;
		return;
		}
	}

	node_array[node_array_size] = new huffman_node;
	node_array[node_array_size]->id = value;
	node_array[node_array_size]->freq = 1;
	node_array_size++;
}

void huffman::traverse(node_ptr node, string code)
{
	if (node->left == NULL && node->right == NULL)
	{
		node->code = code;
	}
	else
	{
		traverse(node->left, code + '0');
		traverse(node->right, code + '1');
	}
}

int huffman::binary_to_decimal(string in)
{
	int result = 0;
	for (int i = 0; i < in.size(); i++)
		result = result * 2 + in[i] - '0';
	return result;
}

string huffman::decimal_to_binary(int in)
{
	string temp = "";
	string result = "";
	while (in)
	{
		temp += ('0' + in % 2);
		in /= 2;
	}
	result.append(32 - temp.size(), '0');
	for (int i = temp.size() - 1; i >= 0; i--)												
	{
		result += temp[i];
	}
	return result;
}

inline void huffman::build_tree(string& path, int a_code)
{
	node_ptr current = root;
	for (int i = 0; i < path.size(); i++)
	{
		if (path[i] == '0')
		{
			if (current->left == NULL)
				current->left = new huffman_node;
			current = current->left;
		}
		else if (path[i] == '1')
		{
			if (current->right == NULL)
				current->right = new huffman_node;
			current = current->right;														 
		}
	}
	current->id = a_code;
}

huffman::huffman(int* in, int dim)
{
	in_value = in;
	dim = dim;
}
void huffman::create_pq()
{
	node_array_size = 0;
	for (int i=0;i<dim;i++){
		create_node_array(in_value[i]);
	}

	for (int i = 0; i < node_array_size; i++)
	{
		if (node_array[i]->freq)
		{
			pq.push(node_array[i]);
		}
	}
}

void huffman::create_huffman_tree()
{
	priority_queue<node_ptr, vector<node_ptr>, compare> temp(pq);
	while (temp.size() > 1)
	{
		root = new huffman_node;
		root->freq = 0;
		root->left = temp.top();
		root->freq += temp.top()->freq;
		temp.pop();
		root->right = temp.top();
		root->freq += temp.top()->freq;
		temp.pop();
		temp.push(root);
	}
}

void huffman::calculate_huffman_codes()
{
	traverse(root, "");
}

int huffman::coding_save()
{
	int* in;
	int index = 0;
	string s = "";
	int code_count;
	if(node_array_size % 32 == 0){
		code_count = 0;
	}
	else{
		code_count = node_array_size % 32;
	}

	in[index] = pq.size();
	index++;
	in[index] = node_array_size;
	index++; 
	priority_queue<node_ptr, vector<node_ptr>, compare> temp(pq);
	while (!temp.empty())
	{
		node_ptr current = temp.top();
		in[index] = current->id;
		index++;
		s.assign(code_count+node_array_size-1 - current->code.size(), '0'); 
		s += '1';
		s.append(current->code);
		const int value = binary_to_decimal(s.substr(0, 32));
		in[index] = value;
		index++;
		for (int i = 0; i < (code_count + node_array_size)/32-1; i++)
		{
			s = s.substr(32);
			in[index] = binary_to_decimal(s.substr(0, 32));
			index++;
		}
		temp.pop();
	}
	s.clear();

	for (int i=0; i<dim; i++){
		for(int j=0; j<node_array_size;j++){
		
			if(in_value[i]==node_array[j]->id){
				s+=node_array[j]->code;
				while (s.size() > 32)
				{
					in[index] = binary_to_decimal(s.substr(0, 32));
					index++;
					s = s.substr(32);
				}

			}
		}
	}
	int count = 32 - s.size();
	if (s.size() < 32)
	{
		s.append(count, '0');
	}
	in[index] = binary_to_decimal(s); 
	index++;
	in[index] = count;
	index++;

	in_value = in;
	c_dim = index;
	
	return c_dim;
}

void huffman::recreate_huffman_tree()
{
	int index = 0;
	int size = in_value[index];
	index++;
	node_array_size = in_value[index];
	index++;
	int code_count;

	if(node_array_size % 32 == 0){
		code_count = 0;
	}
	else{
		code_count = node_array_size % 32;
	}
	
	int c_size = (code_count + node_array_size)/32;

	root = new huffman_node;

	for (int i = 0; i < size; i++)
	{
		int a_code;
		a_code = in_value[index];
		index++;

		string h_code_s = "";
		for (int i=0; i < c_size; i++){
			h_code_s += decimal_to_binary(in_value[index]);
			index++;
		}

		int j = 0;
		while (h_code_s[j] == '0')
		{
			j++;
		}
		h_code_s = h_code_s.substr(j + 1);
		build_tree(h_code_s, a_code);
	}
}


int huffman::decoding_save()
{
	int* in;
	int c_dim = 0;
	int index, size;
	index = 0;
	size = in_value[index];
	int count = in_value[dim-1];
	int code_count;
	
	if(node_array_size % 32 == 0){
		code_count = 0;
	}
	else{
		code_count = node_array_size % 32;
	}
	int c_size = (code_count + node_array_size)/32;
	index = 2 + ( 1 + c_size ) * size;

	node_ptr current = root;
	string path;
	while (index<dim-1)
	{
		path = decimal_to_binary(in_value[index]);
		index++;
		if (index == dim - 2)
			path = path.substr(0, 32 - count);
		for (int j = 0; j < path.size(); j++)
		{
			if (path[j] == '0')
				current = current->left;
			else
				current = current->right;
			if (current->left == NULL && current->right == NULL)
			{
				in[c_dim] = current->id;
				c_dim++;
				current = root;
			}
		}
	}
	in_value = in;
	return c_dim;
}

#endif

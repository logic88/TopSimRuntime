#ifndef __TSTREE_718AF7E3_5B1C_4EF8_AEC5_E3D1F7133A9D_H__
#define __TSTREE_718AF7E3_5B1C_4EF8_AEC5_E3D1F7133A9D_H__
#include "TSAllocate.h"

TS_BEGIN_RT_NAMESPACE

template <class Key,class Data> 
class TSTree 
{
public:
	OVERLOADNEWOPERATOR;

	// TSTree::Iterator
	class Iterator;
	// TSTree::Node

	class Node : public Data
	{	
	private:
		friend class TSTree<Key,Data>;
		friend class TSTree<Key,Data>::Iterator;

		Node(const Key &key) : parent(0), left(0), right(0), balance(0), key(key) 
		{

		}
		~Node() 
		{
			delete left;
			delete right;
		}

		Node *parent;	// parent node
		Node *left;		// child nodes
		Node *right;
		int balance;	// node balance
	public:
		OVERLOADNEWOPERATOR;
		const Key key;
	};

	// TSTree::Iterator
	class Iterator 
	{
	private:
		friend class TSTree<Key,Data>;
		Iterator(Node *node) : node(node) 
		{

		}
	public:
		typedef Key KeyType;
		typedef Data DataType;

		Iterator() : node(0) 
		{

		}

		Iterator(const Iterator &it) : node(it.node) 
		{

		}

		~Iterator() 
		{

		}

		Iterator &operator=(const Iterator &it) 
		{
			node = it.node;
			return *this;
		}

		FORCEINLINE int operator==(const Iterator &it) const 
		{ 
			return node == it.node; 
		}

		FORCEINLINE int operator!=(const Iterator &it) const 
		{ 
			return node != it.node; 
		}

		FORCEINLINE Node &operator*() 
		{ 
			return *node; 
		}

		FORCEINLINE const Node &operator*() const 
		{ 
			return *node; 
		}

		FORCEINLINE Node *operator->() 
		{
			return node; 
		}

		FORCEINLINE const Node *operator->() const 
		{ 
			return node; 
		}

		FORCEINLINE Node *get() 
		{ 
			return node; 
		}

		FORCEINLINE const Node *get() const 
		{ 
			return node; 
		}

		FORCEINLINE Iterator &operator++() 
		{
			next();
			return *this;
		}

		FORCEINLINE Iterator operator++(int) 
		{
			Iterator temp = *this;
			next();
			return temp;
		}

	private:
		void next() 
		{
			if(node->right != 0) 
			{
				node = node->right;
				while(node->left != 0) 
				{
					node = node->left;
				}
			} 
			else 
			{
				Node *y = node->parent;
				while(y != 0 && node == y->right) 
				{
					node = y;
					y = y->parent;
				}
				if(y == 0 || node->right != y) 
				{
					node = y;
				}
			}
		}

		Node *node;
	};

	// TSTree
	TSTree() : length(0), root(0) 
	{

	}
	TSTree(const TSTree &tree) : length(0), root(0) 
	{
		Node *dest_parent = 0;
		copy_proc(root,dest_parent,tree.root);
	}

	~TSTree() 
	{
		delete root;
	}

	TSTree &operator=(const TSTree &tree) 
	{
		if(this == &tree) return *this;
		delete root;
		length = 0;
		root = 0;
		Node *dest_parent = 0;
		copy_proc(root,dest_parent,tree.root);
		return *this;
	}

	void swap(TSTree &tree) 
	{
		if(this == &tree) return;
		int i = length; length = tree.length; tree.length = i;
		Node *n = root; root = tree.root; tree.root = n;
	}

	FORCEINLINE Iterator begin() const 
	{
		if(root) 
		{
			Node *node = root;
			while(node->left) node = node->left;
			return Iterator(node);
		}
		return Iterator(0);
	}
	FORCEINLINE Iterator end() const 
	{
		return Iterator(0);
	}

	FORCEINLINE int size() const 
	{ 
		return length; 
	}

	FORCEINLINE int empty() const 
	{ 
		return (length == 0); 
	}

	void clear() 
	{
		delete root;
		length = 0;
		root = 0;
	}

	template <class T> 
	Iterator find(const T &key) const 
	{
		Node *node = root;
		while(node && (node->key == key) == 0)  
		{
			node = (key < node->key) ? node->left : node->right;
		}
		return Iterator(node);
	}

	FORCEINLINE void remove(const Key &key) 
	{
		int change = 0;
		Node *node = remove_proc(key,root,change);
		delete node;
	}
	FORCEINLINE void remove(const Iterator &it) 
	{
		remove(it->key);
	}
protected:
	void copy_proc(Node *&dest_root,Node *&dest_parent,const Node *src_root) 
	{
		if(src_root == 0) 
		{
			return;
		}

		length++;
		dest_root = new Node(src_root->key);
		dest_root->parent = dest_parent;
		dest_root->balance = src_root->balance;
		const Data *src = static_cast<const Data*>(src_root);
		Data *dest = static_cast<Data*>(dest_root);
		*dest = *src;

		copy_proc(dest_root->left,dest_root,src_root->left);
		copy_proc(dest_root->right,dest_root,src_root->right);
	}

	Node *append_proc(const Key &key,Node *&root,Node *&parent,int &change) 
	{
		// root is NULL
		if(root == 0) 
		{
			length++;
			root = new Node(key);
			root->parent = parent;
			change = 1;
			return root;
		}

		// find node
		Node *ret = 0;
		int increase = 0;
		if(key < root->key) 
		{
			ret = append_proc(key,root->left,root,change);
			increase = -change;
		} 
		else if(root->key < key) 
		{
			ret = append_proc(key,root->right,root,change);
			increase = change;
		} 
		else 
		{
			return root;
		}

		// rebalance tree
		root->balance += increase;
		if(increase && root->balance) 
		{
			change = 1 - balance(root);
		}
		else 
		{
			change = 0;
		}

		return ret;
	}

	Node *remove_proc(const Key &key,Node *&root,int &change) 
	{

		// can't find node
		if(root == 0) 
		{
			change = 0;
			return 0;
		}

		// find node
		Node *ret = 0;
		int decrease = 0;
		if(key < root->key) 
		{
			ret = remove_proc(key,root->left,change);
			if(ret == 0) return ret;
			decrease = -change;
		} 
		else if(root->key < key) 
		{
			ret = remove_proc(key,root->right,change);
			if(ret == 0) return ret;
			decrease = change;
		} 
		else 
		{
			length--;
			ret = root;
			if(root->left == 0 && root->right == 0) 
			{
				change = 1;
				root = 0;
				return ret;
			} 
			else if(root->left == 0 || root->right == 0) 
			{
				change = 1;
				if(root->left) 
				{
					root = root->left;
				}
				else 
				{
					root = root->right;
				}
				root->parent = ret->parent;
				ret->left = 0;
				ret->right = 0;
				return ret;
			} 
			else 
			{
				ret = remove_proc(root->right,decrease);
				const_cast<Key&>(root->key) = ret->key;
				const Data *src = static_cast<const Data*>(ret);
				Data *dest = static_cast<Data*>(root);
				*dest = *src;
			}
		}
		root->balance -= decrease;
		if(decrease && root->balance) 
		{
			change = balance(root);
		}
		else if(decrease) 
		{
			change = 1;
		}
		else 
		{
			change = 0;
		}

		return ret;
	}

	Node *remove_proc(Node *&root,int &change) 
	{
		Node *ret = 0;
		int decrease = 0;
		if(root->left != 0) 
		{
			ret = remove_proc(root->left,change);
			if(ret == 0)
			{
				return ret;
			}
			decrease = -change;
		} 
		else 
		{
			ret = root;
			if(root->right == 0) 
			{
				change = 1;
				root = 0;
				return ret;
			} 
			else 
			{
				change = 1;
				root = root->right;
				root->parent = ret->parent;
				ret->left = 0;
				ret->right = 0;
				return ret;
			}
		}
		root->balance -= decrease;
		if(decrease && root->balance) 
		{
			change = balance(root);
		}
		else if(decrease) 
		{
			change = 1;
		}
		else 
		{
				change = 0;
		}
		return ret;
	}

	int rotate_left_once(Node *&root) 
	{
		int change = (root->right->balance != 0);
		Node *old_root = root;
		root = old_root->right;
		old_root->right = root->left;
		root->left = old_root;
		root->parent = old_root->parent;
		old_root->parent = root;
		if(old_root->right) 
		{
			old_root->right->parent = old_root;
		}

		old_root->balance = -(--root->balance);

		return change;
	}

	int rotate_right_once(Node *&root) 
	{
		int change = (root->left->balance != 0);
		Node *old_root = root;
		root = old_root->left;
		old_root->left = root->right;
		root->right = old_root;
		root->parent = old_root->parent;
		old_root->parent = root;
		if(old_root->left)
		{
			old_root->left->parent = old_root;
		}
		old_root->balance = -(++root->balance);

		return change;
	}

	int rotate_left_twice(Node *&root) 
	{
		Node *old_root = root;
		Node *old_root_right = root->right;
		root = old_root->right->left;
		old_root->right = root->left;
		root->left = old_root;
		old_root_right->left = root->right;
		root->right = old_root_right;
		root->parent = old_root->parent;
		old_root->parent = root;
		old_root_right->parent = root;
		if(old_root->right) 
		{
			old_root->right->parent = old_root;
		}

		if(old_root_right->left)
		{
			old_root_right->left->parent = old_root_right;
		}

		root->left->balance = -((root->balance > 0) ? root->balance : 0);
		root->right->balance = -((root->balance < 0) ? root->balance : 0);
		root->balance = 0;

		return 1;
	}

	int rotate_right_twice(Node *&root) 
	{
		Node *old_root = root;
		Node *old_root_left = root->left;
		root = old_root->left->right;
		old_root->left = root->right;
		root->right = old_root;
		old_root_left->right = root->left;
		root->left = old_root_left;
		root->parent = old_root->parent;
		old_root->parent = root;
		old_root_left->parent = root;
		if(old_root->left) 
		{
			old_root->left->parent = old_root;
		}
		if(old_root_left->right) 
		{
			old_root_left->right->parent = old_root_left;
		}
		root->left->balance = -((root->balance > 0) ? root->balance : 0);
		root->right->balance = -((root->balance < 0) ? root->balance : 0);
		root->balance = 0;

		return 1;
	}

	int balance(Node *&root) 
	{
		if(root->balance < -1) 
		{
			if(root->left->balance == 1)
			{
				return rotate_right_twice(root);
			}
			else 
			{
				return rotate_right_once(root);
			}
		} 
		else if(root->balance > 1) 
		{
			if(root->right->balance == -1) 
			{
				return rotate_left_twice(root);
			}
			else 
			{
				return rotate_left_once(root);
			}
		}
		return 0;
	}

	int length;		// node count
	Node *root;		// root of the tree
};

TS_END_RT_NAMESPACE

#endif // __TSTREE_718AF7E3_5B1C_4EF8_AEC5_E3D1F7133A9D_H__

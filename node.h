#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include <cstdlib>
#include <map>
#include <set>
#include <vector>

#define BITLENGTH 8

//forward declaration
class Node;

//The following code is just for reference. You can define your own finger table class.
//Since the index uniquely determines the interval, only the successor needs to be maintained.  
class FingerTable{
public:
	/**
	 * @param nodeId: the id of node hosting the finger table.
	 */
	FingerTable(unsigned int nodeId);
	void set(size_t index, Node* successor){
		fingerTable_[index] = successor;
	}
	//unsigned int get(size_t index) {
	Node* get(size_t index) {
		return fingerTable_[index];
	}
private:
	unsigned int nodeId_;
	std::vector<Node*> fingerTable_;
};

FingerTable::FingerTable(unsigned int nodeId): nodeId_(nodeId) {
	// According to Chord paper, the finger table starts from index=1
	fingerTable_.resize(BITLENGTH + 1);
}

class Node {
public:
	Node(unsigned int id): id_(id), fingerTable_(id){} ;
	//TODO: implement node join function
	/**
	 * @param node: the first node to contact with to initialize join process. If this is the first node to join the Chord network, the parameter is NULL.
	 */
	void join(Node* node);
	//TODO: implement DHT lookup
	Node* find_successor(unsigned int key, bool printFlag);
	//TODO: implement DHT key insertion
	void insert(unsigned int key, unsigned int value);
	//TODO: implement DHT key deletion
	void remove(unsigned int key);
	unsigned int find(unsigned int key);
	// TODO: complete print function
	void prettyPrint();
private:
	uint64_t id_;
	FingerTable fingerTable_;
	std::map<unsigned int, unsigned int> localKeys_;
    Node* predecessor;

    //helper functions
    Node* find_predecessor(unsigned int key, bool printFlag);
    Node* closest_preceding_finger(unsigned int key);
    void update_finger_table(Node* s, int i);
    bool withinIntervalOpen(unsigned int start, unsigned int finish, unsigned int valToFind);
    bool withinIntervalLC(unsigned int start, unsigned int finish, unsigned int valToFind);
    bool withinIntervalRC(unsigned int start, unsigned int finish, unsigned int valToFind);
};

#endif

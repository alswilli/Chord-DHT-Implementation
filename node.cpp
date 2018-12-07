#include "node.h"
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <string>
#define M 8
#define SUCCESSOR fingerTable_.get(1)

std::set<unsigned int> nodeSet;

/* add node n to the network
 * @param(n1) a node given to the joining node as a reference
 */
void Node::join(Node* n1) {
    Node* n = this;

    if (n1 == NULL) {
        // first node!  
        for (int i = 1; i <= M ; i++) 
            n->fingerTable_.set(i, n);
        predecessor = n;
        nodeSet.insert(n->id_);
    }
    else {
        //if another node with this node's ID is already in the network, then don't
        std::set<unsigned int> :: iterator itr;
        itr = nodeSet.find(n->id_);
        if(itr != nodeSet.end()) { // i.e. if the ID is found in the set
            printf("error: already a node %d in network. aborting join\n", n->id_);
            return;
        }
        nodeSet.insert(n->id_);
        //else, not the first node

        //init finger table
        //Following four lines complete the join like a doubly linked list insertion
        n->fingerTable_.set(1, n1->find_successor((unsigned int)n->id_ + 1 % (unsigned int)pow((double)2, (double)M), false)); //assigning successor  
        n->predecessor = n->SUCCESSOR->predecessor;
        n->SUCCESSOR->predecessor = n;
        n->predecessor->fingerTable_.set(1, n); // Making sure to have the successor of the predecessor be current node
        
        for (int i = 1; i < M ; i++) {
            unsigned int finger_ip1_start = ((unsigned int)(n->id_)+(unsigned int)pow((double)2, (double)(i))) % 
                    (unsigned int)(pow((double)2, (double)M ));
            if(withinIntervalLC(finger_ip1_start, n->id_, n->fingerTable_.get(i)->id_)) {
                n->fingerTable_.set(i+1, n->fingerTable_.get(i));
            }
            else {
                n->fingerTable_.set(i+1, n1->find_successor(finger_ip1_start, false));
            }
        }

        //update others
        for (int i = 1; i <= M ; i++) {
            Node* p = n->find_predecessor(n->id_ - (unsigned int)pow((double)2, (double)i-1), false); 
            p->update_finger_table(n, i);
        }     
    }

    // Take over keys from successor. Also print finger table, keys that are migrated
    prettyPrint();

    std::map<unsigned int, unsigned int> successorKeys = n->SUCCESSOR->localKeys_;
    std::map<unsigned int, unsigned int>::iterator iter;

    bool keysMoved = false;

    printf("The keys to be moved from %d to %d are: \n", n->SUCCESSOR->id_, n->id_);

    // check if keys need to be migrated, and if so, migrate them
    for (iter = successorKeys.begin(); iter != successorKeys.end(); iter++)
    {
        unsigned int succKey = iter->first;
        unsigned int succKeyVal = iter->second;
        if(withinIntervalRC(succKey, predecessor->id_, n->id_)) { 
            keysMoved = true; 
			printf("Key = %d \n", succKey);
            n->localKeys_.insert(std::make_pair(succKey, succKeyVal));
            successorKeys.erase(iter);
        }
    }
    if(keysMoved == false) {
        printf("No keys were moved \n");
    }
	return ;
}
/* find which node in the network is responsible for storing the input key
 * @param(key) the key to be searched for
 * @return the ID of the node responsible for storing the input key
 */
unsigned int Node::find(unsigned int key) {
    Node* n = this;
    printf("lookup: checking node %d for key %d\n", id_, key);
    if (withinIntervalRC(key, n->predecessor->id_, n->id_)) {
        if (n->localKeys_.find(key) != n->localKeys_.end()) {
            printf("Found key %d at local node %d \n", key, n->id_);
            return n->id_; 
        }
        else {
            std::cout << "No key " << key << " found for lookup" << '\n';
        }
    }
    else {
        Node* responsibleNode = n->find_successor(key, true);
        printf("lookup: checking node %d for key %d\n", responsibleNode->id_, key);
        if(responsibleNode->localKeys_.find(key) != responsibleNode->localKeys_.end()) {
            printf("Found key %d at remote node %d \n", key, responsibleNode->id_);
            return responsibleNode->id_; 
        }
        else {
            std::cout << "No key " << key << " found for lookup" << '\n';
        }
    }
}
/* insert a (key, value) pair into the network. automatically puts them in the correct node
 * @param(key) the key to be inserted
 * @param(value) the value corresponding to @param(key)
 */
void Node::insert(unsigned int key, unsigned int value) {
    Node* n = this;

    // if key range is within the range of IDs that n is responsible for
    if (withinIntervalRC(key, n->predecessor->id_, n->id_)) {
        if (n->localKeys_.find(key) != n->localKeys_.end()) {
            printf("Key %d overwritten at local node %d to hold value %d instead of value %d \n", key, n->id_, value, n->localKeys_.find(key)->second);
            n->localKeys_.find(key)->second = value;
        }
        else {
            printf("Key %d inserted at local node %d \n", key, n->id_);
            n->localKeys_.insert(std::make_pair(key, value));
        }
    }    
    //else, find the node whose range of IDs contains the key
    else { 
        Node* responsibleNode = n->find_successor(key, false);

        if(responsibleNode->localKeys_.find(key) != responsibleNode->localKeys_.end()) {
            printf("Key %d overwritten at remote node %d to hold value %d instead of value %d \n", key, responsibleNode->id_, value, responsibleNode->localKeys_.find(key)->second);
            responsibleNode->localKeys_.find(key)->second = value;
        }
        else {
            printf("Key %d inserted at remote node %d \n", key, responsibleNode->id_);
            responsibleNode->localKeys_.insert(std::make_pair(key, value));
        }
    }
}
/* removes a (key, value) pair from the network
 * @param(key) the key of the (key, value) pair to be removed
 */
void Node::remove(unsigned int key) {   
    Node* n = this;
    
    // if key range is within the range of IDs that n is responsible for
    if (withinIntervalRC(key, n->predecessor->id_, n->id_)) {
        if (n->localKeys_.find(key) != n->localKeys_.end()) {
            printf("Key %d erased at local node %d \n", key, n->id_);
            n->localKeys_.erase(key);
        }
        else {
            printf("No key %d found for removal\n", key);
        }
    }   	
    //else, find the node whose range of IDs contains the key
    else { 
        Node* responsibleNode = n->find_successor(key, false);
        if(responsibleNode->localKeys_.find(key)->first) {
            printf("Key %d erased at remote node %d \n", key, responsibleNode->id_);
            responsibleNode->localKeys_.erase(key);
        }
        else {
            printf("No key %d found for removal\n", key);
        }
    }
}

/* prints the finger table of the node
 */
void Node::prettyPrint() {
    Node* n = this;

    std::cout << "The finger table for the newly joined node " << n->id_ << '\n';
    for (int i = 1; i <= M; i++) {
        std::cout << i << " || " << n->fingerTable_.get(i)->id_ << '\n'; 
    }
    std::cout << '\n';
}

/* finds the successor node to the input key
 * @param(key) the key whose successor is being searched for
 * @param(printFlag) flag determining whether to print nodes being searched for during lookup
 * @return the node that is the successor of the input key
 */
Node* Node::find_successor(unsigned int key, bool printFlag) {
    Node* n = this;

    Node* n1 = n->find_predecessor(key, printFlag);
    return n1->SUCCESSOR;
}

/* finds the predecessor node to the input key
 * @param(key) the key whose predecessor is being searched for
 * @param(printFlag) flag determining whether to print nodes being searched for during lookup
 * @return the node that is the predecessor of the input key
 */
Node* Node::find_predecessor(unsigned int key, bool printFlag) {
    Node* n1 = this;

    while(withinIntervalRC(key, n1->id_, n1->SUCCESSOR->id_) != true) { 
        n1 = n1->closest_preceding_finger(key);
        if (printFlag) {
            printf("lookup: checking node %d for key %d\n", n1->id_, key);
        }
    }
    	
	return n1;
}
/* finds the closest preceeding node in the finger table to the input key
 * @param(key) the key whose predecessor is being searched for
 * @return the node that is the closest preceeding node in the node's finger table of the input key
 */
Node* Node::closest_preceding_finger(unsigned int key) {
	Node* n = this;

    for (int i = M; i > 0; i--) {

        if(withinIntervalOpen(n->fingerTable_.get(i)->id_, n->id_, key)) {
            return n->fingerTable_.get(i);
        }
    }
    return n;
}

/* updates the finger table of the node. called when a node joins or leaves the network
 * @param(s) if s a ﬁnger of calling node n, update n’s ﬁnger table with s 
 * @param(i) the index in the finger table to check for node s
 */
void Node::update_finger_table(Node* s, int i) {
    Node* n = this;

	unsigned int fti = n->fingerTable_.get(i)->id_;

    if (withinIntervalOpen(s->id_, n->id_, fti)) {
        n->fingerTable_.set(i, s);
        Node* p = n->predecessor;
        p->update_finger_table(s, i);
    }

}

/* check if the key is within the interval (start, finish)
 * @param(valToFind) the value being searched for
 * @param(start) the beginning of the interval to search
 * @param(finish) the end of the interval to search
 * @return true if valToFind is in the interval, false otherwise
 */
bool Node::withinIntervalOpen(unsigned int valToFind, unsigned int start, unsigned int finish) {
    bool within = false;

    // none of the values cross the 0 line compared to other inputs
    if(start < finish && valToFind < finish && valToFind > start) {
        within = true;
    }
    // going from start to finish crosses 0 line
    else if (start > finish && (valToFind < finish || valToFind > start)) {
        within = true;
    }
    // self-explanatory
    else if (start == finish) {
         within = true;
    }

    return within;
}
/* check if the key is within the interval [start, finish)
 * @param(valToFind) the value being searched for
 * @param(start) the beginning of the interval to search
 * @param(finish) the end of the interval to search
 * @return true if valToFind is in the interval, false otherwise
 */
bool Node::withinIntervalLC(unsigned int valToFind, unsigned int start, unsigned int finish) {
    bool within = false;

    // none of the values cross the 0 line compared to other inputs
    if(start < finish && valToFind < finish && valToFind >= start) {
        within = true;
    }
    // going from start to finish crosses 0 line
    else if (start > finish && (valToFind < finish || valToFind >= start)) {
        within = true;
    }
    else if (start == finish) {
        within = true;
    }

    return within;
}

/* check if the key is within the interval (start, finish]
 * @param(valToFind) the value being searched for
 * @param(start) the beginning of the interval to search
 * @param(finish) the end of the interval to search
 * @return true if valToFind is in the interval, false otherwise
 */
bool Node::withinIntervalRC(unsigned int valToFind, unsigned int start, unsigned int finish) {
    bool within = false;

    // none of the values cross the 0 line compared to other inputs
    if(start < finish && valToFind <= finish && valToFind > start) {
        within = true;
    }
    // going from start to finish crosses 0 line
    else if (start > finish && (valToFind <= finish || valToFind > start)) {
        within = true;
    }
    else if (start == finish) {
        within = true;
    }

    return within;
}

/* runs tests to make sure the above functions work properly.
 * queries the user for a random seed to use for the random ID generation
 */
int main() {

    std::cout << "input a random seed to use: ";
    int randomSeed;
    std::cin >> randomSeed;
    std::srand(randomSeed);  //set the random seed. change the input to get a different set of outputs

    Node n0(std::rand() % (int)pow((double)2, (double)M));
    Node n1(std::rand() % (int)pow((double)2, (double)M));
    Node n2(std::rand() % (int)pow((double)2, (double)M));
    Node n3(std::rand() % (int)pow((double)2, (double)M));
    Node n4(std::rand() % (int)pow((double)2, (double)M));
    Node n5(std::rand() % (int)pow((double)2, (double)M));
    Node n6(std::rand() % (int)pow((double)2, (double)M));
    Node n7(std::rand() % (int)pow((double)2, (double)M));

    unsigned int k0 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k1 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k2 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k3 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k4 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k5 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k6 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int k7 = std::rand() % (int)pow((double)2, (double)M);

    unsigned int v0 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v1 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v2 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v3 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v4 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v5 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v6 = std::rand() % (int)pow((double)2, (double)M);
    unsigned int v7 = std::rand() % (int)pow((double)2, (double)M);

    // all operations done on n0 in case some node insertions fail 

    n0.join(NULL);
    n1.join(&n0);
    n2.join(&n0);
    n3.join(&n2);

    n0.insert(k0, v0);
    n0.insert(k1, v1);
    n0.insert(k2, v2);
    n0.insert(k3, v3);
    printf("inserting duplicate key with new value\n");
    n0.insert(k0, 123);

    n0.find(k0);
    n0.find(k1);
    n0.find(k2);
    n0.find(k3);

    n0.remove(k2);
    n0.remove(k3);

    n0.find(k2);
    n0.find(k3);
    
    n4.join(&n3);
    n5.join(&n2);
    n6.join(&n4);
    n7.join(&n6);

    n0.insert(k4, v4);
    n0.insert(k5, v5);
    n0.insert(k6, v6);
    n0.insert(k7, v7);

    n0.find(k4);
    n0.find(k5);
    n0.find(k6);
    n0.find(k7);

    n0.remove(k0);
    n0.remove(k1);
    printf("removed keys %d and %d\n", k0, k1);

    n0.find(k0);
    n0.find(k1);
}

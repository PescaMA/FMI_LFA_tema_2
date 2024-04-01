#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <deque>
#include <vector>

class FA{ /// abstract implementation of a finite automata
protected:
    static const char LAMBDA = '@'; /// the character decided to be lambda/epsilon/empty symbol.
    int startState;
    std::unordered_set<int> finalStates; /// unordered_set for O(1) verification if a state is final.
    std::unordered_map<int, std::unordered_map<char,std::unordered_set<int> > > edges;
    /// edges is an adjacency list. [StartingNode] list of [Edge "cost"/letter] [EndingNode].

public:
    FA(){}
    void read(std::istream& in){/// input data format is present in requirements.txt
        int n,m;
        std::deque<int> states;
        std::string listFinals;
        int nrFinalStates;
        /// Didn't use the above variables so I didn't put them as members.
        in >> n;
        for(int i=0;i<n;i++){
            int state;
            in >> state;
            states.push_back(state);
        }
        in >> m;
        for(int i=0;i<m;i++){
            int startNode, endNode;
            char letter;
            in >> startNode >> endNode >> letter;
            edges[startNode][letter].insert(endNode);
        }
        in >>startState;
        in >> nrFinalStates;
        for(int i=0;i<nrFinalStates;i++){
            int state;
            in >> state;
            finalStates.insert(state);
        }
    }
   friend std::istream& operator>>(std::istream& in, FA& myFA){
        myFA.read(in);
        return in; /// used read so I could write that inside the class.
   }
};
int main(){
std::ifstream fin("input.txt");
std::ofstream fout("output.txt");

}

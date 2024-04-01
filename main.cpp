#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <deque>
#include <vector>

class FA{ /// abstract implementation of a finite automata
protected:
    const char LAMBDA = '@'; /// the character decided to be lambda/epsilon/empty symbol.
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
   friend std::ostream& operator<<(std::ostream& out, FA& myFA){
        for(auto el:myFA.edges){
            out << el.first << ": ";
            for(auto edge:el.second){
                for(auto next:edge.second)
                    out << "-" << edge.first << "->" << next;
            }
            out << '\n';
        }
        return out; /// used read so I could write that inside the class.
   }
};

class l_NFA : public FA{

    /// returns start and finish state
    std::pair<int,int> readRegex(const std::string& regExp, int& pos){
        std::pair<int,int> result = {-1,-1};

        while((unsigned int
        )pos < regExp.size() && regExp[pos] != ')' ){
            char ch = regExp[pos];
            pos++;
            while(ch == '(')
                std::pair<int,int> nodes = readRegex(regExp,pos);

            if(ch == ')')
                break;

            if(ch == '*'){
                if(result.first == -1)
                    throw std::invalid_argument("Starred nothing!");
                edges[result.first][LAMBDA].insert(result.second);
                edges[result.second][LAMBDA].insert(result.first);
                break;
            }
            edges[pos][ch].insert(pos+1);
            if(result.first == -1)
                result.first = pos;
            result.second = pos + 1;

        }
        return result;


    }
public:
    l_NFA(const std::string& regExp){
        int index = 0;
        startState = 0;
        readRegex(regExp, index);

    }
};


int main(){
std::ifstream fin("input.txt");
std::ofstream fout("output.txt");
l_NFA test("a");
std::cout << test;
}

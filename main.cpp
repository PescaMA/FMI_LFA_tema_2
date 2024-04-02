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
                    out << " -" << edge.first << "-> " << next;
            }
            out << '\n';
        }
        return out; /// used read so I could write that inside the class.
   }
};

class l_NFA : public FA{

    /// returns start and finish state
    std::pair<int,int> readRegex(const std::string& regExp, int& pos, int& bracketsCount){
        std::pair<int,int> result = {-1,-1};

        while((unsigned int)pos < regExp.size()){

            char ch = regExp[pos];
            pos++;

            if(ch == '('){
                bracketsCount++;
                std::pair<int,int> nodes = readRegex(regExp,pos,bracketsCount);
                result.second = nodes.second;
                if(result.first == -1)
                    result.first = nodes.first;
            }


            if(ch == ')'){
                bracketsCount--;
                if(bracketsCount < 0)
                    throw std::invalid_argument("Wrong parenthesis!");
                break;
            }


            if(ch == '*' || ch == '+'){
                if(result.second == -1)
                    throw std::invalid_argument("Starred nothing!");

                edges[result.second][LAMBDA].insert(result.first);

                if(ch == '*')
                    edges[result.first][LAMBDA].insert(result.second);
            }
            if((ch >= 'A' && ch <='Z') ||
               (ch >= 'a' && ch <='z') ||
               (ch >= '0' && ch <='9')){

                if(result.second == -1)
                    result.second = pos;

                if(result.first == -1 || bracketsCount == 0)
                    result.first = result.second;

                edges[result.second][ch].insert(result.second+1);
                result.second++;

            }
        }
        return result;


    }
public:
    l_NFA(const std::string& regExp){
        int index = 0, bracketsCount = 0;
        std::pair<int,int> result = readRegex(regExp, index,bracketsCount);
        startState = result.first;
        finalStates.insert(result.second);
    }
};


int main(){
std::ifstream fin("input.txt");
std::ofstream fout("output.txt");
l_NFA test("(ac)*");

std::cout << test;
}

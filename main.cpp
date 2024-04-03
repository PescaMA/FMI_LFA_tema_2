#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

class FA{ /// abstract implementation of a finite automata
protected:
    const char LAMBDA = '@'; /// the character decided to be lambda/epsilon/empty symbol.
    int startState;
    std::unordered_set<int> finalStates; /// unordered_set for O(1) verification if a state is final.
    std::unordered_map<int, std::unordered_map<char,std::unordered_set<int> > > edges;
    /// edges is an adjacency list. [StartingNode] list of [Edge "cost"/letter] [EndingNode].

public:
    FA(){}
    bool inline acceptedLetter(char ch){
        return (ch >= 'A' && ch <='Z') ||
               (ch >= 'a' && ch <='z') ||
               (ch >= '0' && ch <='9');
    }
    void read(std::istream& in){/// input data format is present in requirements.txt
        int n,m;
        std::vector<int> states;
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
       std::vector<int> nodes;
        for(auto el:myFA.edges)
            nodes.push_back(el.first);

        std::sort(nodes.begin(),nodes.end());


        for(auto node:nodes){
            std::vector<std::pair<int,char> > sortedEdges;
            for(auto edge:myFA.edges[node])
                for(auto edgeNode:edge.second)
                    sortedEdges.emplace_back(edgeNode,edge.first);

            std::sort(sortedEdges.begin(),sortedEdges.end());

            out << node << ":";
            for(auto edge:sortedEdges)
                out << " -" << edge.second << "-> " << edge.first;
            out << '\n';
        }
            /*
            for(auto edge:el.second){
                for(auto next:edge.second)
                    out << " -" << edge.first << "-> " << next;*/

        return out; /// used read so I could write that inside the class.
   }
};

class l_NFA : public FA{

    typedef std::pair<int,int> IntPair;
    struct regExInfo{

        const std::string& regExp;
        char ch;
        unsigned int pos = 0;
        IntPair nodes = {-1,1}; /// first and last states of current expression.
        int bracketsCount = 0;
        regExInfo(const std::string& regExp) : regExp(regExp){}
    };
    bool KleeneStar(regExInfo& info, bool lastElement = true){
        if(info.ch != '*' )
            return false;

        if(info.nodes.second == -1)
            throw std::invalid_argument("Starred nothing!");

        IntPair starredNodes;

        if(lastElement) /// if you star only the last element.
            starredNodes = {info.nodes.second - 1, info.nodes.second};
        else
            starredNodes = info.nodes;

        edges[starredNodes.second][LAMBDA].insert(starredNodes.first);

        edges[starredNodes.first][LAMBDA].insert(starredNodes.second);
        return true;
    }
    bool openBracket(regExInfo& info){
        if(info.ch != '(')
            return false;
        info.bracketsCount++;

        IntPair oldNodes = info.nodes;
        readRegex(info);

        if(oldNodes.first != -1)
            info.nodes.first = oldNodes.first;
        return true;
    }
    bool closeBracket(regExInfo& info){
        if(info.ch != ')')
            return false;

        info.bracketsCount--;
        if(info.bracketsCount < 0)
            throw std::invalid_argument("Wrong parenthesis!");

        if(info.pos < info.regExp.size() && info.regExp[info.pos] == '*'){
            info.ch = info.regExp[info.pos];
            KleeneStar(info,false);
            info.pos++;
        }

        return true;
    }
    bool letter(regExInfo& info){
        if(!acceptedLetter(info.ch))
             return false;

        if(info.nodes.first == -1)
            info.nodes.first = info.nodes.second;

        edges[info.nodes.second][info.ch].insert(info.nodes.second+1);
        info.nodes.second++;

        return true;
    }
    void readRegex(regExInfo& info){
        info.nodes.first = -1;

        while(info.pos < info.regExp.size()){

            info.ch = info.regExp[info.pos];
            info.pos++;

            if(openBracket(info))
                continue;

            if(closeBracket(info))
                break;

            KleeneStar(info); /// since not in parenthesis, star only last element

            letter(info);

        }
    }
public:
    l_NFA(const std::string& regExp){
        std::string preParsedExp;

        for(char ch:regExp){
            if(ch != ' ')
                preParsedExp.push_back(ch);
        } /// ignore all spaces

        regExInfo info(preParsedExp);


        readRegex(info);

        startState = info.nodes.first;
        finalStates.insert(info.nodes.second);
    }
};


int main(){
std::ifstream fin("input.txt");
std::ofstream fout("output.txt");
l_NFA test("(a* b*)  *");

std::cout << test;
}

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
    void clear(){
        finalStates.clear();
        for(auto edge:edges)
            for(auto transition:edge.second)
                transition.second.clear();
        edges.clear();
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

        if(myFA.finalStates.empty())
            return out << " Empty FA! ";

        std::vector<int> nodes;

        out << "Start node: " << myFA.startState << '\n';

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
                out << " -" << edge.second << "-> " << edge.first << ",";
            out << '\n';
        }

        out << "Final states: ";
        for(auto node:myFA.finalStates)
            out << node << ", ";
        out << '\n';

        return out; /// used read so I could write that inside the class.
   }
};

class l_NFA : public FA{

    typedef std::pair<int,int> IntPair;
    struct regExInfo{
        const std::string& regExp; /// the regular expression to be parsed
        char ch; /// the character being processed
        unsigned int pos = 0; /// position in parsing string (of next character)
        IntPair nodes = {-1,1}; /// first and last states of current expression.
        int bracketsCount = 0;

        regExInfo(const std::string& regExp) : regExp(regExp){}
        bool inline inWord(){return pos < regExp.size();}
    };
    bool letter(regExInfo& info){
        if(!acceptedLetter(info.ch))
             return false;

        if(info.nodes.first == -1)
            info.nodes.first = info.nodes.second;

        edges[info.nodes.second][LAMBDA].insert(info.nodes.second+1);
        info.nodes.second++;

        edges[info.nodes.second][info.ch].insert(info.nodes.second+1);
        info.nodes.second++;

        edges[info.nodes.second][LAMBDA].insert(info.nodes.second+1);
        info.nodes.second++;

        /// result looks like this: o -@-> o -ch-> o -@-> o, where o is a new state, @ is Lambda
        /// and ch is the edge character. Such a structure is necessary for an easier-to-implement union.

        return true;
    }
    bool KleeneStar(regExInfo& info, bool onlyOneElement = true){
        if(info.ch != '*' )
            return false;

        if(info.nodes.first == -1)
            throw std::invalid_argument("Starred nothing! (position: " + std::to_string(info.pos) + ") ");

        if(info.regExp[info.pos-2] == '*') /// starring a star
            return false;

        IntPair starredNodes = info.nodes;

        if(onlyOneElement)
            starredNodes = {info.nodes.second - 3, info.nodes.second};

        edges[starredNodes.second - 1][LAMBDA].insert(starredNodes.first + 1);
        edges[starredNodes.first + 1][LAMBDA].insert(starredNodes.second - 1);

        return true;
    }
    bool unionOp(regExInfo& info){
        if(info.ch != '|' && info.ch != '+')
            return false;
        if(info.nodes.first == -1)
            return false; /// union of empty set

        IntPair oldNodes = info.nodes;

        info.nodes.second++;
        readRegex(info);

        if(info.nodes.first == -1){
            info.nodes = oldNodes;
            return false; /// union of empty set
        }

        edges[oldNodes.first][LAMBDA].insert(info.nodes.first);
        edges[oldNodes.second][LAMBDA].insert(info.nodes.second);
        info.nodes.first = oldNodes.first;

        return true;
    }

    bool openBracket(regExInfo& info){
        if(info.ch != '(')
            return false;
        info.bracketsCount++;

        IntPair oldNodes = info.nodes;
        readRegex(info);

        if(info.inWord() && info.regExp[info.pos] == '*'){
            info.ch = '*';
            KleeneStar(info,false);
            info.pos++;
        }

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

        return true;
    }

    void readRegex(regExInfo& info){
        info.nodes.first = -1; /// denote not having a first node.

        while(info.inWord()){

            info.ch = info.regExp[info.pos];
            info.pos++;

            if(openBracket(info))
                continue;

            if(closeBracket(info))
                break; /// end subExpression.

            letter(info);

            KleeneStar(info);

            if(unionOp(info))
                break; /// (in case we were inside brackets), we need to end subexpression.

        }
    }
public:
    l_NFA(const std::string& regExp){
        std::string preParsedExp;

        for(char ch:regExp) /// remove all spaces.
            if(ch != ' ')
                preParsedExp.push_back(ch);

        regExInfo info(preParsedExp);
        try{
            readRegex(info);
            startState = info.nodes.first;
            finalStates.insert(info.nodes.second);
        }
        catch (const std::invalid_argument& e){
            std::cerr << "Invalid regular expression: " << e.what() << '\n';
            clear();
        }
    }
};


int main(){
    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");

    std::vector<std::string> prevTests = {
"a(*)", /// should show error message
"a()*", /// should show error message
"a(*+b)",
"a(())()b", /// same as "ab"
"a(())()+",
"a**",
"(a*+b + c)*",
"(((a*))*+b+c)*",
"a(+)bc*d",
"(a+b)*|c|d",
"c|a*"};

    for(auto input:prevTests){
        std::cout << "RUNNING INPUT: " << input << "\n";
        l_NFA test(input);
        std::cout << test << "\n\n\n";
    }

}

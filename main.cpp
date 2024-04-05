#include <iostream>
#include <fstream>
#include <map>
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

        if(myFA.finalStates.empty() && myFA.edges.empty())
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
class InfoToDFA;

class DFA : public FA{
    friend InfoToDFA;
};

struct InfoToDFA{
    DFA result;
    std::map<std::vector<int>,int> rename;
    int currentId = 1;
public:
    InfoToDFA(){}
    inline bool find(const std::vector<int>& nodes){
        return rename.find(nodes) != rename.end();
    }
    void add(const std::vector<int>& nodes){
        if(find(nodes))
            return;
        rename[nodes] = currentId;
        currentId++;
    }
    void addEdge(const std::vector<int>& nodes,char ch, const std::vector<int>& nextNodes){
        result.edges[ rename[nodes] ] [ch].insert(rename[nextNodes]);
    }
};

class NFA : public FA{

    void getDFA(InfoToDFA &info,std::vector<int> currentState){
        if(info.find(currentState))
            return;
        info.add(currentState);

        std::unordered_map<char,std::unordered_set<int> > allNextNodes;
        for(auto state:currentState){
            for(auto edge:edges[state]){
                for(auto node:edge.second){
                    allNextNodes[edge.first].insert(node);
                }
            }
        }


        for(auto edge: allNextNodes){
            std::vector<int> nextStates;

            char ch = edge.first;
            for(auto node:edge.second){
                nextStates.push_back(node);

            getDFA(info,nextStates);
            /// now nextStates will be in rename map

            info.addEdge(currentState,ch,nextStates);



            }
        }

    }
public:
    DFA getDFA(){
        /// o explozie de stari...
        InfoToDFA info;
        getDFA(info,{startState});
        return info.result;
    }
};

class l_NFA : public FA{

    typedef std::pair<int,int> IntPair;
    struct regExInfo{
        const std::string& regExp; /// the regular expression to be parsed
        char ch; /// the character being processed
        unsigned int pos = 0; /// position in parsing string (of next character)
        bool inUnion = false;
        IntPair nodes = {-1,1}; /// first and last states of current expression.
        int bracketsCount = 0;

        regExInfo(const std::string& regExp) : regExp(regExp){}
        bool inline inWord(){return pos < regExp.size();}

    };
    void inline push(regExInfo& info, const char& ch){
        edges[info.nodes.second][ch].insert(info.nodes.second+1);
        info.nodes.second++;
    }
    bool letter(regExInfo& info){
        if(!acceptedLetter(info.ch))
             return false;

        if(info.nodes.first == -1)
            info.nodes.first = info.nodes.second;

        push(info,LAMBDA);
        push(info,info.ch);

        /// result looks like this: o -@-> o -ch-> o, where o is a new state, @ is Lambda
        /// and ch is the edge character. Such a structure is necessary for an easier-to-implement union.

        return true;
    }
    bool KStarOne(regExInfo& info){
        if(info.ch != '*' )
            return false;
        makeKleeneStar(info,{info.nodes.second - 2, info.nodes.second});
        return true;
    }
    bool KStarMany(regExInfo& info){
        if(!info.inWord() || info.regExp[info.pos] != '*')
            return false;

        makeKleeneStar(info,info.nodes);
        info.pos++;
        return true;
    }
    void inline throwNothingStar(regExInfo info){
        if(info.nodes.first == -1)
            throw std::invalid_argument("Starred nothing! (position: " + std::to_string(info.pos) + ") ");
    }
    bool KStarUnion(regExInfo& info){
        if(!info.inWord() || info.regExp[info.pos] != '*')
            return false;
        throwNothingStar(info);


        for(auto node:edges[info.nodes.first][LAMBDA])
            edges[info.nodes.second][LAMBDA].insert(node);
        edges[info.nodes.first][LAMBDA].insert(info.nodes.second);

        info.pos++;
        return true;
    }

    void makeKleeneStar(regExInfo& info, IntPair starredNodes){
        throwNothingStar(info);

        edges[starredNodes.second][LAMBDA].insert(starredNodes.first + 1);
        edges[starredNodes.first + 1][LAMBDA].insert(starredNodes.second);
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
        info.inUnion = true;

        edges[oldNodes.first][LAMBDA].insert(info.nodes.first);

        push(info,LAMBDA);
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

        if(info.inUnion)
            KStarUnion(info);
        else
            KStarMany(info);

        info.inUnion = false;

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

            KStarOne(info);

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

void runRegexTests(){
        std::vector<std::string> prevTests = {
"a(*)", /// should show error message
"a()*", /// should show error message
"a(*+b)", /// should show error message
"a(())()b", /// same as "ab"
"a(++b++)", /// same as "ab"
"a(())()+", /// same as "a"
"(a**)**",      /// same as "a*"
"a*+b", /// 1st problem
"a+b*", /// 2nd

        ///allNextNodes.clear();problem
"c+(a+b)*", /// 3rd problem
"(a+b)*+c", /// 4th problem
"(a*+b + c)*",
"a(+)bc*d"};

    for(auto input:prevTests){
        std::cout << "RUNNING INPUT: " << input << "\n";
        l_NFA test(input);
        std::cout << test << "\n\n\n";
    }
}

int main(){

    /// runRegexTests();

    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");

    NFA test;
    fin >> test;


    DFA result = test.getDFA();

    std::cout << result;
}

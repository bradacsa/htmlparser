/* Includes */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>

#include <variant>
#include <typeinfo> 

/* HTML parser class */
class HTMLparser{
    public:

        /* HTML SearchResult */
        typedef std::vector<int> elementPath;
        typedef std::vector<elementPath> searchResult;

        /* Public search enumerators */
        enum searchBys{
            TAGNAME,
            ATTRIBUTE_NAME,
            ATTRIBUTE_VALUE,
            TEXT_CONTENT
        };

        /* Search options pack */
        struct searchOps{
            std::string value;
            searchBys by;
        };

        /* HTML Tag property structure */
        struct htmlProperty{
            std::string name;
            std::string value;
        };

        /* HTML Structure */
        struct htmlElement{
            std::string name;
            std::vector<htmlProperty> properties;
            std::variant<std::string, std::vector<htmlElement>> content;
        };
        
        /* Initialization of the class */
        HTMLparser(std::string htmlCode){
            int prs = 0;
            int *prsPointer = &prs;
            std::vector<htmlTag> tokenizedHTML = tokenizeHTML(htmlCode);
            std::vector<htmlTag> repairedTagPairs = repairTagPairs(tokenizedHTML);
            emittedHTML = emitHTML(repairedTagPairs, prsPointer);
        };
        
        /* Reconstruct a new HTML tree */
        void reParse(std::string htmlCode){
            emittedHTML.clear();
            int prs = 0;
            int *prsPointer = &prs;
            emittedHTML = emitHTML(tokenizeHTML(htmlCode), prsPointer);
        }
        
        /* Search element by tagname */
        /*searchResult search(std::string tagname){
            searchResult resultVector;
            searchResult* scPtr = &resultVector;

            //searchEngine(tagname, scPtr, emittedHTML);
            return resultVector;
        }*/
        
        /* Search by the given props */
        searchResult searchBy(std::vector<searchOps> searchParams){
            searchResult resultVector;
            std::vector<htmlElement> tmpElementList = emittedHTML;

            for(int i=0; i<searchParams.size(); i++){
                //resultVector.clear();
                //resultVector.shrink_to_fit();
                //resultVector = search(searchParams[i].value, searchParams[i].by, tmpElementList);
                searchResult(search(searchParams[i].value, searchParams[i].by, tmpElementList)).swap(resultVector);
                
                tmpElementList.clear();
                for(int j = 0; j < resultVector.size(); j++){
                    tmpElementList.push_back(getElementByPath(resultVector[j]));
                }
            }
            return resultVector;
        }

        /* Get element by the path */
        htmlElement getElementByPath(elementPath path){
            std::variant<std::string, std::vector<htmlElement>> content;
            content = emittedHTML;

            for(int i=0; i<path.size()-1; i++){
                auto tmpElement = std::get<std::vector<htmlElement>>(content)[path[i]].content;
                content = tmpElement; 
            }
            htmlElement result = std::get<std::vector<htmlElement>>(content)[path[path.size()-1]];
            return result;
        }
        
        /* Deconstruct class */
        ~HTMLparser(){};

    private:
        /* Parser states */
        enum states{
            START,
            TEXT,
            OPEN_TAG,
            SELFCLOSING_TAG,
            END_TAG,
            DOCT_OR_COMM,
            COMMENT_LIKE,
            COMMENT,
            COMMENT_END_LIKE,
            COMMENT_END,
            DOCTYPE,
            PROPERTIES,
            PROP_EQUALS,
            PROP_DQ,
            PROP_SQ
        } state;

        /* HTML tag types */
        enum htmlTypes{
            NONE,
            TEXT_CONT,
            TAG_OPEN,
            TAG_END,
            TAG_SELFCLOSING,
            TAG_DOCTYPE,
            TAG_COMMENT
        } htmlType;

        /* HTML Tag structure */
        struct htmlTag{
            std::string name;
            std::vector<htmlProperty> properties;
            htmlTypes type;
        };
        
        /* Struct for tag pairing */
        struct tagPair{
            std::string tagName;
            int placement;
        };

        /* Placeholder for emitted HTML document */
        std::vector<HTMLparser::htmlElement> emittedHTML;
        
        /* Illegal character warning */
        void illegalChar(std::string msg = ""){
            std::cout << msg << std::endl;
        };
        
        /* Tokenizing the HTML code */
        std::vector<htmlTag> tokenizeHTML(std::string htmlCode){
            /* Reset to START state */
            states State = states::START;

            const char *strCode = htmlCode.c_str();
            int htmlLen = strlen(strCode);
            int ptr = 0;

            /* Temporary variables */
            std::string tmpStr = ""; // To store the string letter by letter
            htmlProperty tmpProp; // To store the property name and value pairs
            htmlTag tmpTag; // To store the tag units
            std::vector<htmlTag> tmpTagVector; // To collect the tag units

            /* Initializing everything first (to avoid garbage values) */
            tmpProp.name = "";
            tmpProp.value = "";
            tmpTag.name = "";
            tmpTag.type = htmlTypes::NONE;

            // Looping through the HTML string
            while(ptr!=htmlLen){
                char db = strCode[ptr];
                switch(State){
                    case states::START:
                        switch(strCode[ptr]){
                            case '<':
                                State = states::OPEN_TAG;
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case ' ':
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                State = states::TEXT;
                                break;
                        }
                        break;
                    case states::TEXT:
                        switch(strCode[ptr]){
                            case '<':
                               if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpTag.type=htmlTypes::TEXT_CONT;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;

                                State = states::OPEN_TAG;
                                break;
                            case '>':
                                illegalChar();
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                    case states::OPEN_TAG:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                if(tmpTag.type==htmlTypes::NONE){
                                    tmpTag.type=htmlTypes::TAG_OPEN;
                                }
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                illegalChar();
                                break;
                            case '/':
                                if(tmpStr=="" && tmpTag.name==""){
                                    State = states::END_TAG;
                                }
                                
                                if(tmpStr!="" && tmpTag.name==""){
                                    State = states::SELFCLOSING_TAG;
                                }

                                if(tmpTag.name!=""){
                                    State = states::SELFCLOSING_TAG;
                                }
                                break;
                            case '!':
                                State = states::DOCT_OR_COMM;
                                break;
                            case '-':
                                illegalChar();
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                    tmpStr = "";
                                }
                                State = states::PROPERTIES;
                                break;
                            case '\n':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                    tmpStr = "";
                                }
                                State = states::PROPERTIES;
                                break;
                            default:
                                tmpStr+=strCode[ptr];
                                break;
                        }
                        break;
                    case states::SELFCLOSING_TAG:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpTag.type=htmlTypes::TAG_SELFCLOSING;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                illegalChar();
                                break;
                            case '/':
                                illegalChar();
                                break;
                            case '!':
                                illegalChar();
                                break;
                            case '-':
                                illegalChar();
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                break;
                            case '\n':
                                illegalChar();
                                break;
                            default:
                                tmpStr+=strCode[ptr];
                                break;
                        }
                        break;
                    case states::END_TAG:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpTag.type=htmlTypes::TAG_END;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                illegalChar();
                                break;
                            case '/':
                                illegalChar();
                                break;
                            case '!':
                                illegalChar();
                                break;
                            case '-':
                                illegalChar();
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr+=strCode[ptr];
                                break;
                        }
                        break;
                    case states::DOCT_OR_COMM:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                illegalChar();
                                break;
                            case '/':
                                illegalChar();
                                break;
                            case '!':
                                illegalChar();
                                break;
                            case '-':
                                State = states::COMMENT_LIKE;
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr+=strCode[ptr];
                                State = states::DOCTYPE;
                                break;
                        }
                        break;
                    case states::COMMENT_LIKE:
                        switch(strCode[ptr]){
                            case '-':
                                State = states::COMMENT;
                                break;
                            case '\n':
                                break;
                            default:
                                illegalChar();
                                break;
                        }
                        break;
                    case states::COMMENT:
                        switch(strCode[ptr]){
                            case '-':
                                State = states::COMMENT_END_LIKE;
                                break;
                            case '\n':
                                tmpStr += '\n';
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                    case states::COMMENT_END_LIKE:
                        switch(strCode[ptr]){
                            case '-':
                                State = states::COMMENT_END;
                                break;
                            case '\n':
                                tmpStr += '-'; tmpStr += '\n';
                                break;
                            default:
                                tmpStr += '-'; tmpStr += strCode[ptr];
                                State = states::COMMENT;
                                break;
                        }
                        break;
                    case states::COMMENT_END:
                        switch(strCode[ptr]){
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpTag.type = htmlTypes::TAG_COMMENT;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '\n':
                                tmpStr += '-'; tmpStr += '-'; tmpStr += '\n';
                                State = states::COMMENT;
                                break;
                            default:
                                tmpStr += '-'; tmpStr += '-'; tmpStr += strCode[ptr];
                                State = states::COMMENT;
                                break;
                        }
                        break;
                    case states::DOCTYPE:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpTag.type = htmlTypes::TAG_DOCTYPE;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                illegalChar();
                                break;
                            case '/':
                                illegalChar();
                                break;
                            case '!':
                                illegalChar();
                                break;
                            case '-':
                                illegalChar();
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                if(tmpStr!=""){
                                    tmpTag.name = tmpStr;
                                }
                                tmpStr = "";
                                tmpTag.type = htmlTypes::TAG_DOCTYPE;
                                State = states::PROPERTIES;
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                    case states::PROPERTIES:
                        switch(strCode[ptr]){
                            case '<':
                                illegalChar();
                                break;
                            case '>':
                                if(tmpStr!=""){
                                    tmpProp.name = tmpStr;
                                    tmpProp.value = "";
                                }
                                tmpTag.properties.push_back(tmpProp);
                                if(tmpTag.type == htmlTypes::NONE){
                                    tmpTag.type = htmlTypes::TAG_OPEN;
                                }
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '=':
                                if(tmpStr!=""){
                                    tmpProp.name = tmpStr;
                                }
                                else{
                                    illegalChar();
                                }
                                tmpStr = "";
                                State = states::PROP_EQUALS;
                                break;
                            case '/':
                                if(tmpStr!=""){
                                    tmpProp.name = tmpStr;
                                    tmpProp.value = "";
                                }
                                if(tmpProp.name!=""){
                                    tmpTag.properties.push_back(tmpProp);
                                }
                                tmpTag.type = htmlTypes::TAG_SELFCLOSING;
                                tmpTagVector.push_back(tmpTag);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                
                                State = states::START;
                                break;
                            case '!':
                                illegalChar();
                                break;
                            case '-':
                                illegalChar();
                                break;
                            case '"':
                                illegalChar();
                                break;
                            case '\'':
                                illegalChar();
                                break;
                            case ' ':
                                if(tmpStr!=""){
                                    tmpProp.name = tmpStr;
                                    tmpProp.value = "";
                                }
                                if(tmpProp.name!=""){
                                    tmpTag.properties.push_back(tmpProp);
                                }

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                    case states::PROP_EQUALS:
                        switch(strCode[ptr]){
                            case '>':
                                if(tmpProp.name!=""){
                                    tmpTag.properties.push_back(tmpProp);
                                }
                                tmpTagVector.push_back(tmpTag);

                                if(tmpTag.type == htmlTypes::NONE){
                                    tmpTag.type = htmlTypes::TAG_OPEN;
                                }

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                tmpTag.properties.clear();
                                tmpTag.name = "";
                                tmpTag.type = htmlTypes::NONE;
                                State = states::START;
                                break;
                            case '"':
                                State = states::PROP_DQ;
                                break;
                            case '\'':
                                State = states::PROP_SQ;
                                break;
                            case ' ':
                                break;
                            case '\n':
                                break;
                            default:
                                illegalChar();
                                break;
                        }
                        break;
                    case states::PROP_DQ:
                        switch(strCode[ptr]){
                            case '"':
                                if(tmpStr!=""){
                                    tmpProp.value = tmpStr;
                                }

                                if(tmpTag.type == htmlTypes::NONE){
                                    tmpTag.type = htmlTypes::TAG_OPEN;
                                }
                                tmpTag.properties.push_back(tmpProp);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                State = states::OPEN_TAG;
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                    case states::PROP_SQ:
                        switch(strCode[ptr]){
                            case '\'':
                                if(tmpStr!=""){
                                    tmpProp.value = tmpStr;
                                }

                                if(tmpTag.type == htmlTypes::NONE){
                                    tmpTag.type = htmlTypes::TAG_OPEN;
                                }
                                tmpTag.properties.push_back(tmpProp);

                                tmpStr = "";
                                tmpProp.name = "";
                                tmpProp.value = "";
                                State = states::OPEN_TAG;
                                break;
                            case '\n':
                                break;
                            default:
                                tmpStr += strCode[ptr];
                                break;
                        }
                        break;
                }
                ptr++;
            }
            return tmpTagVector;
        };

        /* Find tag pairs to prevent SELFCLOSING_TAG fails */
        std::vector<htmlTag> repairTagPairs(std::vector<htmlTag> htmlElements){
            std::vector<tagPair> tags;
            int i=0;

            /* Find the OPEN_TAG-ed SELFCLOSING_TAG-s */
            tagPair tmpTag;
            int j = htmlElements.size();
            while(i<htmlElements.size()){
                switch(htmlElements[i].type){
                    case htmlTypes::TAG_OPEN:
                        tmpTag.placement = i;
                        tmpTag.tagName = htmlElements[i].name;
                        tags.push_back(tmpTag);
                        break;
                    case htmlTypes::TAG_END:
                        for(auto iter = tags.rbegin(); iter != tags.rend(); ++iter){
                            if(iter->tagName==htmlElements[i].name){
                                tags.erase(--(iter.base()));
                                break;
                            }
                        }
                        break;
                }
                i++;
            }

            /* Change them to SELFCLOSED_TAG-s */
            for(int i=0; i < tags.size(); i++){
                htmlElements[tags[i].placement].type = htmlTypes::TAG_SELFCLOSING;
            }
            return htmlElements;
        }
            
        /* Emitting tokenized HTML Elements */
        std::vector<htmlElement> emitHTML(std::vector<htmlTag> htmlElements, int *reachedPtr){
            std::vector<htmlElement> tmpDocument;
            htmlElement tmpNode;

            /*
                These types are all STOP'N'SAVE (S'N'S) types
                    TEXT_CONT <-
                    TAG_OPEN
                    TAG_END <-
                    TAG_SELFCLOSING <-
                    TAG_DOCTYPE <-
                    TAG_COMMENT <-

                At TAG_OPEN we need to call the function again.
                If we reach a S'N'S we need to save it to tmpDocument and then go forward.
            */
            int i = *reachedPtr;

            while(i < htmlElements.size()){
                switch(htmlElements[i].type){
                    case htmlTypes::TAG_OPEN:
                        tmpNode.name = htmlElements[i].name;
                        tmpNode.properties = htmlElements[i].properties;
                        i++;
                        tmpNode.content = emitHTML(htmlElements, &i);
                        *reachedPtr = i;
                        tmpDocument.push_back(tmpNode);

                        tmpNode.content = "";
                        tmpNode.name.clear();
                        tmpNode.properties.clear();

                        break;
                    case htmlTypes::TAG_END:
                        *reachedPtr = i;
                        return tmpDocument;
                        break;

                    case htmlTypes::TEXT_CONT:
                        tmpNode.content = htmlElements[i].name;
                        tmpNode.name = "text";
                        tmpNode.properties = htmlElements[i].properties;
                        tmpDocument.push_back(tmpNode);

                        tmpNode.content = "";
                        tmpNode.name.clear();
                        tmpNode.properties.clear();
                        break;
                    case htmlTypes::TAG_SELFCLOSING:
                        tmpNode.content = "";
                        tmpNode.name = htmlElements[i].name;
                        tmpNode.properties = htmlElements[i].properties;
                        tmpDocument.push_back(tmpNode);

                        tmpNode.content = "";
                        tmpNode.name.clear();
                        tmpNode.properties.clear();
                        break;
                    case htmlTypes::TAG_DOCTYPE:
                        tmpNode.content = "";
                        tmpNode.name = "doctype";
                        tmpNode.properties = htmlElements[i].properties;
                        tmpDocument.push_back(tmpNode);

                        tmpNode.content = "";
                        tmpNode.name.clear();
                        tmpNode.properties.clear();
                        break;
                    case htmlTypes::TAG_COMMENT:
                        tmpNode.content = htmlElements[i].name;
                        tmpNode.name = "comment";
                        tmpNode.properties = htmlElements[i].properties;
                        tmpDocument.push_back(tmpNode);

                        tmpNode.content = "";
                        tmpNode.name.clear();
                        tmpNode.properties.clear();
                        break;
                }
                i++;
            }
            return tmpDocument;
        };
        
        /* Searchengine */
        searchResult search(std::string value, searchBys by, std::vector<htmlElement> searchIn, std::vector<int> path = {}){
            searchResult res;

            int i = 0;
            elementPath tmpPath;
            tmpPath = path;

            while(i<searchIn.size()){
                tmpPath.push_back(i);
                htmlElement elementInPos = getElementByPath(tmpPath);
                switch(by){
                    case searchBys::TAGNAME:
                        if(value==elementInPos.name){
                            res.push_back(tmpPath);
                        }
                        break;
                    case searchBys::ATTRIBUTE_NAME:
                        for(int j = 0; j < elementInPos.properties.size(); j++){
                            if(value==elementInPos.properties[j].name){
                                res.push_back(tmpPath);
                            }
                        }
                        break;
                    case searchBys::ATTRIBUTE_VALUE:
                        for(int j = 0; j < elementInPos.properties.size(); j++){
                            if(value==elementInPos.properties[j].value){
                                res.push_back(tmpPath);
                            }
                        }
                        break;
                }

                if(elementInPos.content.index()==1){
                    searchResult deepSrch = search(value, by, std::get<std::vector<htmlElement>>(elementInPos.content), tmpPath);
                    for(int c = 0; c<deepSrch.size(); c++){
                        res.push_back(deepSrch[c]);
                    }
                }

                i++;
                tmpPath.pop_back();
            }
            return res;
        }
};

/* Main sequence */
int main(int argc, char **argv){
    
    //std::string HTML = "<!DOCTYPE html><a href=\"index.html\">Link to <b>index</b> page</a><br/><b>Hello</b><!--This is a comment - you can do whatever you want -- or it's gonna blow up...-->";
    
    std::string line = "", HTML="";
    std::ifstream myfile ("index.html");
        while (getline(myfile,line))
        {
            HTML += line + '\n';
        }
    myfile.close();

    HTMLparser parser = HTMLparser(HTML);
    //HTMLparser::searchResult result = parser.search("meta");
    //HTMLparser::htmlElement e = parser.getElementByPath(result[0]);
    HTMLparser::searchResult res = parser.searchBy({{"meta", HTMLparser::searchBys::TAGNAME}, {"property", HTMLparser::searchBys::ATTRIBUTE_NAME}, {"article:published_time", HTMLparser::searchBys::ATTRIBUTE_VALUE}});
 
    return 0;
}

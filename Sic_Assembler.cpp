#include<algorithm>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<sstream>
#include<stdio.h>
#include<math.h>
#include<vector>
#include<string>
#include<map>
//#include<asmblr.h>

using namespace std;

vector<vector<string> > programCode;
vector<string> HTEformat;
vector<string> objectCode;
map<string,string> opcode;
map<string,string> symbolTable;
vector<string> locationCounter;


// asmblr.h
///////////////////////////////////////////////
int myPow(int x,int power)
{
    int result=1;
    for(int i=0;i<power;i++)
        result*=x;
    return result;
}

int convertIntStringToInt(string x)
{
    int result=0,c=0;
    for(int i=x.size()-1;i>=0;i--)
    {
        result+=( (x[i]-'0') * myPow(10,c)  );
        c++;
    }
    return result;
}
int convertHexStringToInt(string x)
{
    map<char,int> HEX;
    HEX['0']=0;HEX['1']=1;HEX['2']=2;HEX['3']=3;HEX['4']=4;HEX['5']=5;HEX['6']=6;HEX['7']=7;
    HEX['8']=8;HEX['9']=9;HEX['A']=HEX['a']=10;HEX['B']=HEX['b']=11;HEX['C']=HEX['c']=12;
    HEX['D']=HEX['d']=13;HEX['E']=HEX['e']=14;HEX['F']=HEX['f']=15;

    int result=0,c=0;
    for(int i=x.size()-1;i>=0;i--)
    {
        result+=( HEX[x[i]] * myPow(16,c)  );
        c++;
    }
    return result;
}

string toUpperCase(string x)
{
    for(int i=0;i<x.size();i++)
        x[i] = toupper(x[i]);
    return x;
}

string convertIntToHexString(int number)
{
    char x[10];
    itoa(number,x,16);
    string y(x);
    while(y.size()<4)
        y.insert(0,"0");
    y=toUpperCase(y);
    return y;
}

vector<string> parse(char line[])
{
    int len=strlen(line),i;
    vector<string> lineParsed;
    string temp;
    for(i=0;i<len;i++)
    {
        if(line[i]==' ' || line[i]=='\t' || line[i]=='\n')
        {
            if(temp.size()!=0)
                lineParsed.push_back(temp);
            temp.clear();
        }
        else
            temp+=line[i];
    }
    if(temp.size()!=0)
        lineParsed.push_back(temp);
    return lineParsed;
}

map<string,string> getOpcode()
{
    map<string,string> opcode;
    FILE *opcodeInput;
    vector<string> temp;
    char line[20];

    opcodeInput = fopen ("OpcodeLoader.txt", "r");

    while(fgets(line,20,opcodeInput) != NULL)
    {
        temp=parse(line);
        opcode[temp[0]]=temp[1];
        temp.clear();
    }
    fclose(opcodeInput);
    return opcode;
}


string convertCharsToAsciiString(string x)
{
    int y=0;
    x=toUpperCase(x);
    string ret,temp;
    char arr[5];
    for(int i=0;i<x.size();i++)
    {
        itoa(x[i],arr,16);
        temp=arr;
        if(temp.size()<2)temp.insert(0,"0");
        ret+=temp;
    }
    return ret;
}

void printSymbol(map<string,string> symbolTable)
{
    map<string,string>::iterator it=symbolTable.begin();
    for(it=symbolTable.begin();it!=symbolTable.end();++it)
        cout<< it->first << "-->" << it->second << endl;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


string getStartingAddress()
{
    return programCode[0][2];
}

// 0 for instruction , 1 for a label and instruction, 2 for data, 3 for comment
int getType(int index)
{
    if(programCode[index][0]==".")
        return 3;
    if(opcode.count(programCode[index][0])>0)
        return 0;
    if(programCode[index][1]=="WORD" ||
       programCode[index][1]=="BYTE" ||
       programCode[index][1]=="RESW" ||
       programCode[index][1]=="RESB"  )
        return 2;
    return 1;
}

string incrementCounter(string counter,int addedValue)
{
    int start=convertHexStringToInt(counter);
    start+=addedValue;
    return convertIntToHexString(start);
}

string getProgramCodeLength()
{
    int start=convertHexStringToInt(locationCounter[1]);
    int end=convertHexStringToInt(locationCounter[locationCounter.size()-1]);
    string length=convertIntToHexString(end-start);
    while(length.size()<6)length.insert(0,"0");
    return length;
}

void passOne()
{
    string counter=getStartingAddress();
    locationCounter.push_back(counter);
    symbolTable[programCode[0][0]]=locationCounter[0];
    locationCounter.push_back(counter);
    int type,value,l=1;
    for(int i=1;i<programCode.size()-1;i++)
    {
        type=getType(i);

        if(type==3)
            continue;

        if(type==0)
            counter=incrementCounter(counter,3);
        else
        {
            if(symbolTable.count(programCode[i][0])==0)
                symbolTable[programCode[i][0]]=locationCounter[l];
            if(type == 1)
                counter=incrementCounter(counter,3);
            else
            {
                if(programCode[i][1]=="RESB")
                {
                    value=convertIntStringToInt(programCode[i][2]);
                    counter=incrementCounter(counter,value);
                }
                else if(programCode[i][1]=="RESW")
                {
                    value=convertIntStringToInt(programCode[i][2])*3;
                    counter=incrementCounter(counter,value);
                }
                else if(programCode[i][1]=="WORD")
                    counter=incrementCounter(counter,3);
                else if(programCode[i][1]=="BYTE")
                {
                    if(programCode[i][2][0]=='C')
                    {
                        int size=programCode[i][2].size();
                        size-=3;
                        counter=incrementCounter(counter,size);
                    }
                    else
                        counter=incrementCounter(counter,1);
                }
            }
        }

        locationCounter.push_back(counter);
        l++;
    }
}

void passTwo()
{
    string out,variable;
    int value;
    for(int i=1;i<programCode.size()-1;i++)
    {
        out.clear();

        //indexed
        if(programCode[i][programCode[i].size()-1]=="X")
        {
            if(programCode[i].size()==3)
            {
                out=opcode[programCode[i][0]];
                variable=programCode[i][1].substr(0,programCode[i][1].size()-1);
                variable=symbolTable[variable];
                out+=incrementCounter(convertIntToHexString(convertHexStringToInt(variable)),myPow(2,15));
            }
        }

        else if(programCode[i].size()==1)
        {

            if(getType(i)==3)continue;
            else //RSUB
            {
                out=opcode[programCode[i][0]];
                out+="0000";
            }
        }

        else if(programCode[i].size()==2)
        {
            out=opcode[programCode[i][0]];
            out+=symbolTable[programCode[i][1]];
        }
        else
        {
            if(opcode.count(programCode[i][1])>0)
            {
                out=opcode[programCode[i][1]];
                out+=symbolTable[programCode[i][2]];
            }
            else
            {
                if(programCode[i][1]=="WORD")
                    out="00"+convertIntToHexString(convertIntStringToInt(programCode[i][2]));
                else if(programCode[i][1]=="BYTE")
                {
                    if(programCode[i][2][0]=='X')
                        out=programCode[i][2].substr(2,programCode[i][2].size()-3);
                    else
                        out=convertCharsToAsciiString(programCode[i][2].substr(2,programCode[i][2].size()-3));
                }
            }
        }
        if(out.size())
            objectCode.push_back(out);
    }
}

void addHpart()
{
    string temp,progName;
    temp+="H.";
    progName=programCode[0][0];
    while(progName.size()<6)progName+=' ';
    temp+=progName+".";//program name
    temp+="00"+getStartingAddress()+'.';//starting address;
    temp+=getProgramCodeLength();
    HTEformat.push_back(temp);
}

void addTpart()
{
    string temp,start,prefix,length,end,x;
    bool newRecord=0;
    int counter=0,objectCodeCounter=0,counterFlag=0;

    start="00"+getStartingAddress();

    prefix="T."+start+'.';
    int l=1;
    //printf("%d\n",programCode.size()-1);
    for(int i=1;i<programCode.size()-1;i++)
    {

        if(getType(i)==3)continue;
        if(programCode[i].size()>1 && (programCode[i][1]=="RESB" || programCode[i][1]=="RESW" || counter==10))
        {
            if(counter==10)
                counterFlag=1;
            if(temp.size())
            {
                end=locationCounter[l];
                length=convertIntToHexString(convertHexStringToInt(end) - convertHexStringToInt(start));
                prefix+=length.substr(2,length.size());
                prefix+=temp;
                HTEformat.push_back(prefix);
            }
            temp.clear();
            counter=0;
            newRecord=1;
            if(!counterFlag)l++;
            if(!counterFlag)continue;
        }
        if(newRecord)
        {
            counterFlag=0;
            newRecord=0;
            start= "00"+locationCounter[l];
            prefix="T."+start+'.';
        }

        temp+='.'+objectCode[objectCodeCounter++];
        counter++;
        l++;

    }
    if(temp.size())
    {
        end=locationCounter[locationCounter.size()-1];
        length=convertIntToHexString(convertHexStringToInt(end) - convertHexStringToInt(start));
        prefix+=length.substr(2,length.size());
        prefix+=temp;
        HTEformat.push_back(prefix);
    }

}

void addEpart()
{
    string temp="E.00"+getStartingAddress();
    HTEformat.push_back(temp);
}

void generateHTEformat()
{
    addHpart();
    addTpart();
    addEpart();
}

void print(vector<string> v)
{
    for(int i=0;i<v.size();i++)
        cout << v[i]<< endl;
}
int main()
{
   // freopen("in2.txt","r",stdin);
   // freopen("out.txt","w",stdout);
    opcode=getOpcode();
    char inputLine[200];

    while(gets(inputLine)){
        programCode.push_back(parse(inputLine));
    }

    passOne();
    passTwo();
    generateHTEformat();
    print(HTEformat);

	return 0;
}

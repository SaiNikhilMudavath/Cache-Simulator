#include<iostream>
#include<vector>
#include<fstream>
#include<string>

using namespace std;

class block
{
    public:
    bool validbit=false;
    string tag="";
    int recent_use=0;
    bool dirtybit=false;

};

vector<vector<block>> allsets;
int loadhits=0;int loadmiss=0;int storehits=0;int storemiss=0;
int totalloads=0;int totalstores=0;
long long clockcycles=0;

//number conversions
int log2(int n)
{
    int result = -1;
    while(n)
    {
        n>>=1;
        ++result;
    }
    return result;
}
long long bintodec(string binary)
{
    long long n=0;long long base=1;
    for(int i=binary.size()-1;i>=0;i--)
    {
        if(binary[i]=='1')
        {
            n+=base;
        }
        base*=2;
    }
    return n;
}
std::string hextobin(string input)
{
    std::string result="";
    for(size_t i=0;i<input.size();i++)
    {
        if(input[i]=='0')
        {
            result+="0000";
        }
        else if(input[i]=='1')
        {
            result+="0001";
        }
        else if(input[i]=='2')
        {
            result+="0010";
        }
        else if(input[i]=='3')
        {
            result+="0011";
        }
        else if(input[i]=='4')
        {
            result+="0100";
        }
        else if(input[i]=='5')
        {
            result+="0101";
        }
        else if(input[i]=='6')
        {
            result+="0110";
        }
        else if(input[i]=='7')
        {
            result+="0111";
        }
        else if(input[i]=='8')
        {
            result+="1000";
        }
        else if(input[i]=='9')
        {
            result+="1001";
        }
        else if((input[i]=='A')||(input[i]=='a'))
        {
            result+="1010";
        }
        else if((input[i]=='B')||(input[i]=='b'))
        {
            result+="1011";
        }
        else if((input[i]=='C')||(input[i]=='c'))
        {
            result+="1100";
        }
        else if((input[i]=='D')||(input[i]=='d'))
        {
            result+="1101";
        }
        else if((input[i]=='E')||(input[i]=='e'))
        {
            result+="1110";
        }
        else if((input[i]=='F')||(input[i]=='f'))
        {
            result+="1111";
        }
    }
    return result;
}
vector<string> parse(string input)
{
    vector<string> result;
    string oper(1,input[0]);
    string address = hextobin(input.substr(4,8));
    result.push_back(oper);
    result.push_back(address);
    return result;
}
vector<vector<string>> read_trace(string tracefile)
{
    std::ifstream file(tracefile);
    if(!file.is_open())
    {
        std::cerr <<tracefile+" cant be opened"<<endl;
        std::exit(EXIT_FAILURE);
    }
    string line;
    vector<vector<string>> instructions;
    while(std::getline(file,line))
    {
        vector<string> a=parse(line);
        instructions.push_back(a);
    }
    file.close();
    return instructions;
}

int cache_load(int n_sets,string instruction,string lru,int i,int offset,bool ls,int n_bytes)
{
    int logvar=log2(n_sets);
    int set_index=bintodec(instruction.substr(32-logvar-offset,logvar));
    set_index=set_index%n_sets;
    string tagfield=instruction.substr(0,(32-logvar-offset));
    int focus_block_index;

    //set index means the set to which this address corresponds to 
    bool load_hit=false;int empty_index=0;;bool found_empty_index=false;
    
    for(size_t j=0;j<allsets[set_index].size();j++)
    {
        if((allsets[set_index][j].validbit)&&(allsets[set_index][j].tag==tagfield))
        {
            //load hit

            load_hit=true;
            allsets[set_index][j].recent_use=i;
            focus_block_index=0;
            if(ls)
            {
                loadhits++;
            }
            clockcycles++;
        }
        else if((allsets[set_index][j].validbit==false)&&(!found_empty_index))
        {
            empty_index=j;
            found_empty_index=true;
        }
    }
    
    if((!load_hit)&&(found_empty_index))
    {
        //load miss
        allsets[set_index][empty_index].validbit=true;
        allsets[set_index][empty_index].tag=tagfield;
        allsets[set_index][empty_index].recent_use=i;
        focus_block_index=empty_index;
        if(ls)
        {
            loadmiss+=1;
        }
        clockcycles+=(25*n_bytes)+1;
    }
    else if ((lru=="lru") &&(!load_hit)&&(!found_empty_index))
    {
        //implement lru or fifo

        /*find the least recently used block. how do you find ? get all the blocks' 
        recent_use variable and sort them and get the block number and replace that */

        vector<block> focus_set=allsets[set_index];
        int lru_index=0;
        for(size_t h=1;h<focus_set.size();h++)
        {
            if(focus_set[h].recent_use<focus_set[lru_index].recent_use)
            {
                lru_index=h;
            }
        }
        /*now we got the lru index. now replace the block with the new tag field.*/
        allsets[set_index][lru_index].tag=tagfield;
        allsets[set_index][lru_index].recent_use=i;
        focus_block_index=lru_index;
        if(ls)
        {
            loadmiss+=1;
        }
        clockcycles+=(25*n_bytes)+1;

        /*for counting the clock cycles we have to check the dirty bit and change the dirty bit*/
        if(allsets[set_index][lru_index].dirtybit==true)
        {
            clockcycles+=(25*n_bytes);
            allsets[set_index][lru_index].dirtybit=false;
        }
    }
    else if ((lru=="fifo") && (!load_hit)&&(!found_empty_index))
    {
        /*we have to replace the first in element track that using the index again*/
        
        vector<block> new_block_snippet;
        for(size_t e=1;e<allsets[set_index].size();e++)
        {
            new_block_snippet.push_back(allsets[set_index][e]);
        }

        block new_block;
        new_block.dirtybit=false;new_block.recent_use=i;new_block.validbit=true;
        new_block.tag=tagfield;
        new_block_snippet.push_back(new_block);
        allsets[set_index]=new_block_snippet;
        focus_block_index=allsets[set_index].size();
        /*for counting the clock cycles we have to check the dirty bit*/
        if(ls)
        {
            loadmiss+=1;
        }
        clockcycles+=(25*n_bytes)+1;
        if(allsets[set_index][(allsets[set_index].size()-1)].dirtybit==true)
        {
            clockcycles+=(25*n_bytes);
            allsets[set_index][allsets[set_index].size()-1].dirtybit=false;
        } 
    }
    return focus_block_index;
}

int main(int argc,char* argv[])
{
    int n_sets= std::stoi(argv[1]);
    int n_blocks=std::stoi(argv[2]);
    int n_bytes= std::stoi(argv[3]);
    string wa=argv[4];
    string wtwb=argv[5];
    string lru=argv[6];

    vector<vector<string>> instructions;
    allsets.resize(n_sets);
    for(int i=0;i<n_sets;i++)
    {
        allsets[i].resize(n_blocks);
    }
    int offset=log2(n_bytes);

    string line;
    while(std::getline(cin,line))
    {
        vector<string> ins=parse(line);
        instructions.push_back(ins);
    }

    for(size_t i=0;i<instructions.size();i++)
    {   
        if(instructions[i][0]=="l")
        {
            cache_load(n_sets,instructions[i][1],lru,i,offset,true,n_bytes);
            totalloads++;
        }
        else if (instructions[i][0]=="s")
        {
            totalstores++;

            int logvar=log2(n_sets);
            int set_index=bintodec(instructions[i][1].substr(32-logvar-offset,logvar));
            set_index=set_index%n_sets;
            string tagfield=instructions[i][1].substr(0,32-logvar-offset);

            bool store_hit=false;
            for(size_t j=0;j<allsets[set_index].size();j++)
            {
                if((allsets[set_index][j].validbit)&&(allsets[set_index][j].tag==tagfield))
                {
                    //store hit
                    store_hit=true;
                    /*independent of write allocate .
                    just check if it is write through or write back.*/
                    if(wtwb=="write-through")
                    {
                        /*writing to memory. just increase the clock cycles
                        and change the lru */
                        allsets[set_index][j].recent_use=i;
                        clockcycles+=101;
                    }
                    else if (wtwb=="write-back")
                    {
                        /*we write this when we evict the block during store. so we
                        change the dirty bit only and change lru.*/
                        allsets[set_index][j].recent_use=i;
                        allsets[set_index][j].dirtybit=true;
                        clockcycles++;
                    }
                    storehits++;
                }
            }
            if(!store_hit)
            {
                //store miss
                /*we try to load the address to the cache and then change using 
                write through or write back but even first verify the write allocate 
                or no write allocate.*/
                
                if(wa=="write-allocate")
                {
                    //write to cache and then write to memory
                    int focus_block_index=cache_load(n_sets,instructions[i][1],lru,i,offset,false,n_bytes);
                    /*now we got the block into cache. now write the data and
                    if it is write back or write through then update the cycles.*/

                    /*now we got the focus block index. so we write back 
                    or write through into it.*/
                    if(wtwb=="write-through")
                    {
                        allsets[set_index][focus_block_index].recent_use=i;
                        clockcycles+=(25*n_bytes)+1;
                    }
                    else if(wtwb=="write-back")
                    {
                        allsets[set_index][focus_block_index].recent_use=i;
                        allsets[set_index][focus_block_index].dirtybit=true;
                        clockcycles+=1;
                    }
                }
                else if (wa=="no-write-allocate")
                {
                    /* just directly write to memory. nothing to do with cache
                    just increase the clock cycles. */
                    clockcycles+=100;
                }
                storemiss++;
            }
        }   
        else
        {
            cout<<"invalid instruction l/s"<<endl;
        }
    }

    cout<<"Total loads: "<<totalloads<<endl;
    cout<<"Total stores: "<<totalstores<<endl;
    cout<<"Load hits: "<<loadhits<<endl;
    cout<<"Load misses: "<<loadmiss<<endl;
    cout<<"Store hits: "<<storehits<<endl;
    cout<<"Store misses: "<<storemiss<<endl;
    cout<<"Clock Cycles: "<<clockcycles<<endl;

}

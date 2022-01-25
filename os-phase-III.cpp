//-----------------LIBRARY------------------------

#include<iostream>
#include<fstream>  // library for read and write file
#include<string>   // library to use string data type in cpp
#include <cstring>  // special string operation by converting it into char
#include<sstream>
#include <vector>
#include <algorithm>
#include<queue>
#include<windows.h>
//#include <cstdlib>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

using namespace std;

// ---------------------------- PCB ----------------------------------------------------------------------------------------------------------------------------------
class PCB
{
    public:
        int job_id;
        int TTL;
        int TLL;
        int TTC;
        int TLC;
        char flag;
        int STPC;      //starting address of track
        int ETPC;
        int STDC;      //starting address of track
        int ETDC;
        int Pcount;  // program card count
        int Dcount ;  // Data card count
        int ptr;

        PCB()
        {
            job_id = 0;
            this->TTL = 0;
            this->TLL = 0;
            this->TTC = 0;
            this->TLC = 0;
            this->flag = ' ';
            this->STPC = -1;
            this->ETPC = -1;
            this->Pcount = 0;
            this->Dcount = 0;
            this ->ptr = -1;
        }

        void init(int jI,int TTL, int TLL, char ff, int ptr)
        {
            job_id = jI;
            this->TTL = TTL;
            this->TLL = TLL;
            this->TTC = 0;
            this->TLC = 0;
            this->flag = ff;
            this->ptr = ptr;
            this->STPC = -1;
            this->ETPC = -1;
            this->STDC = -1;
            this->ETDC = -1;
            //this->Pcount++;
            //this->Dcount++;
        }
        void show()
        {
            cout<<" PTR  : "<<ptr<<"\n ----------\n ID   : "<<job_id<<"\n ----------\n TTL  : "<<TTL<<"\n ----------\n TLL  : "<<TLL<<"\n ----------\n Flag : "<<flag<<"\n ----------\n STPC : "<<STPC<<"\n ----------\n ETPC : "<<ETPC<<"\n ----------\n STDC : "<<STDC<<"\n ----------\n ETDC : "<<ETDC<<"\n ----------\n PC   : "<<Pcount<<"\n ----------\n DC   : "<<Dcount<<endl;
        }

};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//---------------------------- Class for BUFFER ----------------------------------------------------------------------------------------------------------------------

class B
{
public :
    char b[41];
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------


//****  Global Variables  ***// --------------------------------------------------------------------------------------------------------------------------------------

char M[300][4];  // declared memory to store program and data cards
char HD[500][4];
int m;         // used to traverse defined memory
char IR[4];      // Instruction register to store 4 byte instruction from program card
int IC;           // Instruction counter points to next instruction going to execute
char R[4];         // General purpose register 'R' used for LR ,SR , CR instructions
int c;              // toggle Register used as a flag
int TS;             // time slice
int TSC ;            // time slice counter

int SI;           // Service Interrupt
int PI;
int TI;
int IOI = 1;           // Time Interrupt
char PTR[4];        // page table register

int VA;            // Virtual Address
int RA;           // Real Address
string TASK = "";
//int EM;             // Error MSG

int IRi;
vector<int> frames;
PCB P[10];

char buffer[41];            // intermediate buffer for data transmition.
//char long_buf[120];
string s;
//char b[10][4];

// BUFFER Q --------------------------------------------------------------------------------

queue<B> EBQ;  // empty buffer q
queue<B> IFBQ;  //Input Full buffer q
queue<B> OFBQ;   // Output full Buffer q

// PCB Q -------------------------------------------------------------------------------------

queue<PCB> LQ;
queue<PCB> RDQ;
queue<PCB> IOQ;
queue<PCB> TQ;

//---------------------------------------------------------------------------------------------

int head =0;
int j=0;
int rj =0;
int UT=0;   // Universal Timer

//channel attribute
int CHt[3],CHs[3];
int CHTt[3]={5,5,2};

///////////////////////////////////////////////////////////////////////////////----------------------------------------------------------------------------------


// Functions -----------------------------------------------------------------------------------------------------------------------------------------------------


// 00 --------------------------------------------------------------------------------------
void init()
{                                   // function to initialize global variables and arrays
    for(int i =0;i<500;i++)
    {
        for(int j=0;j<4;j++)
        {
            if (i<300)
                M[i][j]='\0';
                HD[i][j]='\0';
        }
        if(i<4)
        {
            IR[i]='\0';
            R[i]='\0';
            //buffer[i]='\0';
            PTR[i]='\0';
        }

    }




    IC=0;
    c;
    m=0;
    SI=0;
    PI=0;
    TI=0;
    frames = {1,10,29,13,4,25,16,7,24,9,0,11,28,3,14,26,6,17,18,19,20,21,12,23,8,5,15,27,21,2};
}

//-----------------------------------------------------------------------------------------------------------------

//01 --------------------------------------------------------------------------------------------------------------


void show_memory()              // to display content of defined memory 'M'
{
    cout<<"\n\n";
    for(int i =0;i<300;i++)
    {                                   //  representation of memory.
        if (i<10)
        {
            cout<<"M["<<" "<<i<<"] |  ";
        }
        else
        {
            cout<<"M["<<i<<"] |  ";
        }
        for(int j =0;j<4; j++)
            cout<<M[i][j]<<"  |  ";
        cout<<"\n_______________________________\n";
        if((i%10)==9)
            cout<<"\n________________________________\n";
    }
    cout<<"\n";
}

//--------------------------------------------------------------------------------------------------------------------

void show_HD()              // to display content of defined memory 'M'
{
    cout<<"\n\n";
    for(int i =0;i<300;i++)
    {                                   //  representation of memory.
        if (i<10)
        {
            cout<<"HD["<<" "<<i<<"] |  ";
        }
        else
        {
            cout<<"HD["<<i<<"] |  ";
        }
        for(int j =0;j<4; j++)
            cout<<HD[i][j]<<"  |  ";
        cout<<"\n_______________________________\n";
        if((i%10)==9)
            cout<<"\n________________________________\n";
    }
    cout<<"\n";
}

//---------------------------------------------------------------------------------------------------------------------------

int Random_Gen(vector<int> &v)
{
    int i=0;
	random_shuffle(v.begin(), v.end());
	int r = v.front();
	v.erase(v.begin());
	//cout<<r<<endl;
	random_shuffle(v.begin(), v.end());
	return r;
}

//---------------------------------------------------------------------------------------------------------------------------

void init_PT(int ptr )
{
    for(int j=ptr;j<ptr+10;j++)
    {
        for (int z=0;z<4;z++)
        {
            M[j][z]='#';
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------

int AddM(int VA)
{
    /* for(int x =0;x<4; x++)
    {
        cout<<"ptr "<<x<< "  "<<PTR[x]<<endl;
    }*/

    int pte = RDQ.front().ptr * 10 + VA/10;
    cout<<"pte :"<<pte<<"  "<<VA<<endl;
    RA = ((((int)M[pte][2] - 48)*10 + (int)M[pte][3] - 48 )*10)+ VA%10;
    cout<<"RA : "<<RA<<endl;
    return RA;
}

//---------------------------------------------------------------------------------------------------------------------------

void clear_buf(B buf)        // to clear buffer for using it repetitively
{
    for(int i =0;i<41;i++)
    {
        buf.b[i]='\0';
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

void Allocate( int SA)
{

}


// Function Declaration ---------------------------------------------------------------------------------------------------------------------------------------------

void MOS(char c);
void StartCH(int i);
void exe_user_pro();
void start_exe();
void simulation();
void loading();

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

// I/O files -------------------------------------------------------------------------------------------------------------------------------------------------------

ifstream inf("job.txt");      //opening input file in read mode
ofstream outf("output.txt");    //opening output file in write mode


//---------------------------------------------------------------------------------------------------------------------------


void Read()
{
    cout<<"R";              // function to read the data card from input file and store that data card in the memory using intermediate buffer
    //inf>>buffer;
    //inf.getline(buffer,40);
    //str_char(s);
    cout<<"*****";
    if(!IOQ.empty())
    {
        int h=IOQ.front().STDC;
        int i=0;
        int f=0;
        int z =1;
        string a1,b1;
        cout<<IR;
        a1=IR[2];
        b1=IR[3];

        try
        {
            int a= stoi(a1);
            int b = stoi(b1);
        }
        catch (const std::invalid_argument& ia)
        {
            cout<<"*c";
            z=0;
            PI = 2;
            MOS('p');
        }

        if (z)
        {
            int sa = Random_Gen(frames);
            cout<<"sa "<<sa << endl;
            int q=0;
            int m = sa*10;
            int ptr = IOQ.front().ptr;

            while(M[ptr*10+q][0]!='#')
            {
                q++;
            }
                    //cout<<"\nQ:"<<ptr*10+q<<endl;
            M[ptr*10+q][0]='0';
            M[ptr*10+q][1]='0';
            M[ptr*10+q][2]= (char)((sa/10)+48);
            M[ptr*10+q][3]= (char)((sa%10)+48);
            //cout<<a<<b<<endl;

            //cout<<m<< i<<" ";
            while(h <= IOQ.front().ETDC)
            {
                for(i =0;i<4; i++)
                {
                    // cout<<m<< i<<" ";
                    if (HD[h][i]=='\0')
                    {
                        //cout<<"a\n ----------"<<h;

                        IOQ.front().STDC = h+1;
                        f=1;
                        break;
                    }
                    M[m][i]=HD[h][i];
                }
                if (f==1)
                    break;
                m++;
                h++;
                i=0;
            }
        }

        IOQ.front().Dcount--;
        RDQ.push(IOQ.front());
        cout<<"rdq: "<<RDQ.size();
        IOQ.pop();
        cout<<"IOQ: "<<IOQ.size();

    }

    /*if (buffer[0]=='$' && buffer[1]=='E')
    {
        z=0;
        MOS('o');
    }*/
    TASK = "";

}

//---------------------------------------------------------------------------------------------------------------------------

void Write()
{                       //function to write output of job into output file using buffer
    cout<<"W";
    int h=0;
    int i=0;
    int f=0;
    int k=0;
    int z = 1;
    int holder;
    int y,a,b;
    string a1,b1;
    a1=IR[2];
    b1=IR[3];
    try
    {
        a= stoi(a1);
        b = stoi(b1);
    }
    catch (const std::invalid_argument& ia)
    {
        cout<<"*c";
        z=0;
        PI = 2;
        MOS('p');
    }
    if (z)
    {
        cout<<a<<b<<endl;
        VA=a*10;
        int h1 = IOQ.front().ptr * 10 + VA/10;
        h = ((((int)M[h1][2] - 48)*10 + (int)M[h1][3] - 48 )*10)+ VA%10;
        if (h<0 || h>299)
        {
            z=0;
            PI = 3;
            MOS('p');

        }
        else
        {
            holder = h;
            //clear_buf();
            //
            for (int x = IOQ.front().STPC;x<=IOQ.front().ETDC;x++)
            {
                for(int xx = 0;xx<4;xx++)
                {
                    HD[x][xx]='\0';
                }
            }
            head = IOQ.front().STPC;
            IOQ.front().STDC = head;
            while(h<(holder+10))
                    {
                        for(i =0;i<4; i++)
                        {
                           // cout<<m<< i<<" ";
                            if (M[h][i]=='\0')
                            {
                                //cout<<h;
                                y=h+1;
                                if (M[y][0]=='\0')
                                {
                                    //cout<<"***********"<<M[y][0]<<"**************"<<y<<" ";
                                    f=1;
                                    break;
                                }
                                else
                                {
                                    M[h][i]=' ';
                                }                                                         //writing memory content into a buffer
                            }

                            HD[head][i]=M[h][i];
                           // cout<<M[(a*10)+b+h][i]<<buffer[(4*h)+i]<<" ";
                           // cout<<m<< i<<" ";

                        }

                        h++;
                        i=0;
                        head++;
                        if (f==1)
                            break;

                }
        }

        //cout<<buffer;           // writing complete buffer content into output file
        //outf<<buffer<<"\n";
        IOQ.front().ETDC = head -1;
        IOQ.front().show();
        RDQ.push(IOQ.front());
        IOQ.pop();
    }
    TASK = "";


}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

void Terminate()
{
   outf<<"\n\n";
   //cout<<"t";
   //show_memory();
   init();
//   clear_buf();   // initialize value for next job
   //load();     //calling same function to execute consecutive job's

}
void IS()
{
    if (!IFBQ.empty())
    {
        cout << "\n performing Input Spooling\n";
        B buffer1 = IFBQ.front();
        cout<<"\nDATA : "<<buffer1.b;
        //P[j-1].show();
        if (P[j-1].flag =='P')
        {
            P[j-1].Pcount++;
            if (P[j-1].STPC == -1 )
                P[j-1].STPC= head;
        }
        else if (P[j-1].flag =='D')
        {
            P[j-1].Dcount++;
            if (P[j-1].STDC == -1 )
                P[j-1].STDC= head;
        }

        //P[j-1].show();
        int i=0;
        int h=0;
        int f=0;
        while(buffer1.b[(4*h)]!='\0')
            {
                //cout<<m<< i<<" ";
                for(i =0;i<4; i++)
                {

                    if (buffer1.b[(4*h)+i] == '\0')
                    {
                        f=1;
                        break;
                    }
                    HD[head][i]=buffer1.b[(4*h)+i];

                    cout<<buffer1.b[(4*h)+i]<<" ";

                }

                head++;
                h++;
                if (f==1)
                    break;
                i=0;
            }

            if (P[j-1].flag =='P')
                P[j-1].ETPC = head - 1;
            else if (P[j-1].flag =='D')
                P[j-1].ETDC = head - 1;
            //P[j-1].show();

            IFBQ.pop();
            clear_buf(buffer1);

            B b;
            clear_buf(b);
            EBQ.push(b);
            cout <<"\ndone IS () call\n";
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LD()
{
    if (!LQ.empty())

    {
        cout<<"\n\t\tLoding.....\n";
        int sa = Random_Gen(frames);
        cout<<"*sa "<<sa << endl;
        int ptr = LQ.front().ptr;
        int st = LQ.front().STPC;
        int q =0;

        while(M[ptr*10+q][0]!='#')
        {
            q++;
        }
            //cout<<"\nQ:"<<ptr*10+q<<endl;
        M[ptr*10+q][0]='0';
        M[ptr*10+q][1]='0';
        M[ptr*10+q][2]= (char)((sa/10)+48);
        M[ptr*10+q][3]= (char)((sa%10)+48);
        m=sa*10;
        int i=0;
        int h=0;
        int f=0;
        int g = 0;

        while(st <= LQ.front().ETPC)
        {
            //cout<<m<< i<<" ";
            for(i =0;i<4; i++)
            {
                // cout<<m<< i<<" ";
                if (HD[st][i]=='\0')
                {
                    //cout<<"a";
                    f=1;
                    break;
                }
                M[m][i]=HD[st][i];
            }
            if (f==1)
                break;
            m++;
            st++;
            i=0;
            }

        cout <<"\n\n \t Loading is done\n";
        if (--LQ.front().Pcount == 0)
        {
            RDQ.push(LQ.front());
        }
        cout<<"rdq :";
        RDQ.front().show();
        LQ.pop();

    }


}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------


void OS()
{
    if (!TQ.empty())
    {
       cout<<"\n Size Of EBQ :  "<<EBQ.size()<<"\n"<<TQ.front().STDC<<"\n"<<TQ.front().ETDC<<" END\n";
       if (!EBQ.empty())
       {
           B buf = EBQ.front();
           int g=0;
           TQ.front().show();
           for(int v = TQ.front().STDC;v<=TQ.front().ETDC;v++)
           {
               for(int vv =0;vv<4;vv++)
               {
                   buf.b[4*g+vv] = HD[v][vv];
                   cout<<HD[v][vv];
               }
               g++;
           }
           cout<<buf.b;
           EBQ.pop();
           OFBQ.push(buf);

       }
    }
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------


void IR1()
{
    /*B buf = EBQ.front();
    inf.getline(buf.b,41);
    cout<<buf.b<<endl;
    EBQ.pop();
    IFBQ.push(buf);*/
    cout <<" loading card one by one in the empty buffers.................................................\n";
    //Sleep(500);
    if(!inf.eof() && !EBQ.empty())
    {

        B buf = EBQ.front();
        clear_buf(buf);
        cout<<"\n Filling empty buffer and making it as Input full buffer \n";
        inf.getline(buf.b,40);
        //cout<<buf.b<<endl;
        EBQ.pop();
        IFBQ.push(buf);

        cout<<"\nSize of empty buffer q :         "<<EBQ.size();
        cout<<"\nSize of Input full buffer q :     "<<IFBQ.size()<<endl;
        //cout<<IFBQ.front().b<<endl;
        StartCH(1);

    if (!IFBQ.empty())
    {
        B buffer = IFBQ.front();


        if(buf.b[0]=='$' && buf.b[1]=='A' && buf.b[2]=='M' && buf.b[3]=='J')
        {
            cout<<"\n\n buffer contains : "<<buf.b<<endl<< "\n*** Initializing PCB ***\n";
            //loading();
            j = ((int)buf.b[6] -48)*10 +(int)buf.b[7] -48;
            if (j==1)
                init();
            int t = ((int)buf.b[10] -48)*10 +(int)buf.b[11] -48;
            int l = ((int)buf.b[14] -48)*10 +(int)buf.b[15] -48;
            int ptr= Random_Gen(frames);

            P[j-1].init(j,t,l,'P',ptr);
            //P.TTL = t;
            //P.TLL = l;
            cout<<"\n    PCB \n ----------\n";
            P[j-1].show();
             cout<<"\n";
            // Sleep(1000);
            /*PTR[0]='0';
            PTR[1]='0';

            PTR[2]=(char)((ptr/10)+48);
            PTR[3]= (char)((ptr%10)+48);*/

            init_PT(ptr*10);

            IFBQ.pop();
            B b;
            clear_buf(b);
            EBQ.push(b);

            /*for(int x =0;x<4; x++)
            {

                 cout<<"ptr "<<x<< "  "<<PTR[x]<<endl;
            }
            cout<<"i";*/

        }
        else if(buf.b[0]=='$' && buf.b[1]=='D' && buf.b[2]=='T' && buf.b[3]=='A')
        {
            cout<<"\nn buffer contains"<<buf.b<<endl<< "\n *** Updating PCB ***";

            P[j-1].flag = 'D';
            IFBQ.pop();
            B b;
            clear_buf(b);
            EBQ.push(b);
            //Sleep(500);
        }
        else if(buf.b[0]=='$' && buf.b[1]=='E' && buf.b[2]=='N' && buf.b[3]=='D')
        {
            cout<<"\n"<<buf.b<<endl;

            LQ.push(P[j-1]);
            cout<<"\n *** Moving PCB in Load Queue ***\n";
            //loading();
            cout<<"\n    PCB : \n";
            LQ.front().show();
            //Sleep(1000);
            TASK = "LD";
            IFBQ.pop();
            B b;
            clear_buf(b);
            EBQ.push(b);
        }
        else
        {
            //P.flag='D';
            cout <<"\n\n"<<"Program card or Data Card :  ";
            cout<<buf.b<<endl;

            //cout<<"\nstatting CH3\n";
            TASK = "IS";
            StartCH(3);

        }

    }

    }

}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------


void IR3(string task)
{
    cout<<"Task is : "<<task<<endl;
    if(task == "IS")
    {
        IS();
        TASK = "";
    }
    else if (task == "LD")
    {
        LD();
        TASK ="";
    }
    else if (task == "READ")
    {
        Read();
    }
    else if (task == "WRT")
    {
        Write();
    }
    else if (task == "OS")
    {
        OS();
        TASK = "";
    }
    //IS();
    //LD();
    if (!RDQ.empty()&& task == "")
    {
        cout<<"\nnow in ready q **\n";
        rj = RDQ.front().job_id;
        start_exe();
       // RDQ.pop();
    }



    StartCH(3);

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------


void MOS( char c)
{

    if (c == 's')
    {
        switch(SI)
        {
            case 1:
                //Read();
                IOQ.push(RDQ.front());
                RDQ.pop();
                cout<<"rdqm: "<<RDQ.size();
                TASK = "READ";
                StartCH(3);
                simulation();
                simulation();
                cout<<"s";
                MOS('i');
                //cout<<"@@@@";
                cout<<"1s";
                break;
            case 2:
                IOQ.push(RDQ.front());
                RDQ.pop();
                TASK = "WRT";
                StartCH(3);
                simulation();
                simulation();
                MOS('i');
                break;
            case 3:
                TQ.push(RDQ.front());
                RDQ.pop();
                cout<<"RDQ->>TQ";
                //TASK = "OS";
                //Terminate();
                break;
            default :
                break;
        }
    }//---------------------------------------------------------------------------------------------------------------------------
    else if (c == 'p')
    {
        switch(PI)
        {
            case 1:
                cout<<"PE>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
                outf<<"ERROR  : Operation Error"<<"\n";
                outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
                cout<<"opp\n";
                //cout<<buffer;
                //while(inf.eof() == 0)
                /*while (buffer[1]!='E')
                {
                    inf.getline(buffer,40);
                    cout<<buffer;
                    //if (buffer[1]=='E')
                        //break;
                }*/

                SI=3;
                MOS('s');

                break;
            case 2:
                cout<<"0PDE>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
                outf<<"ERROR  : Operand Error"<<"\n";
                outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
                cout<<"opD\n";
                //cout<<buffer;
                //while(inf.eof() == 0)
                while (buffer[1]!='E')
                {
                    inf.getline(buffer,40);
                    cout<<buffer;
                    //if (buffer[1]=='E')
                        //break;
                }

                SI=3;
                MOS('s');
                break;
            case 3:
                cout<<"pf>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
                outf<<"ERROR  : Invalid Page Fault"<<"\n";
                outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
                cout<<"pF\n";
                //cout<<buffer;
                //while(inf.eof() == 0)
                while (buffer[1]!='E')
                {
                    inf.getline(buffer,40);
                    cout<<buffer;
                    //if (buffer[1]=='E')
                        //break;
                }

                SI=3;
                MOS('s');
                break;
            default :
                break;
        }
    }//---------------------------------------------------------------------------------------------------------------------------
    else if (c == 't')
    {
        cout<<"TTE>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
        outf<<"ERROR  : Time Limit Exceeded"<<"\n";
        outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
        cout<<"ott\n";
        cout<<buffer;
        //while(inf.eof() == 0)
        while (buffer[1]!='E')
        {
            inf.getline(buffer,40);
            cout<<buffer;
            //if (buffer[1]=='E')
                //break;
        }

        SI=3;
        MOS('s');
    }//---------------------------------------------------------------------------------------------------------------------------
    else if (c == 'o')
    {
        cout<<"DOE>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
                outf<<"ERROR  : Out of Data Error"<<"\n";
                outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
                cout<<"oDE\n";
                //cout<<buffer;
                //while(inf.eof() == 0)
                while (buffer[1]!='E')
                {
                    inf.getline(buffer,40);
                    cout<<buffer;
                    //if (buffer[1]=='E')
                        //break;
                }

                SI=3;
                MOS('s');
    }//---------------------------------------------------------------------------------------------------------------------------
    else if (c == 'l')
    {
        cout<<"TLE>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
        outf<<"ERROR  : Line Limit Exceeded"<<"\n";
        outf<<"STATUS : Program( job id ="<<P[j-1].job_id<<" ) "<<" Terminated Abnormaly.........................";
        cout<<"oll\n";
        //cout<<buffer;
        //while(inf.eof() == 0)
        while (buffer[1]!='E')
        {
            inf.getline(buffer,40);
            cout<<buffer;
            //if (buffer[1]=='E')
                //break;
        }

        SI=3;
        MOS('s');
    }//---------------------------------------------------------------------------------------------------------------------------
    else if (c == 'i')
    {
        switch(IOI)
        {
            case 1:
                cout <<"\nStarting IR1 Routine ....................................................\n\n";
                IR1();
                //cout<<"@@@@";
                break;
            case 2:
                //IR2()
                break;
            case 3:
                cout <<"\nStarting IR1 Routine ....................................................\n\n";
                IR1();
                //IR2();
                break;
            case 4:
                cout<<"\n  Calling IR3 Routine ....................................................\n\n";
                IR3(TASK);
                break;
            case 5:
                IR1();
                IR3(TASK);
                //cout<<"@@@@";
                break;
            case 6:
                //IR2();
                IR3(TASK);
                break;
            case 7:
                IR1();
                //IR2();
                IR3(TASK);
                break;
            default :
                break;
        }
    }

    return;

}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

void exe_user_pro()
{
    IC = 0;
    VA = IC;
    int z = 1;

    int m1 = AddM(VA);
    /*for(int x =0;x<4; x++)
    {
        cout<<"ptr "<<x<< "  "<<PTR[x]<<endl;
    }*/
     int ia= m1;

    while(M[m1][0]!='\0')
    {

        //cout<<IC<<"  "<<M[IC][0];
        for(int j =0;j<4; j++)
        {
            IR[j]=M[m1][j];             //loading IR with Program Card instruction.
            //if (M[IC][j]=='H')
                //break;
        }
        for(int j =0;j<4; j++)
            cout<<"\nIR:"<<IR[j]<<"    ";
        //cout<<"->";
        IC++;

        m1++;
                       // incrementing IC after each instruction get stored in IR
        //---------------------------------------------------------------------------------------------------------------------------


        if (IR[0]=='L' && IR[1]=='R')
        {
            RDQ.front().TTC++;
            if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
            string a1,b1;           // LR instruction implementation
            a1=IR[2];
            b1=IR[3];
            int a,b;
            try
            {
                a= stoi(a1);
                b = stoi(b1);
            }
            catch (const std::invalid_argument& ia)
            {
                cout<<"*c";
                z=0;
                PI = 2;
                MOS('p');
            }
            if (z)            // loading general purpose register R according to instruction.
            {
                VA=a*10+b;
                int l= AddM(VA);
                //cout<<"|"<<a<<b<<"|";
                for(int j =0;j<4; j++)
                {
                    R[j]=M[l][j];
                }
                for(int j =0;j<4; j++)
                    cout<<R[j];

            }
            cout<<"\n LR : " << R;

        }//---------------------------------------------------------------------------------------------------------------------------
        else if(IR[0]=='S' && IR[1]=='R')
        {
            RDQ.front().TTC+=2;
            if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
            string a1,b1;           // SR instruction implementation
            a1=IR[2];
            b1=IR[3];
            int a,b;
            try
            {
                a= stoi(a1);
                b = stoi(b1);
            }
            catch (const std::invalid_argument& ia)
            {
                cout<<"*c";
                z=0;
                PI = 2;
                MOS('p');
            }
            if (z)
            {
                VA=a*10+b;
                int l= AddM(VA);
                //cout<<"|"<<a<<b<<"|";
                for(int j =0;j<4; j++)
                {
                    M[l][j]=R[j];
                }
                for(int j =0;j<4; j++)
                    cout<<M[l][j];
            }

            cout<<"\n SR : " << R;

        }//---------------------------------------------------------------------------------------------------------------------------
        else if(IR[0]=='C' && IR[1]=='R')
        {
            RDQ.front().TTC++;
            if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
            string a1,b1;           // CR instruction implementation
            a1=IR[2];
            b1=IR[3];
            int a,b;
            try
            {
                a= stoi(a1);
                b = stoi(b1);
            }
            catch (const std::invalid_argument& ia)
            {
                cout<<"*c";
                z=0;
                PI = 2;
                MOS('p');
            }
            if (z)
            {
                VA=a*10+b;
                int l= AddM(VA);
                //cout<<"|"<<a<<b<<"|";
                for(int j =0;j<4; j++)
                {
                    if (M[l+b][j]!=R[j])
                    {
                        c=0;
                        break;
                    }
                    else
                        c=1;
            }
            }
        }//---------------------------------------------------------------------------------------------------------------------------
        else if(IR[0]=='B' && IR[1]=='T')
        {
            RDQ.front().TTC++;
            if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
            int a,b;
            if (c==1)               // BT instruction implementation
            {
                string a1,b1;
                a1=IR[2];
                b1=IR[3];
                try
                {
                    a= stoi(a1);
                    b = stoi(b1);
                }
                catch (const std::invalid_argument& ia)
                {
                    cout<<"*c";
                    z=0;
                    PI = 2;
                    MOS('p');
                }
                if (z)
                {
                    IC=a*10+b;
                    exe_user_pro();
                    break;
                }

            }
        }//---------------------------------------------------------------------------------------------------------------------------
        else if (IR[0]=='G' && IR[1]=='D')
        {
           RDQ.front().TTC+=2;
           cout<<RDQ.front().TTC<<"   "<<RDQ.front().TTL;
            if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
           SI=1;
           MOS('s');
           //cout<<"####";
           //break;
        }//---------------------------------------------------------------------------------------------------------------------------
        else if (IR[0]=='P' && IR[1]=='D')
        {
           RDQ.front().TTC++;
           RDQ.front().TLC++;
           RDQ.front().show();
           cout<<RDQ.front().TTC<<"   "<<RDQ.front().TTL;
            /*if (RDQ.front().TTC > RDQ.front().TTL)
            {
                MOS('t');
                break;
            }
            else if (RDQ.front().TLC > RDQ.front().TLL)
            {
                MOS('l');
                break;
            }*/
           SI=2;
           MOS('s');
           //break;
        }//---------------------------------------------------------------------------------------------------------------------------
        else if (IR[0]=='H' && IR[1]=='\0')
        {
           RDQ.front().TTC++;
            /*if (RDQ.front().TTC > RDQ.front().TTL + 1)
            {
                MOS('t');
                break;
            }*/
           cout<<"in t";
           SI=3;
           MOS('s');
           //break;
        }
        else
        {
            cout<<"*e";
            PI = 1;
            MOS('p');
        }

        if (m1 == ia+10 && M[m1-1][3]!='\0' && M[m1-1][0]!='H')
        {
            cout<<"IC :"<<IC;
            exe_user_pro();

            break;


        }

    }

}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------


void start_exe()
{
    cout<<"\nExe Started..........\n";
    exe_user_pro();

}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
/*void load()
{
    //string s;
    while(inf.eof() == 0)
    {
        //inf>>buffer;
        //string s;
        inf.getline(buffer,41);
        //str_char(s);

        //cout<<buffer;
        /*cout<<"buffer line:";
        for(int i =0;buffer[i]!='\0';i++)
        cout<<buffer[i];
        cout<<'\n';
        if(buffer[0]=='$' && buffer[1]=='A' && buffer[2]=='M' && buffer[3]=='J')
        {

            int j = ((int)buffer[6] -48)*10 +(int)buffer[7] -48;
            if (j==1)
                init();
            int t = ((int)buffer[10] -48)*10 +(int)buffer[11] -48;
            int l = ((int)buffer[14] -48)*10 +(int)buffer[15] -48;

            P.init(j,t,l,'s',1);
            P.TTL = t;
            P.TLL = l;
            P.show();
            PTR[0]='0';
            PTR[1]='0';
            int ptr= Random_Gen(frames);
            P.
            PTR[2]=(char)((ptr/10)+48);
            PTR[3]= (char)((ptr%10)+48);

            init_PT(ptr*10);

            for(int x =0;x<4; x++)
            {

                 cout<<"ptr "<<x<< "  "<<PTR[x]<<endl;
            }
            cout<<"i";

        }
        else if(buffer[0]=='$' && buffer[1]=='D' && buffer[2]=='T' && buffer[3]=='A')
        {
            start_exe();
        }
        else if(buffer[0]=='$' && buffer[1]=='E' && buffer[2]=='N' && buffer[3]=='D')
        {
            cout<<"bk";
            continue;
        }
        else
        {
            int sa = Random_Gen(frames);
            cout<<"*sa "<<sa << endl;
            int q=0;
            int ptr = ((int)PTR[2] - 48)*10 + (int)PTR[3] - 48;

            while(M[ptr*10+q][0]!='#')
            {
                q++;
            }
            //cout<<"\nQ:"<<ptr*10+q<<endl;
            M[ptr*10+q][0]='0';
            M[ptr*10+q][1]='0';
            M[ptr*10+q][2]= (char)((sa/10)+48);
            M[ptr*10+q][3]= (char)((sa%10)+48);

            m=sa*10;
            int i=0;
            int h=0;
            int f=0;
            int g = 0;
            while(buffer[(4*h)+i-g]!='\0')
            {
                //cout<<m<< i<<" ";
                for(i =0;i<4; i++)
                {
                   // cout<<m<< i<<" ";
                    if (buffer[(4*h)+i-g]=='\0')
                    {
                        //cout<<"a";
                        f=1;
                        break;
                    }

                    M[m][i]=buffer[(4*h)+i-g];
                    if (buffer[(4*h)+i-g]=='H' && buffer[(4*h)+i+1-g]!='\0')
                    {
                        m++;
                        h++;
                        g=3;
                        break;
                    }
                   // cout<<m<< i<<" ";
                    if (i>2)
                    {

                        m++;
                        h++;
                        //i=0;
                    }

                }
                if (f==1)
                    break;
                //if (i==4)
                i=0;


            }


        }

    }
}*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

void simulation()
{

    for (int cn=1; cn<4;cn++)
    {
        if (CHs[cn-1])
        {
            cout<<"\nchannel "<<cn<<"'s Timmer ";
            CHt[cn-1]++;
            cout<<"is "<<CHt[cn-1]<<endl;

            /*while (CHt[cn-1]!=CHTt[cn-1])
            {
                CHt[cn-1]++;
                UT++;
            }*/
            if (CHt[cn-1]==CHTt[cn-1])
            {
                if (cn<3)
                {
                    IOI +=cn;
                    cout<<"\n IOI -> becomes "<<IOI<<endl;
                }
                else
                    IOI +=4;
                CHs[cn-1]=0;
                cout<<"\n IOI becomes -> "<<IOI<<endl;
                //cout<<"s"<<IOI;
            }
        }
    }
    cout<<"\n Universal Timer : "<<++UT << ".  ................................................\n";
    //Sleep(2000);
    //MOS('i');

}



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------



void StartCH(int i)
{
    //cout<<"\n Now in Start channel function";
    if (i==1)
    {
        IOI -=1;
        CHt[i-1] = 0;
        CHs[i-1] = 1;
        cout<<"\nStarting channel 1 ...............................................................................\n"<<endl;
        //loading();
        cout<<"  IOI value -> "<<IOI;
    }
    else if (i==2)
    {
        IOI -=2;
        CHt[i-1] = 0;
        CHs[i-1] = 1;
        cout<<"\nStarting channel 2 ...............................................................................\n"<<endl;
        //loading();
        cout<<"  IOI value -> "<<IOI;
    }
    else if (i==3)
    {
        if (IOI-4>=0)
            IOI-=4;
        else
            IOI=0;
        cout<<IOI<<endl;
        CHt[i-1] = 0;
        CHs[i-1] = 1;
        cout<<"\n Starting channel 3 ...............................................................................\n"<<endl;
        //loading();
        cout<<"  IOI value -> "<<IOI;
    }
    //cout<<"\n Now in Simulation function";
    //simulation();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

int main()
{
    //int UT=0;

    //load();
    //show_memory();
    //inf.close();
    //outf.close();

    system("color 0b");

    cout<< "In Main Function : \n\n\n\n";

    int w=0;
    //cout<<EBQ.size();
    cout<<"Initializing the empty buffer q (EBQ) with 10 empty buffers........\n";
    for(int qq =0; qq<10; qq++)
    {
        B b;

        clear_buf(b);
        EBQ.push(b);
    }

    cout<<"\nSize of empty buffer q :         "<<EBQ.size();
    cout<<"\nSize of Input full buffer q :     "<<IFBQ.size()<<endl;

    cout<<"\n\n Operating System Loading, Please Wait \n\n" ;
    //loading();

    while(w<94)
    {
        //cout<<"\n"<<w<<endl;
        if (w==0)
            StartCH(1);
        simulation();
        //
        MOS('i');
        // simulation();
        w++;

    }

    cout<<"\n"<<UT<<"\n"<<CHt[0]<<"\n"<<IOI<<"\n"<<"\n"<<EBQ.size()<<"\n"<<IFBQ.size()<<"\n"<<CHs[0];
    show_HD();

    show_memory();
    cout<<"\n\n done \n";
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------Decorative-------------------------------------------------------------------------------------------------------------------------------------

void loading()
{
    char a = 177, b = 219;

	for (int i = 0; i <= 97; i++)
		cout << a;
	cout << "\r";
	for (int i = 0; i <= 97; i++)
	{
		cout << b;
		Sleep(15);

	}
}

//------------------------------------------------------------THANKS--------------------------------------------------------------------------------------------------

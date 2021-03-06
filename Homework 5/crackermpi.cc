#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<unistd.h>
#include<crypt.h>
#include<sstream>
#include<cstring>
#include<omp.h>
#include<cassert>
#include<mpi.h> 

using namespace std;
class passcracker{
    public:
        
        void get_mpi_info(){
            // Get the number of processes
            MPI_Comm_size(MPI_COMM_WORLD, &world_size);
            // Get the rank of the process
            MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
        }

        void load_vectors(int argc,char*argv[]){
        
            ifstream ins_salts, ins_diconary, ins_encrypted;
            if(argc == 1){
                ins_salts.open("given/salts");
                ins_diconary.open("given/words");
                ins_encrypted.open("given/enc_passwords");
            }else if(argc > 3){
                ins_salts.open(argv[1]);
                ins_diconary.open(argv[2]);
                ins_encrypted.open(argv[3]);
            }else{
                cout << "Please Pass the proper agruments salts,dictonary,encrypted passwords" << endl;
                assert(0);
            }

            string tmp;
            
            while (ins_salts >>tmp){
                salts.push_back(tmp);
            }
            while (ins_diconary >> tmp){
                diconary.push_back(tmp);
            }
            while (ins_encrypted >> tmp){
                encrypted.push_back(tmp);
            }

            ins_salts.close();
            ins_diconary.close();
            ins_encrypted.close();
        }

        void attack(){
            stringstream sstmp;
            vector<string> single_digit; // made to hold single digits 
            vector<string> double_digit; // made to hold double digit numbers
            vector<string> triple_digit; // mode to hold triple digit numbers
            for(int a=0; a<10; a++){
                sstmp << a;
                single_digit.push_back(sstmp.str());
                sstmp.str(string()); // clear 
            }
            for(int b=0; b<100; b++){
                if(b<10){sstmp << 0 << b;}
                else{sstmp << b;}
                double_digit.push_back(sstmp.str());
                sstmp.str(string());
            }
            for (int c=0; c<1000; c++){
                if(c < 10){sstmp << 0 << 0 << c;}
                else if (c < 100){sstmp << 0 << c;}
                else{sstmp << c;}
                triple_digit.push_back(sstmp.str());
                sstmp.str(string());   
            }

            size_t wordscreated = 0;// use wordscreated to divide the work among the proccesses.
            
            for (int i=0; i<diconary.size(); i++){ 
                if (wordscreated % world_size == world_rank){     
                    pass(diconary[i]); // word
                }wordscreated++;    

                for(int a=0; a<single_digit.size();a++){
                    if (wordscreated % world_size == world_rank){ 
                        pass(diconary[i]+single_digit[a]); // word + 1digit
                    }wordscreated++;                         
                    if (wordscreated % world_size == world_rank){  
                        pass(single_digit[a]+diconary[i]); // 1digit + word     
                    }wordscreated++;                      
                }
                for(int b=0; b<double_digit.size(); b++){                 
                    if (wordscreated % world_size == world_rank){ 
                        pass(diconary[i]+double_digit[b]); //word + 2digit
                    }wordscreated++;                         
                    if (wordscreated % world_size == world_rank){  
                        pass(double_digit[b]+diconary[i]); // 2digit + word
                    }wordscreated++;     
                }
                for(int c=0; c<triple_digit.size(); c++){
                    if (wordscreated % world_size == world_rank){ 
                        pass(diconary[i]+triple_digit[c]); // word + 3digit
                    }wordscreated++;                         
                    if (wordscreated % world_size == world_rank){ 
                        pass(triple_digit[c]+diconary[i]); // 3digit + word
                    }wordscreated++; 
                }
                    // second word cases taken out of assignment
            }
             
        }

        void make_test_cases(){
            vector<string> testpasses = {"ant123","464roach","78bug","99whale","783dog","fish67","rhino005"};
            for(int i=0; i < testpasses.size(); i++){
                if(world_rank==2)
                cout << crypt(testpasses[i].c_str(),salts[i].c_str()) << endl;
            }

        }
        

    private: 

        void pass(string generated){
            for(int i=0; i < salts.size(); i++){
                string cryptedPass = crypt(generated.c_str(),salts[i].c_str());
                if(encrypted[i] == cryptedPass){ // we found the password matched the encrypted password
                    cout <<"Rank: #" << world_rank  << " Found password #" << i <<  " it is: "<< generated << endl; 
                }
            }
        }

        // mpi information //////////////////////////////
        int world_size;
        int world_rank;
        ////////////////////////////////////////////////
        int maxPasswordLength=-1;
        vector <string> salts , diconary, encrypted;

};


int main(int argc, char** argv){
    
    MPI_Init(NULL,NULL);
    
    passcracker crack;

    crack.get_mpi_info();
    crack.load_vectors(argc,argv);
    crack.attack();

    MPI_Finalize();
}

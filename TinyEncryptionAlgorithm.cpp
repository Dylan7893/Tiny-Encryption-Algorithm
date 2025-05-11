//directives for various libraries
#include <iostream> //standard input/output streams
#include <fstream>  //reading and writing to files
#include <random>   //generating random numbers
#include <ctime>    //obtain the current time (seed random generator)
#include <vector>   //dynamic arrays that can grow in size
#include <iomanip>  //formatting output
#include <cstdint>  //fixed-width integer types

//using the standard namespace
using namespace std;

//function prototypes
void code(int32_t*, int32_t*);
void decode(int32_t*, int32_t*);


//start of program
int main()
{
    int32_t k[4]; //int32 array for key
    char cont; //user input to run program again
    string fileName; //holds file name
    ifstream inputFile; //creating a file object from the ifstream class
    vector<int32_t> block(2); //creating a vector to hold two longs
    
    //do-while loop used to run program multiple times
    do {
        int modeOfOperation = -1; //used to select mode initialized to an invalid response
        while(modeOfOperation != 1 && modeOfOperation != 2) //validation loop
        {
            cout << "Would you like to encrypt or decrypt? (1 = encrypt or 2 = decrypt): "; //asks for input
            cin >> modeOfOperation; //gets mode input
            
            if (modeOfOperation != 1 && modeOfOperation != 2) //checks for an error
                cerr << "Please enter valid input (1 = encrypt or 2 = decrypt)"; //displays error
        }

        if (modeOfOperation == 1) //if the user selects encryption
        {
            srand(static_cast<unsigned int>(time(0))); //seed random number generator with time

            for (int index = 0; index < 4; ++index) //interates through key array
                k[index] = rand(); //assigns each index a random long
    
            cout << "\nKey: "; //outputs a message indicating the following string will be the key
            for (int i = 0; i < 4; ++i) //iterates through key array
                cout << hex << setw(8) << setfill('0') << k[i] << " "; //formatting and outputting key as hexadecimal
    
            cout << dec << endl; //resetting the output stream to decimal

            bool validFile = false; //flag for opening file
            while (!validFile) //if file is not open ask for new input
            {
                cin.ignore(); //discard characters from the input buffer
                cout << "Enter the plain text file name: "; //asks for plaint text file
                getline(cin, fileName); //gets file name
                
                inputFile.open(fileName); //attempts to open the file
                
                if (!inputFile) //if file does not open
                    cerr << "Error opening file: " << fileName << endl << endl; //display error
                else
                    validFile = true; //if file does open exit loop
            }

            ofstream outputFile("encryptedText.txt"); //creatses a output file

            while(true) //reading the file and breaking at end of file
            {
                //reads 32 bits for each element from inputFile to block
                inputFile.read(reinterpret_cast<char*>(block.data()), sizeof(int32_t) * block.size());
                    
                unsigned long bytes = inputFile.gcount(); //obtaining the number of bytes (characters) in the inputFile
                
                if (bytes < sizeof(int32_t) * block.size()) //if there was less than 64 bits of input padding with null
                {
                    for(unsigned long index = bytes; index < sizeof(int32_t) * block.size(); ++index)
                        reinterpret_cast<char*>(block.data())[index] = 0; //padding with null
                }
                
                if (bytes == 0) //breaks loop at the end of the file
                    break;
        
                code(block.data(), k);//encrypt each block
                outputFile.write(reinterpret_cast<char*>(block.data()), sizeof(int32_t) * block.size()); //file output
            }

            inputFile.close(); //close input file
            outputFile.close(); //close output file
            cout << "\nEncryption is finished and the encrypted text has been saved to 'encryptedText.txt'.\n"; //console output
        }
        
        else //if the user selects decryption
        {
            cout << "Enter key: "; //asks the user for a key
            for(int index = 0; index < 4; ++index) //iterates through the key
                cin >> hex >> k[index]; //reads the key as a hexadecimal
            
            bool validFile = false; //flag for opening file
            while (!validFile) //if file is not open ask for new input
            {
                cin.ignore(); //discard characters from the input buffer
                cout << "Enter the encrypted file name: "; //asks for the encrypted file
                getline(cin, fileName); //get filename from input
                
                inputFile.open(fileName); //attempts to open the file
                
                if (!inputFile) //if file does not open
                    cerr << "Error opening file: " << fileName << endl << endl; //display error
                else
                    validFile = true; //if file does open exit loop
            }
            
            ofstream outputFile("decryptedText.txt"); //create an output file
            
            while(inputFile.read(reinterpret_cast<char*>(block.data()), sizeof(int32_t) * block.size()))//reading block of two 32bits
            {
                decode(block.data(), k); //decrypt each block
                outputFile.write(reinterpret_cast<char*>(block.data()), sizeof(int32_t) * block.size()); //file output
            }

            inputFile.close(); //close files
            outputFile.close(); //close files
            cout << "Decryption is finished and the decrypted text saved to 'decryptedText.txt'.\n"; //console output
        }

        cout << "\nWould you like to continue? (y/n): "; //asks the user if they would like to continue
        cin >> cont; //gets iser input

    } while(cont == 'y' || cont  == 'Y'); //if user wants to continue start again; else end loop

    cout << endl; //formatting
    return 0; //finished successfully
}

//function definition for code
void code(int32_t* v, int32_t* k) //parameters takes an array for a block of data and an array for a key
{
    uint32_t y = v[0], z = v[1], sum = 0, n = 32; //declare local variables
    const uint32_t delta = 0x9e3779b9; //a key schedule constant

    while (n-- > 0) //basic cycle start
    {
        //The routine relies on the alternate use of XOR and ADD to provide nonlinearity
        sum += delta;
        y += ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
        z += ((y << 4) + k[2]) ^ (y + sum) ^ ((y >> 5) + k[3]);
    }//end of cycle

    v[0] = y;
    v[1] = z;
}

//function definition for decode
void decode(int32_t* v, int32_t* k) //parameters takes an array for a block of data and an array for a key
{
    uint32_t n = 32, sum, y = v[0], z = v[1]; //declare local variables
    const uint32_t delta = 0x9e3779b9; //a key schedule constant
    sum = delta << 5; //performs bitwise left shift on delta by 5 positions and assigns sum

    while (n-- > 0) //basic cycle start
    {
        //The routine relies on the alternate use of XOR and ADD to provide nonlinearity
        z -= ((y << 4) + k[2]) ^ (y + sum) ^ ((y >> 5) + k[3]);
        y -= ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
        sum -= delta;
    }//end of cycle

    v[0] = y;
    v[1] = z;
}

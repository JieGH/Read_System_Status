// 
// 12/2018 Newcastle
// This C++ program build in g++ (GCC) 7.3.0, in Cygwin64, with C++14
//v0.27

#include <iostream>     // for basic input/output library
#include <sstream>      // istringstream
#include <fstream>      // for read file
#include <iomanip>      // for set precision
#include <unistd.h>     // for sleep

using namespace std;

int num_of_cpu = 0; // number of CPU
int num_words = 0; // number of words in the 'stat' file
int cnt = 0; // to avoid calculate first row of data in "stat", see member function 'get_cpu_ratios_ss'
int sample_count = 0; // count how many times the program had run

ifstream fp;    // for read file
string word, filename; // declare two string for read file
string str; // declare str for istringstream

class Information {
    // the variable inside the class member function could be private
    // for member function 'get_page_swap_ratio'
    string page_swap_name;
    unsigned long long in;
    unsigned long long out;
    // for member function 'get_intr_ctxt_num'
    string intr_ctxt_name;
    unsigned long long value;
    // for member function 'get_cpu_ratios_ss'
    string cpu_name;
    unsigned long long busy;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
public:
    // these two variable need to be public because they will be returned to 'main'
    double page_swap_ratio;
    double intr_ctxt_num;
    // declare class member function
    double get_page_swap_ratio(string str);
    double get_intr_ctxt_num(string str);
    void get_cpu_ratios_ss(string str);
};
// This function use istringstream
// to calculate the page, swap in/out ratio value then return it
double Information::get_page_swap_ratio(string str) {
    istringstream iss(str);
    iss >> page_swap_name >> in >> out;
    // this 'if' make sure the display of ratio is user friendly
    if (in && out) {
        // '(double)' make sure the ratio is reasonable otherwise ratio will not be 'double type'
        page_swap_ratio = (double) in / out;
    } else
        // this does not mean page_swap_ratio is equal to zero,
        // further action will be made in 'main', see line xxx for detail
        page_swap_ratio = 0;
    return page_swap_ratio;
}
// similer with 'get_page_swap_ratio', useing istringstream to get interrupts and context switch in billions
double Information::get_intr_ctxt_num(string str) {
    istringstream iss(str);
    iss >> intr_ctxt_name >> value;
    // to get value in unit 'billions'
    intr_ctxt_num = (double) value / 1000000000;
    return intr_ctxt_num;
}
// This function use istringstream
// to calculate the busy, nice, system, idle usage precentage then print out all the results.
void Information::get_cpu_ratios_ss(string str) {

    // the condition inside the "if" is to avoid calculate first row of the CPU data
    if (cnt != 0) {
        // Using "istringstream" to fetch the number in certain structure and calculate the data
        istringstream iss(str);
        iss >> cpu_name >> busy >> nice >> system >> idle;
        cout << "CPU" << cnt - 1 << "\t";
        // calculate all the persentage and print them out along with the name of CPU
        // make sure the results are "double".
        cout << (double) busy / (busy + nice + system + idle) * 100 << "%" << "\t";
        cout << (double) nice / (busy + nice + system + idle) * 100 << "%" << "\t";
        cout << (double) system / (busy + nice + system + idle) * 100 << "%" << "\t";
        cout << (double) idle / (busy + nice + system + idle) * 100 << "%" << "\n";
    }
    cnt++; // to avoid calculate first row of data in "stat"
}

// This function could obtained the number of CPUs that used in the machine.
// This function will compare all the elements from "string array[]" with string "cpu0, cpu1, cpu2...."
// The variable "num_of_cpu" will increment every time when the strings matched
void get_num_cpu(string array[]) {
    int t = 1;
    for (int i = 0; i < t; ++i) {
        // create a string str1 obtained "cpu"
        string str1("cpu");
        // add the number i behind the third letter which is behind "u"
        string cpu_num(to_string(i));
        str1.insert(3, cpu_num);
        // compare this newly created string " cpu'i' " with every elements in string array[]
        // the comparison will initiate "num_words * 2" times, based on the number of elements in the "stat" file
        for (int j = 0; j < num_words * 2; ++j) {
            string str2(array[j]);
            if (str2.compare(str1) == 0) {
                // The variable "num_of_cpu" will increment every time when the strings matched
                num_of_cpu++;
                ++t;
            }
        }
    }
}

// Pre-read the file to find out the number of words in the "stat" file
// num_words will help sercond_pre_read function to decide the size of buf_array array
void first_pre_read() {
    filename = "/proc/stat";
    fp.open(filename.c_str());
    // make sure the file had been open, otherwise abort
    if (!fp.is_open()) {
        cerr << "Cannot open the file ... exiting... " << endl;
        exit(EXIT_FAILURE);
    }
    // Count how many words or elements in this file
    // to help second_pre_read function to decide the size of dynamic array buf_array
    while (fp >> word) {
        ++num_words;
    }
    fp.close();
    cout << "Number of words in this file is: " << num_words << " and Pre-read finished." << endl;
}

// second_pre_read function create a dynamic array then calling get_num_cpu function,
// to calculate the number of CPU of the machine
void second_pre_read() {
    int i = 0;
    filename = "/proc/stat";
    // Create a dynamic array to store all the words fetched from the "stat" file to calculate the number of CPUs.
    string *buf_array = new string[num_words * 2];
    fp.open(filename.c_str());
    // make sure the file had been open, otherwise abort.
    if (!fp.is_open()) {
        cerr << "Cannot open the file ... exiting... " << endl;
        exit(EXIT_FAILURE);
    }
    // To store all the words or elements of "stat" file to the dynamic array "buf_array".
    // Add space ("\n") between each elements.
    while (fp >> word) {
        buf_array[i] = word;
        buf_array[++i] = "\n";
        ++i;
    }
    fp.close();
    // Calling "get_num_cpu" function to calculate the number of CPUs of this machine.
    get_num_cpu(buf_array);
    cout << "Number of CPU is: " << num_of_cpu << endl;
    // Dynamic array "buf_array" only been used in number of CPUs calculation,
    // thus this array will be delete after the program obtained the "num_of_cpu"
    delete[] buf_array;
}

int main() {
    // To obtain the number of elements in the "stat" file.
    first_pre_read();
    // To obtain the number of CPUs used in this machine.
    // The number of CPU generally will not change during ever boot of machine
    // It is waste computation power to calculate number of cpu in multiple times.
    second_pre_read();
    // "first_pre_read" and "second_pre_read" are only been executed onece each.

    // The "stat" file will be read again in every 0.5 second in a 'while' loop
    // to obtain the system usage information and display it in certain layout
    while (1) {
        // cout command to print everthing in the same place withput scrolling the entire screen.
        cout << "\e[1;1H\e[2J";
        // To indicate how many times the program is running, number of Cores and the structure of the table.
        cout << "++++ Mr Jie Lei Student ID: 160522787 @Newcastle University   12/2018 ++++\n" << endl;
        cout << "Number "<< sample_count << " times running..." << endl;
        cout << "Running times: "<< sample_count/2 << " seconds\n" << endl;
        cout << "BEGIN" << endl;
        cout << "-------------------------------------------" << endl;
        cout << "CPU Cores: " << num_of_cpu << endl;
        cout << "-------------------------------------------" << endl;
        cout << "CPU\tbusy\tnice\tsystem\t idle" << endl;
        cout << "===========================================" << endl;
        // read file again, comments see "first_pre_read" and "second_pre_read"
        filename = "/proc/stat";
        fp.open(filename.c_str());
        if (!fp.is_open()) {
            cerr << "Cannot open the file ... exiting... " << endl;
            exit(EXIT_FAILURE);
        }
        // Based on the "num_of_cpu" obtained form "second_pre_read",
        // this for loop will fetch the first few lines contained CPU information,
        // and calculate the busy, nice, system, idle usage precentage then print out all the results.

        // declare class 'information' as 'stat'
        Information stat;
        // To set the precision of the display which is one digit after the decimal point
        cout << setiosflags(ios::left | ios::fixed);
        cout.precision(1);
        // to obtain, calculate and display cpu usage information based on the number of CPUs
        for (int j = 0; j <= num_of_cpu; ++j) {
            // read a line and store the information as string 'str'
            getline(fp, str);
            // calling class member function to get the ratios
            stat.get_cpu_ratios_ss(str);
        }
        cnt = 0; // reset the "cnt" inside "get_cpu_ratios_ss" for avoid read first row of the "stat" file
        cout << "---------------------------------------------------" << endl;

        // To set the precision of the display which is six digits after the decimal point
        cout << setiosflags(ios::left | ios::fixed);
        cout.precision(6);
        getline(fp, str); // read page info line as "str".
        // if Page out equal to zero print following message
        if (stat.get_page_swap_ratio(str) == 0)
            cout << "Page in/out Ratio is infinity because Page out = 0. " << endl;
        else
            cout << "Page in/out Ratio is: " << stat.get_page_swap_ratio(str) << endl;

        getline(fp, str); // read swap info line as "str".
        // if Swap out equal to zero print following message
        if (stat.get_page_swap_ratio(str) == 0)
            cout << "Swap in/out Ratio is infinity because Swap out = 0. " << endl;
        else
            cout << "Swap in/out Ratio is: " << stat.get_page_swap_ratio(str) << endl;
        // To set the precision of the display which is two digits after the decimal point
        cout << setiosflags(ios::left | ios::fixed);
        cout.precision(2);
        getline(fp, str); // read intr info line as "str".
        cout << "Interrupts serviced: " << stat.get_intr_ctxt_num(str) << " billions since booting" << endl;

        getline(fp, str); // read ctxt info line as "str".
        cout << "Context switch counts: " << stat.get_intr_ctxt_num(str) << " billions since booting" << endl;
        cout << "---------------------------------------------------" << endl;
        cout << "END" << endl;

        fp.close(); // close the file.
        usleep(500000); //this will ensure 0.5 second time delay
        sample_count++; //indicate how many times this while loop running.
    }
}
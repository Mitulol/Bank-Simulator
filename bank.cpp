// Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

#include <iostream>
#include <getopt.h>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <unordered_set>
#include <queue>

using namespace std;

class Bank{

    private:

        struct Transaction {
            uint32_t transaction_id;
            uint64_t placement_time;
            string sender_id;
            string recepient_id;
            uint32_t amount;
            uint64_t execution_time;
            uint32_t fee;
            uint32_t fee_sender;
            uint32_t fee_recipient;
            bool shared;
        };

        struct User {
            uint64_t registration_time;
            string user_id;
            uint32_t pin;
            uint32_t balance;
            unordered_set<string> valid_IPs = {};
            vector<Transaction> outgoing_transactions = {};
            vector<Transaction> incoming_transactions = {};
        };

        unordered_map<string, User> registered_users;
        vector<Transaction> transaction_master_list;
        string filename;
        uint64_t last_place_time = 0;
        uint32_t transaction_count = 0;
        bool verbose = false;
        
        //uint64_t last_valid_place_time = 0;
    
    public:

        void get_options(int argc, char** argv) {

            int option_index = 0;
            int option;

            opterr = false;

            struct option longOpts[] = {
                        { "file", required_argument, nullptr, 'f'},
                        { "verbose", no_argument, nullptr, 'v'},
                        { "help", no_argument, nullptr, 'h'},
                        { nullptr, 0, nullptr, '\0'}
                    };

            while ((option = getopt_long(argc, argv, "f:vh", longOpts,&option_index)) != -1) {

                switch (option) {

                    case 'v':
                        verbose = true;
                        break;

                    case 'f':
                        filename = optarg;
                        break;

                    case 'h':
                        std::cout << "a helpful message explaining how to use this program" << std::endl;
                        exit(0);

                    default:
                        cerr << "Inavalid command line option\n";
                        break; 
                }
            }
        }

        void read_users() {

            string line;

            ifstream inputFile(filename);
            //inputFile.open(filename);
            if (!inputFile) {
                cerr << "Registration file failed to open.\n";
                exit(1);
            }
            uint64_t input_timestamp;
            string input_name;
            uint32_t input_pin;
            uint32_t input_balance;

            while (getline(inputFile, line)) {

                input_timestamp = convert_timestamp(line.substr(0, 17));
                //cout << input_timestamp << endl;

                size_t index = line.find('|', 18);
                //cout << index << endl;

                input_name = line.substr(18, index - 18);
                //cout << input_name << endl;

                //cout << "I am about to use stoi()" << endl;

                input_pin = stoi(line.substr(index + 1, 6));
                //cout << input_pin << endl;

                input_balance = stoi(line.substr(index+8));
                //cout << input_balance << endl;

                registered_users[input_name] = User{input_timestamp, input_name, input_pin, input_balance};

            }

            inputFile.close();
        }

        uint64_t convert_timestamp(string timestamp_in) {

            //cout << "I am about to use convert_timestamp" << endl;
            
            uint64_t timestamp_out = 0;

            for (size_t i = 0; i < 16; i += 3) {
                timestamp_out = (timestamp_out * 100ULL) + stoi(timestamp_in.substr(i, 2));
            }

            return timestamp_out;
        }

        void handle_operations() {

            char first_letter;
            cin >> first_letter;

            while (first_letter) {

                if (first_letter == '#') comment();

                else if (first_letter == 'l') login();

                else if (first_letter == 'o') out();

                else if (first_letter == 'p') place();

                else if (first_letter == '$') break;

                else cout << "Invalid operation.\n";

                cin >> first_letter;
            }

            dollar_sign();

        }

        void comment() {

            string comment;
            getline(cin, comment);
        }

        void login() {

            string junk;
            cin >> junk; //this will be "ogin"

            string user_id_in;
            cin >> user_id_in;

            uint32_t user_pin_in;
            cin >> user_pin_in;

            string IP_in;
            cin >> IP_in;

            auto iter = registered_users.find(user_id_in);
            if (iter == registered_users.end()) {
                if (verbose) {
                    cout << "Failed to log in " << user_id_in << ".\n";
                }
                return;
            }
            else {
                if (iter->second.pin == user_pin_in) {
                    iter->second.valid_IPs.insert(IP_in);
                    if (verbose){
                        cout << "User " << user_id_in << " logged in.\n";
                    }
                }
                else {
                    if (verbose) {
                        cout << "Failed to log in " << user_id_in << ".\n";
                    }
                }
            }
        }

        void out() {

            string junk;
            cin >> junk; //this will be "ut"

            string user_id_in;
            cin >> user_id_in;

            string IP_in;
            cin >> IP_in;

            auto iter = registered_users.find(user_id_in);

            //check for user exists
            if (iter == registered_users.end()) {
                if (verbose) {
                    cout << "Failed to log out " << user_id_in << ".\n";
                }
                return;
            }

            //user does exist
            else {
                auto iter2 = iter->second.valid_IPs.find(IP_in);

                //entered IP does not exist
                if (iter2 == iter->second.valid_IPs.end()) {
                    if (verbose) {
                        cout << "Failed to log out " << user_id_in << ".\n";
                    }
                    return;
                }

                //entered IP does exist
                else {
                    //cout << "Size of validIP set before erase: " << iter->second.valid_IPs.size() << endl;
                    iter->second.valid_IPs.erase(iter2);
                    //cout << "Size of validIP set after erase: " << iter->second.valid_IPs.size() << endl;
                    if (verbose) {
                        cout << "User " <<user_id_in << " logged out.\n";
                    }
                }
            }
        }

        void dollar_sign() {
            
            string junk;
            getline(cin, junk); //this will be "$$"

            while (!pending_transactions.empty()) {

                auto iter_to_sender = registered_users.find(pending_transactions.top().sender_id);
                auto iter_to_recipient = registered_users.find(pending_transactions.top().recepient_id);

                if (iter_to_sender -> second.balance >= pending_transactions.top().amount + pending_transactions.top().fee_sender 
                    && iter_to_recipient->second.balance >= pending_transactions.top().fee_recipient) {

                    // registered_users[pending_transactions.top().sender_id].balance -= (pending_transactions.top().amount) + pending_transactions.top().fee_sender;
                    // registered_users[pending_transactions.top().recepient_id].balance -= (pending_transactions.top().fee_recipient);
                    // registered_users[pending_transactions.top().recepient_id].balance += pending_transactions.top().amount;

                    iter_to_sender->second.balance -= pending_transactions.top().amount + pending_transactions.top().fee_sender;
                    iter_to_recipient->second.balance -= pending_transactions.top().fee_recipient;
                    iter_to_recipient->second.balance += pending_transactions.top().amount;

                    transaction_master_list.push_back(pending_transactions.top());
                    iter_to_sender -> second.outgoing_transactions.push_back(pending_transactions.top());
                    iter_to_recipient -> second.incoming_transactions.push_back(pending_transactions.top());

                    if (verbose)
                        cout << "Transaction executed at " << pending_transactions.top().execution_time << ": $" << pending_transactions.top().amount << " from "
                             << pending_transactions.top().sender_id << " to " << pending_transactions.top().recepient_id << ".\n";
                    }
                
                else {
                    if (verbose)
                        cout << "Insufficient funds to process transaction " << pending_transactions.top().transaction_id << ".\n";
                }

                pending_transactions.pop();
            }
        }

        class transaction_comparator {
            public:
                bool operator() (const Transaction& a, const Transaction& b) {

                    //execution time less than other
                    if (b.execution_time < a.execution_time) {
                        return true;
                    }
                    //execution time more than other
                    else if (b.execution_time > a.execution_time) {
                        return false;
                    }
                    //execution time equal to other
                    else {
                        //transaction id less than other
                        if (b.transaction_id < a.transaction_id) {
                            return true;
                        }
                        //transaction id more than other
                        else {
                            return false;
                        }
                        //no case for transaction id equal to other
                    }
                }
        };

    private:

        priority_queue <Transaction, vector<Transaction>, transaction_comparator> pending_transactions;
    
    public:

        void place() {
            
            string junk;
            cin >> junk; //this will be "lace"

            string placement_time_in;
            cin >> placement_time_in;
            uint64_t converted_placement_time_in = convert_timestamp(placement_time_in);

            string IP_in;
            cin >> IP_in;

            string sender_id_in;
            cin >> sender_id_in;

            string recipient_id_in;
            cin >> recipient_id_in;

            uint32_t amount_in;
            cin >> amount_in;

            string execution_time_in;
            cin >> execution_time_in;
            uint64_t converted_execution_time_in = convert_timestamp(execution_time_in);

            char fee_payer_in;
            cin >> fee_payer_in;
            bool shared_in;
            if (fee_payer_in == 'o') {
                shared_in = false;
            }
            else {
                shared_in = true;
            }
            // else {
            //     cerr << "Incorrect input\n";
            // }

            //errors to check for
            //execution time before placement time error check
            if (execution_time_in < placement_time_in) {
                cerr << "You cannot have an execution date before the current timestamp.\n";
                exit(1); 
            }

            //timestamps decreasing check
            if ((converted_placement_time_in < last_place_time)) {
                cerr << "Invalid decreasing timestamp in 'place' command.\n";
                exit(1);
            }
            else {
                last_place_time = converted_placement_time_in;
            }

            //checking placement validity
            //1. check 3 days or less
            if ((converted_execution_time_in - converted_placement_time_in > 3000000ULL)) {
                if (verbose) {
                    cout << "Select a time less than three days in the future.\n";
                }
                return;
            }

            //2. check sender exists
            auto sender_iter = registered_users.find(sender_id_in);
            if (sender_iter == registered_users.end()) {
                if (verbose) {
                    cout << "Sender "<< sender_id_in << " does not exist.\n";
                }
                return;
            }

            //3. check recipient exists
            auto recipient_iter = registered_users.find(recipient_id_in);
            if (recipient_iter == registered_users.end()) {
                if (verbose) {
                    cout << "Recipient " << recipient_id_in << " does not exist.\n";
                }
                return;
            }

            //4. check execution date after sender and recipient registration date
            uint64_t sender_registration_time = sender_iter -> second.registration_time;
            uint64_t recipient_registration_time = recipient_iter -> second.registration_time;

            //cout << "Converted execution time is: " << converted_execution_time_in << endl;
            //cout << "Sender registration time is: " << sender_registration_time << endl;
            //cout << "Recipient registration time is: " << recipient_registration_time << endl;

            if (converted_execution_time_in < sender_registration_time || converted_execution_time_in < recipient_registration_time) {
                
                if (verbose) {
                    cout << "At the time of execution, sender and/or recipient have not registered.\n";
                }
                return;
            }

            //5. check active session if user is logged in anywhere
            if (sender_iter->second.valid_IPs.empty()) {
                if (verbose) {
                    cout << "Sender " << sender_id_in << " is not logged in.\n";
                }
                return;
            }

            //6. check fraudulent activity
            auto sender_IP_iter = sender_iter->second.valid_IPs.find(IP_in);
            if (sender_IP_iter == sender_iter->second.valid_IPs.end()) {
                if (verbose) {
                    cout << "Fraudulent transaction detected, aborting request.\n";
                }
                return;
            }

            //placement checks are now complete

            //calculate fee
            uint32_t fee;
            fee = amount_in/100;

            if (fee < 10) {
                fee = 10;
            }
            else if (fee > 450) {
                fee = 450;
            }
            //converted_execution_time_in - registered_users[sender_id_in].registration_time > 50000000000ULL
            if (converted_execution_time_in - sender_iter->second.registration_time > 50000000000ULL){
                fee = (fee * 3) / 4;                
            }

            uint32_t fee_recipient;
            uint32_t fee_sender;

            if (shared_in == true) {

                fee_recipient = fee/2;
                fee_sender = fee - fee_recipient;
            }
            else {

                fee_sender = fee;
                fee_recipient = 0;
            }
            

            //last_valid_place_time = converted_placement_time_in;



            //removing transactions
            while (!pending_transactions.empty() && pending_transactions.top().execution_time <= converted_placement_time_in) {
                
                auto iter_to_sender = registered_users.find(pending_transactions.top().sender_id);
                auto iter_to_recipient = registered_users.find(pending_transactions.top().recepient_id);

                if (iter_to_sender -> second.balance >= pending_transactions.top().amount + pending_transactions.top().fee_sender 
                    && iter_to_recipient->second.balance >= pending_transactions.top().fee_recipient) {

                    // registered_users[pending_transactions.top().sender_id].balance -= (pending_transactions.top().amount) + pending_transactions.top().fee_sender;
                    // registered_users[pending_transactions.top().recepient_id].balance -= (pending_transactions.top().fee_recipient);
                    // registered_users[pending_transactions.top().recepient_id].balance += pending_transactions.top().amount;

                    iter_to_sender->second.balance -= pending_transactions.top().amount + pending_transactions.top().fee_sender;
                    iter_to_recipient->second.balance -= pending_transactions.top().fee_recipient;
                    iter_to_recipient->second.balance += pending_transactions.top().amount;

                    transaction_master_list.push_back(pending_transactions.top());
                    iter_to_sender -> second.outgoing_transactions.push_back(pending_transactions.top());
                    iter_to_recipient -> second.incoming_transactions.push_back(pending_transactions.top());

                    if (verbose)
                        cout << "Transaction executed at " << pending_transactions.top().execution_time << ": $" << pending_transactions.top().amount << " from " 
                             << pending_transactions.top().sender_id << " to " << pending_transactions.top().recepient_id << ".\n"; 
                    }
                
                else {
                    if (verbose)
                        cout << "Insufficient funds to process transaction " << pending_transactions.top().transaction_id << ".\n";
                }

                pending_transactions.pop();                
            }

            Transaction temp {transaction_count, converted_placement_time_in, sender_id_in, recipient_id_in, amount_in, converted_execution_time_in, fee, fee_sender, fee_recipient, shared_in};
            pending_transactions.push(temp);
            if (verbose)
                cout << "Transaction placed at " << converted_placement_time_in << ": $" << amount_in << " from " << sender_id_in << " to " 
                     << recipient_id_in << " at " << converted_execution_time_in << ".\n";
            transaction_count++;

        } //place()

        void handle_queries() {

            char first_letter;
            //cin >> first_letter;

            while (cin >> first_letter) {

                //cout << "This is the first letter read: " << first_letter << endl;

                if (first_letter == 'l') list_transactions();

                else if (first_letter == 'r') bank_revenue();

                else if (first_letter == 'h') customer_history();

                else if (first_letter == 's') summarize_day();

                else cout << "Invalid query\n";

                //cin >> first_letter;
            }
        }

        class master_list_comparator {

            public:

                bool operator() (const Transaction& a, const uint64_t& b) {

                    if (a.execution_time < b) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
        };

        void list_transactions() {

            string start_timestamp;
            string end_timestamp;
            
            cin >> start_timestamp;
            cin >> end_timestamp;

            uint64_t converted_start_timestamp;
            uint64_t converted_end_timestamp;

            converted_start_timestamp = convert_timestamp(start_timestamp);
            converted_end_timestamp = convert_timestamp(end_timestamp);

            if (converted_end_timestamp < converted_start_timestamp) {
                cout << "Invalid timestamps\n";
                return;
            }

            // uint32_t transaction_id;
            // uint64_t placement_time;
            // string sender_id;
            // string recepient_id;
            // uint32_t amount;
            // uint64_t execution_time;
            // uint32_t fee;
            // uint32_t fee_sender;
            // uint32_t fee_recipient;
            // bool shared;

            //Transaction temp1 = {0, 0, "", "", 0, converted_start_timestamp, 0, 0, 0, false};
            auto starting_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), converted_start_timestamp, master_list_comparator());

            //Transaction temp2 = {0, 0, "", "", 0, converted_end_timestamp, 0, 0, 0, false};
            auto ending_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), converted_end_timestamp, master_list_comparator());

            if (converted_start_timestamp == converted_end_timestamp) {
                //if (ending_iter != transaction_master_list.end()) {
                    while (ending_iter !=transaction_master_list.end() && ending_iter->execution_time == starting_iter->execution_time) {
                        ending_iter++;
                    }
                //}
            }

            uint64_t transaction_count = 0;
            while (starting_iter != transaction_master_list.end() && starting_iter != ending_iter) {

                if (starting_iter->amount != 1)
                cout << starting_iter->transaction_id << ": " << starting_iter->sender_id << " sent " << starting_iter->amount << " dollars to " 
                     << starting_iter->recepient_id << " at " << starting_iter->execution_time << ".\n";
                else
                cout << starting_iter->transaction_id << ": " << starting_iter->sender_id << " sent " << starting_iter->amount << " dollar to " 
                     << starting_iter->recepient_id << " at " << starting_iter->execution_time << ".\n";
                
                transaction_count++;
                starting_iter ++;
            }

            if (transaction_count != 1)
                cout << "There were " << transaction_count << " transactions that were placed between time " << converted_start_timestamp << " to " << converted_end_timestamp << ".\n";
            else
                cout << "There was " << transaction_count << " transaction that was placed between time " << converted_start_timestamp << " to " << converted_end_timestamp << ".\n";
        }

        void bank_revenue() {
            
            //cout << "called bank_revenue()" << endl;

            string start_timestamp;
            string end_timestamp;
            
            cin >> start_timestamp;
            cin >> end_timestamp;

            uint64_t converted_start_timestamp;
            uint64_t converted_end_timestamp;

            converted_start_timestamp = convert_timestamp(start_timestamp);
            converted_end_timestamp = convert_timestamp(end_timestamp);

            if (converted_end_timestamp < converted_start_timestamp) {
                cout << "Invalid timestamps\n";
                return;
            }

            uint32_t bank_revenue = 0;

            //cout << "made it till here" << endl;

            //Transaction temp1 = {0, 0, "", "", 0, converted_start_timestamp, 0, 0, 0, false};
            auto starting_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), converted_start_timestamp, master_list_comparator());

            //Transaction temp2 = {0, 0, "", "", 0, converted_end_timestamp, 0, 0, 0, false};
            auto ending_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), converted_end_timestamp, master_list_comparator());

            uint64_t time_difference = converted_end_timestamp - converted_start_timestamp;
            uint64_t seconds = time_difference % 100ULL;
            uint64_t minutes = (time_difference % 10000ULL)/100ULL;
            uint64_t hours = (time_difference % 1000000ULL)/10000ULL;
            uint64_t days = (time_difference % 100000000ULL)/1000000ULL;
            uint64_t months = (time_difference % 10000000000ULL)/100000000ULL;
            uint64_t years = (time_difference % 1000000000000ULL)/10000000000ULL;

            if (converted_start_timestamp == converted_end_timestamp) {
                //if (ending_iter != transaction_master_list.end()) {
                    while (ending_iter !=transaction_master_list.end() && ending_iter->execution_time == starting_iter->execution_time) {
                        ending_iter++;
                    }
                //}
            }

            while (starting_iter != transaction_master_list.end() && starting_iter != ending_iter) {

                bank_revenue += starting_iter -> fee;
                starting_iter ++;
            }

            cout << "281Bank has collected " << bank_revenue << " dollars in fees over";
            if (years > 1) cout << " " << years << " years";
            else if (years == 1) cout << " " << years << " year";
            else {};

            if (months > 1) cout << " " << months << " months";
            else if (months == 1) cout << " " << months << " month";
            else {};

            if (days > 1) cout << " " << days << " days";
            else if (days == 1) cout << " " << days << " day";
            else {};

            if (hours > 1) cout << " " << hours << " hours";
            else if (hours == 1) cout << " " << hours << " hour";
            else {};

            if (minutes > 1) cout << " " << minutes << " minutes";
            else if (minutes == 1) cout << " " << minutes << " minute";
            else {};

            if (seconds > 1) cout << " " << seconds << " seconds";
            else if (seconds == 1) cout << " " << seconds << " second";
            else {};

            cout << ".\n";
        }

        void customer_history() {
            
            string user_id_in;
            cin >> user_id_in;

            auto iter_to_customer = registered_users.find(user_id_in);

            if (iter_to_customer == registered_users.end()) { 
                cout << "User " << user_id_in << " does not exist.\n";
                return;
            }
            else {
                size_t num_incoming = iter_to_customer->second.incoming_transactions.size();
                size_t num_outgoing = iter_to_customer->second.outgoing_transactions.size();

                cout << "Customer " << user_id_in << " account summary:\n";
                cout << "Balance: $" << iter_to_customer->second.balance << "\n";
                cout << "Total # of transactions: " << num_incoming + num_outgoing << "\n";

                cout << "Incoming " << num_incoming << ":\n";

                //size_t loop_limit_starter_incoming = max(0UL, num_incoming - 10);

                if (num_incoming < 11) {

                    for (size_t i = 0; i < num_incoming; i++) {
                        cout << iter_to_customer->second.incoming_transactions[i].transaction_id << ": " << iter_to_customer->second.incoming_transactions[i].sender_id
                            << " sent " << iter_to_customer->second.incoming_transactions[i].amount; 
                        if (iter_to_customer->second.incoming_transactions[i].amount == 1) {
                            cout << " dollar to ";
                        }
                        else {
                            cout << " dollars to ";
                        }     
                        cout << iter_to_customer->second.incoming_transactions[i].recepient_id << " at " 
                            << iter_to_customer->second.incoming_transactions[i].execution_time << ".\n";
                    }
                }
                else {

                    for (size_t i = num_incoming - 10; i < num_incoming; i++) {
                        cout << iter_to_customer->second.incoming_transactions[i].transaction_id << ": " << iter_to_customer->second.incoming_transactions[i].sender_id
                            << " sent " << iter_to_customer->second.incoming_transactions[i].amount; 
                        if (iter_to_customer->second.incoming_transactions[i].amount == 1) {
                            cout << " dollar to ";
                        }
                        else {
                            cout << " dollars to ";
                        }     
                        cout << iter_to_customer->second.incoming_transactions[i].recepient_id << " at " 
                            << iter_to_customer->second.incoming_transactions[i].execution_time << ".\n";
                    }
                }

                cout << "Outgoing " << num_outgoing << ":\n";

                //size_t loop_limit_starter_outgoing = max(0UL, num_outgoing - 10);
                //cout << "This is the loop starter value for outgoing: " << loop_limit_starter_outgoing << endl;

                if (num_outgoing < 11) {

                    for (size_t i = 0; i < num_outgoing; i++) {
                        cout << iter_to_customer->second.outgoing_transactions[i].transaction_id << ": " << iter_to_customer->second.outgoing_transactions[i].sender_id
                            << " sent " << iter_to_customer->second.outgoing_transactions[i].amount;
                        if (iter_to_customer->second.outgoing_transactions[i].amount == 1) {
                            cout << " dollar to ";
                        }
                        else {
                            cout << " dollars to ";
                        }
                        cout << iter_to_customer->second.outgoing_transactions[i].recepient_id << " at "
                            << iter_to_customer->second.outgoing_transactions[i].execution_time << ".\n";
                        
                    }
                }
                else {

                    for (size_t i = num_outgoing - 10; i < num_outgoing; i++) {
                        cout << iter_to_customer->second.outgoing_transactions[i].transaction_id << ": " << iter_to_customer->second.outgoing_transactions[i].sender_id
                            << " sent " << iter_to_customer->second.outgoing_transactions[i].amount;
                        if (iter_to_customer->second.outgoing_transactions[i].amount == 1) {
                            cout << " dollar to ";
                        }
                        else {
                            cout << " dollars to ";
                        }
                        cout << iter_to_customer->second.outgoing_transactions[i].recepient_id << " at "
                            << iter_to_customer->second.outgoing_transactions[i].execution_time << ".\n";
                        
                    }

                }


            }
        } 

        void summarize_day() {
            
            string timestamp_in;
            cin >> timestamp_in;

            uint64_t converted_timestamp_in;
            converted_timestamp_in = convert_timestamp(timestamp_in);

            uint64_t start_timestamp = (converted_timestamp_in/1000000) * 1000000;
            uint64_t end_timestamp = start_timestamp + 1000000;

            uint32_t bank_revenue_in_day = 0;

            cout << "Summary of [" << start_timestamp << ", " << end_timestamp << "):\n";

            //Transaction temp1 = {0, 0, "", "", 0, start_timestamp, 0, 0, 0, false};
            auto starting_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), start_timestamp, master_list_comparator());

            //Transaction temp2 = {0, 0, "", "", 0, end_timestamp, 0, 0, 0, false};
            auto ending_iter = lower_bound(transaction_master_list.begin(), transaction_master_list.end(), end_timestamp, master_list_comparator());

            uint64_t num_transactions_in_day = 0;
            while (starting_iter != transaction_master_list.end() && starting_iter != ending_iter) {

                if (starting_iter->amount != 1)
                    cout << starting_iter->transaction_id << ": " << starting_iter->sender_id << " sent " << starting_iter->amount << " dollars to " 
                         << starting_iter->recepient_id << " at " << starting_iter->execution_time << ".\n";
                else
                    cout << starting_iter->transaction_id << ": " << starting_iter->sender_id << " sent " << starting_iter->amount << " dollar to " 
                         << starting_iter->recepient_id << " at " << starting_iter->execution_time << ".\n";
                
                num_transactions_in_day++;
                bank_revenue_in_day += starting_iter -> fee;

                starting_iter ++;
            }

            if (num_transactions_in_day != 1)
                cout << "There were a total of " << num_transactions_in_day << " transactions, 281Bank has collected " << bank_revenue_in_day << " dollars in fees.\n";
            else
                cout << "There was a total of " << num_transactions_in_day << " transaction, 281Bank has collected " << bank_revenue_in_day << " dollars in fees.\n";
        }

};

int main(int argc, char** argv) {

            ios_base::sync_with_stdio(false);

            //cout << "hello world" << endl;

            Bank bank;

            bank.get_options(argc, argv);

            bank.read_users();

            bank.handle_operations();

            bank.handle_queries();

            return 0;
        }
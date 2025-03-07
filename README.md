# Bank Simulator

## Overview
The **Bank Simulator** is a C++ program that simulates a banking system where users can register, log in, perform transactions, and query account details. The program processes user actions based on command-line input, handling banking transactions, account management, and historical queries.

## Features
- **User Registration**: Loads registered users from an input file.
- **Login/Logout**: Users can log in with their credentials and perform transactions.
- **Transaction Processing**: Handles placing and executing transactions securely.
- **Fraud Detection**: Prevents unauthorized transactions from unverified IP addresses.
- **Query Support**: Provides transaction history, bank revenue, and user-specific details.
- **Verbose Mode**: Displays detailed logs when enabled.

## File Structure
```
.
├── bank.cpp                 # Main program source file
├── Makefile                 # Compilation script
├── spec-commands.txt        # Example commands for the simulator
├── spec-output-verbose.txt  # Expected output in verbose mode
├── spec-reg.txt             # Sample user registration data
├── Error_messages.txt       # Error messages reference file
├── README.md                # Documentation (this file)
├── output-test-*.txt        # Test output files
├── test-*-commands.txt      # Test case command files
├── test-*-reg.txt           # Test case user registration files
```

## Installation and Compilation
To compile the program, use the provided **Makefile**:
```sh
make
```
This will generate an executable file.

## Usage
Run the program with optional flags:
```sh
./bank_simulator [OPTIONS]
```
Options:
- `-f <filename>` : Specify the registration file.
- `-v` : Enable verbose mode for detailed logs.
- `-h` : Display help message.

Example command:
```sh
./bank_simulator -f spec-reg.txt -v
```

## Input Format
The program reads input from a command file containing banking operations:
```
login <user_id> <pin> <IP>
place <timestamp> <IP> <sender_id> <recipient_id> <amount> <execution_time> <fee_type>
out <user_id> <IP>
$$$
l <start_time> <end_time>   # List transactions
r <start_time> <end_time>   # Bank revenue
h <user_id>                 # Customer history
s <timestamp>               # Summary of transactions for a day
```

## Code Breakdown

### **1. Command-Line Argument Handling**
- Parses options like `-f` (input file) and `-v` (verbose mode).
- Displays a help message if `-h` is passed.

### **2. User Registration**
- Reads users from a file (`spec-reg.txt`) in the format:
  ```
  <timestamp>|<user_id>|<pin>|<balance>
  ```
- Errors handled:
  - Missing file → Program exits.
  - Malformed entries → May cause `stoi()` errors.

### **3. Login & Logout**
- **Login:** Validates user credentials and registers an IP address.
- **Logout:** Ensures a user logs out from a valid session.
- Edge Cases:
  - Incorrect PIN → `"Failed to log in <user_id>."`
  - User not found → Login fails.
  - Logging out from an unknown IP fails.

### **4. Transaction Processing**
- **Placing a Transaction:**
  - Ensures sender and recipient exist and timestamps are valid.
  - Verifies that sender is logged in from the correct IP.
  - Calculates a fee based on transaction amount.
  - Edge Cases & Errors:
    - Execution time before placement → `"You cannot have an execution date before the current timestamp."`
    - Sender not logged in → `"Sender <id> is not logged in."`
    - Fraudulent transaction detection → `"Fraudulent transaction detected, aborting request."`

- **Executing Transactions (`$$$` marker):**
  - Transactions are processed in order of execution time.
  - If a sender lacks sufficient funds, the transaction fails.
  - Edge Cases:
    - `"Insufficient funds to process transaction <id>."`
    - Overdraft protection prevents unintended transactions.

### **5. Queries**
- **List transactions (`l <start_time> <end_time>`)**: Retrieves transactions between timestamps.
- **Calculate bank revenue (`r <start_time> <end_time>`)**: Computes collected fees.
- **Show customer history (`h <user_id>`)**: Displays transaction history.
- **Summarize a day's transactions (`s <timestamp>`)**: Aggregates daily transactions and revenue.

## Sample Run
Input from `spec-commands.txt`:
```
login mertg 080400 231.43.171.61
place 08:02:01:40:22:34 134.31.78.43 mertg paoletti 2000 09:02:01:40:22:34 s
out mertg 231.43.171.61
$$$
l 08:03:01:40:22:36 09:03:01:40:22:34
```

Expected output (`spec-output-verbose.txt`):
```
Failed to log in mertg.
Transaction placed at 80301402236: $2000 from mertg to paoletti at 80302402234.
Transaction executed at 80302402234: $2000 from mertg to paoletti.
User mertg logged out.
281Bank has collected 40 dollars in fees.
```

## Error Handling Summary
- **Invalid login attempts are rejected.**
- **Transactions require users to be logged in from a valid IP.**
- **Execution timestamps must be valid and increasing.**
- **Fraudulent transactions are detected and rejected.**
- **Insufficient funds cause transactions to fail.**

## Author
Developed as part of an **EECS 281** course project.

## License
This project is for educational purposes and is not intended for real-world financial transactions.


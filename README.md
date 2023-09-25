# myBash - Bash-like Shell in C

## Table of Contents

- [Project Description](#project-description)
- [Project Files](#project-files)
- [Tools and Technologies Used](#tools-and-technologies-used)
- [Instructions to Run the Project](#instructions-to-run-the-project)
- [License](#license)
- [Participants](#participants)

## Project Description

The myBash project is a laboratory assignment involving the creation of a Bash-style shell programmed in the C language. This console performs various general functions, such as executing internal or external commands in the background or foreground, redirecting between commands through pipes, and standard input and output redirection.

## Project Files

- **command**:
  - Description: Contains definitions and functions related to the `scommand` (simple command) and `pipeline` (sequence of simple commands separated by pipes) ADTs.
  - Purpose: Facilitates the manipulation and management of simple commands and pipelines in the project.

- **builtin**:
  - Description: Defines functions to check if a command is internal and to execute internal commands.
  - Purpose: Provides an interface to identify and execute internal commands in the shell.

- **execute**:
  - Description: Contains the function to execute pipelines, identify internal commands, and manage redirection.
  - Purpose: Manages the execution of command pipelines and their interaction with internal commands.

- **parser**:
  - Description: Defines the `Parser` ADT and related functions for parsing user input and splitting it into commands and arguments.
  - Purpose: Provides tools for parsing and processing commands entered by the user.

- **parsing**:
  - Description: Contains the `parse_pipeline` function for parsing a command pipeline and creating a corresponding data structure.
  - Purpose: Facilitates syntactical analysis of commands entered by the user and creates data structures used in the shell.

- **make**:
  - Description: This file is a Makefile used to compile the project. It defines compilation rules and commands to build the shell executable.

These files are essential for the project's operation and are used to manage commands, analyze user input, and execute commands in the shell. If you have more details or specific questions about any of these files, feel free to ask.

## Tools and Technologies Used

- **Programming Language:** C
- **Version Control Tools:** GIT and BitBucket
- **Build Tool:** GNU Make
- **Debugging Tools:** Valgrind for memory leak detection, GDB for debugging
- **Project Organization Tools:** Jira and Trello
- **Unit Testing:** Unit tests were used to ensure code quality.
- **Methodology:** Agile methodology
- **Programming Technique:** Defensive programming

## Instructions to Run the Project

### 1. Installing Dependencies

Before running the project, you need to install the necessary dependencies. On a Linux system, you can do this with the following commands:

```bash
sudo apt-get update
sudo apt-get install libglib2.0-dev
sudo apt-get install check
```

### 2. Cloning the Repository

Clone the myBash repository from BitBucket or GitHub using the following command:

```bash
git clone https://your-repository.git
```

### Compilation and Execution

After cloning the repository, navigate to the project's root directory:

```bash
cd myBash
```

Next, compile the code using GNU Make
```bash
make
```
Once compiled, you can run the shell:
```bash
./mybash
```

## 4. Cleaning

To remove all generated executable and object files during compilation, you can use the following command:

```bash
make clean
```

## Commands to run Tests
To run tests from the command file:

```bash
make test-command
```

To run tests from the parsing module:

```bash
make test-parsing
```

For general testing of the entire functionality:

```bash
make test
```

To run memory tests using Valgrind:

```bash
make memtest
```

## License

This project is distributed under the MIT License. For more details, refer to the LICENSE file in the repository.

## Participants


- **Esteban Ezequiel Marin Reyna**
  - Email: ezequiel.marin@mi.unc.edu.ar

- **Alexis Joaquin Ortiz**
  - Email: 

- **Manu Joan Saharrea**
  - Email: 

- **Ignacio A. Ramirez**
  - Email: 

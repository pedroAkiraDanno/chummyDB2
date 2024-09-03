


/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * 02-07-2022, v1.0, Pedro Akira Danno Lima
 *
 * Compile: gcc -Wall -O2 db.c -o dbexec
 */











#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Define the structures
#define PAGE_SIZE 8192

struct PageHeader {
    uint32_t pageNo;
    uint32_t nextPageNo;
    uint16_t freeSpace;
    uint16_t nItems;
    uint16_t offset; // New field for offset
};

struct Person {
    char name[50];
    char address[100];
    int age;
};

struct Tuple {
    uint16_t length;
    struct Person person;
};

// Function to initialize a page header
void initPageHeader(struct PageHeader *header, uint32_t pageNo) {
    header->pageNo = pageNo;
    header->nextPageNo = 0;
    header->freeSpace = PAGE_SIZE - sizeof(struct PageHeader);
    header->nItems = 0;
    header->offset = sizeof(struct PageHeader); // Initialize offset after the header
}

// Function to write a tuple to a binary file
void writeTupleToBinaryFile(FILE *file, const struct Tuple *tuple) {
    fwrite(tuple, sizeof(struct Tuple), 1, file);
}

// Function to write a tuple to a text file
void writeTupleToTextFile(FILE *file, const struct Tuple *tuple) {
    fprintf(file, "Name: %s\nAddress: %s\nAge: %d\n\n", tuple->person.name, tuple->person.address, tuple->person.age);
}

// Function to write pages to binary and text files
void writePagesToFile(int totalTuples) {
    FILE *binaryFile = fopen("database.bin", "wb"); // Open binary file for writing in binary mode
    FILE *textFile = fopen("database.txt", "w"); // Open text file for writing

    if (binaryFile == NULL || textFile == NULL) {
        printf("Error: Unable to open files for writing.\n");
        return;
    }

    struct PageHeader header;
    initPageHeader(&header, 0); // Initialize the first page header
    fwrite(&header, sizeof(struct PageHeader), 1, binaryFile); // Write the page header to the binary file

    struct Tuple tuple;
    for (int i = 0; i < totalTuples; i++) {
        sprintf(tuple.person.name, "Person %d", i + 1);
        strcpy(tuple.person.address, "Some Address");
        tuple.person.age = 30 + i % 70; // Age can vary between 30 and 99
        tuple.length = sizeof(struct Person);

        // Write tuple to binary file
        writeTupleToBinaryFile(binaryFile, &tuple);

        // Write tuple to text file
        writeTupleToTextFile(textFile, &tuple);

        header.nItems++;
        header.freeSpace -= tuple.length;

        if (header.freeSpace < sizeof(struct Tuple)) {
            header.nextPageNo = header.pageNo + 1;
            fwrite(&header, sizeof(struct PageHeader), 1, binaryFile); // Update the page header in the binary file
            initPageHeader(&header, header.nextPageNo); // Initialize a new page header
            fwrite(&header, sizeof(struct PageHeader), 1, binaryFile); // Write the new page header to the binary file
        }
    }

    fclose(binaryFile); // Close the binary file
    fclose(textFile); // Close the text file
}

// Function to display result in prompt
void displayResult(int totalPages, int totalTuples) {
    printf("Data has been written to database.bin and database.txt\n");
    printf("Total Pages: %d\n", totalPages);
    printf("Total Tuples: %d\n", totalTuples);
}






// Function to read and display content from a text file
void displayContentFromTextFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening text file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
}





// Function to read and display tuples from a binary file
void displayTuplesFromBinaryFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening binary file");
        return;
    }

    struct PageHeader header;
    struct Tuple tuple;

    while (fread(&header, sizeof(struct PageHeader), 1, file) == 1) {
        printf("Page No: %u\n", header.pageNo);
        printf("Next Page No: %u\n", header.nextPageNo);
        printf("Free Space: %u\n", header.freeSpace);
        printf("Number of Items: %u\n", header.nItems);
        printf("Offset: %u\n", header.offset);
        
        for (int i = 0; i < header.nItems; i++) {
            if (fread(&tuple, sizeof(struct Tuple), 1, file) != 1) {
                perror("Error reading tuple from binary file");
                fclose(file);
                return;
            }

            printf("Tuple %d:\n", i + 1);
            printf("  Name: %s\n", tuple.person.name);
            printf("  Address: %s\n", tuple.person.address);
            printf("  Age: %d\n", tuple.person.age);
        }
    }

    fclose(file);
}












// Example usage
int main() {
    int totalTuples = 1000000; // Change this to the desired total number of tuples

    writePagesToFile(totalTuples);
    displayResult(totalTuples / (PAGE_SIZE / sizeof(struct Tuple)), totalTuples);


    // Display content from binary file
    printf("Displaying content from binary file:\n");
    displayTuplesFromBinaryFile("database.bin");

    // Display content from text file
    printf("\nDisplaying content from text file:\n");
    displayContentFromTextFile("database.txt");


    

    return 0;
}











/*





To ensure that tuples are added to a new page if the current page exceeds the 8KB size limit, we need to implement a mechanism that checks the remaining space in the page before adding a new tuple. If the remaining space is not sufficient, we create a new page and add the tuple to that page instead. Here's how you can modify the code to achieve this:




In this updated code:

The addTupleToPage function checks if there is enough space in the current page to accommodate the new tuple.
If there is not enough space, it creates a new page and adds the tuple to that page recursively.
It updates the nextPageNo field in the current page header to link it to the newly created page.
This process continues until there is enough space in the page to add the tuple.
The main function demonstrates adding multiple tuples to the page, triggering the creation of new pages as needed when the current page becomes full.
This modification ensures that tuples are added to new pages when the current page's size limit is reached, allowing for efficient management of data within the page structure.







Page 1:
+-------------------------+
|        PageHeader       |
|-------------------------|
| pageNo: 1               |
| nextPageNo: 2           |
| freeSpace: 8100         |
| nItems: 8               |
| offset: sizeof(PageHeader) |
+-------------------------+
|        Tuple 1          |
|-------------------------|
|        Tuple 2          |
|-------------------------|
|        Tuple 3          |
|-------------------------|
|        Tuple 4          |
|-------------------------|
|        Tuple 5          |
|-------------------------|
|        Tuple 6          |
|-------------------------|
|        Tuple 7          |
|-------------------------|
|        Tuple 8          |
+-------------------------+

Page 2:
+-------------------------+
|        PageHeader       |
|-------------------------|
| pageNo: 2               |
| nextPageNo: 0           |
| freeSpace: 8192         |
| nItems: 2               |
| offset: sizeof(PageHeader) |
+-------------------------+
|        Tuple 9          |
|-------------------------|
|        Tuple 10         |
+-------------------------+






In this visualization:

We have two pages: Page 1 and Page 2.
Page 1 contains tuples 1 to 8, and its nextPageNo points to Page 2.
Page 2 contains tuples 9 and 10. Since it's the last page, its nextPageNo is 0, indicating no next page.
This visualization demonstrates how the code dynamically creates new pages when the current page reaches its size limit, allowing for efficient storage of tuples within the page structure.







*/




















/*





COMPILE:    LINUX 

    apt-get install gcc
    vi db.c
    gcc db.c -o dbexec
    ./dbexec






COMPILE:    WINDOWS 
        cd C:\Users\pedro.akira\OneDrive - pwi.com.br\Documentos\GitHub\chummyDB\src
        gcc db6.c -o db6exec
        .\db6exec 








*/









































/*

EXPLAIN: 

























This C program demonstrates a simple implementation of a page-based data structure for storing tuples of data. Let's break down the main components:

1. **Header Definitions**: 
    - The program defines several structures, namely `PageHeader`, `Person`, `Tuple`, and `Page`, to represent the components of the data storage.

2. **Page Structure**:
    - The `Page` structure combines a `PageHeader` and an array of data to represent a single page in the data structure.

3. **Initialization Function**:
    - The `initPage` function initializes a page with the provided page number and sets initial metadata values.

4. **Add Tuple Function**:
    - The `addTupleToPage` function adds a tuple to a page. It calculates the position to insert the tuple, checks if there's enough space, and either inserts the tuple into the current page or recursively creates a new page and adds the tuple to it.

5. **Main Function**:
    - The `main` function demonstrates the usage of the provided functions by initializing a page and adding ten tuples of person data to it.

6. **Memory Management**:
    - The program uses `malloc` to allocate memory for new pages and `free` to release memory after use, ensuring proper memory management.

7. **Example Data**:
    - Each tuple represents a person with a name, address, and age. These tuples are added to the page in the main function.

Overall, this program provides a basic framework for managing data in a page-based structure, suitable for scenarios where data needs to be stored and managed efficiently.




*/
































  /*****************************************************************************************************************
  *                                                   REFERENCIAS                                                  *
  * https://pt.stackoverflow.com/questions/104118/qual-%C3%A9-a-finalidade-dos-comandos-size-t-e-ssize-t-em-c      *
  ******************************************************************************************************************/












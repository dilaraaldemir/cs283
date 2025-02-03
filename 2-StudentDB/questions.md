1. In this assignment I asked you provide an implementation for the get_student(...) function because I think it improves the overall design of the database application. After you implemented your solution do you agree that externalizing get_student(...) into it's own function is a good design strategy? Briefly describe why or why not.

- I agree that externalizing get_student(...) into its own function is a good design strategy. I think this improves the overall design by increasing the readability of the code and make it easier to test at the end. It also helped to make sure there is no code duplication.

2. Another interesting aspect of the get_student(...) function is how its function prototype requires the caller to provide the storage for the student_t structure:

int get_student(int fd, int id, student_t *s);
Notice that the last parameter is a pointer to storage provided by the caller to be used by this function to populate information about the desired student that is queried from the database file. This is a common convention (called pass-by-reference) in the C programming language.

In other programming languages an approach like the one shown below would be more idiomatic for creating a function like get_student() (specifically the storage is provided by the get_student(...) function itself):

//Lookup student from the database
// IF FOUND: return pointer to student data
// IF NOT FOUND: return NULL
student_t *get_student(int fd, int id){
    student_t student;
    bool student_found = false;
    
    //code that looks for the student and if
    //found populates the student structure
    //The found_student variable will be set
    //to true if the student is in the database
    //or false otherwise.

    if (student_found)
        return &student;
    else
        return NULL;
}

Can you think of any reason why the above implementation would be a very bad idea using the C programming language? Specifically, address why the above code introduces a subtle bug that could be hard to identify at runtime? 

- The above implementation would be problematic because it would involve returning a pointer to a local variable. In C, local variables are stored in the stack and memory is released when the function exists, so it would lead to pointing to invalid memory which will cause unidentified behavior. The bug would be hard to detect because it may not show up immediatedly and cause later issues.

3. Another way the get_student(...) function could be implemented is as follows:

//Lookup student from the database
// IF FOUND: return pointer to student data
// IF NOT FOUND or memory allocation error: return NULL
student_t *get_student(int fd, int id){
    student_t *pstudent;
    bool student_found = false;

    pstudent = malloc(sizeof(student_t));
    if (pstudent == NULL)
        return NULL;
    
    //code that looks for the student and if
    //found populates the student structure
    //The found_student variable will be set
    //to true if the student is in the database
    //or false otherwise.

    if (student_found){
        return pstudent;
    }
    else {
        free(pstudent);
        return NULL;
    }
}

In this implementation the storage for the student record is allocated on the heap using malloc() and passed back to the caller when the function returns. What do you think about this alternative implementation of get_student(...)? Address in your answer why it work work, but also think about any potential problems it could cause.

- This implementation works by allocating memory for the student on the heap using malloc(), which allows the function to return the pointer to the caller. This would have some potential issues like causing memory leaks if the caller doesn't free the memory or if malloc() fails, it would return NULL, but this would cause allocation failure.

4. Lets take a look at how storage is managed for our simple database. Recall that all student records are stored on disk using the layout of the student_t structure (which has a size of 64 bytes). Lets start with a fresh database by deleting the student.db file using the command rm ./student.db. Now that we have an empty database lets add a few students and see what is happening under the covers. Consider the following sequence of commands:

> ./sdbsc -a 1 john doe 345
> ls -l ./student.db
    -rw-r----- 1 bsm23 bsm23 128 Jan 17 10:01 ./student.db
> du -h ./student.db
    4.0K    ./student.db
> ./sdbsc -a 3 jane doe 390
> ls -l ./student.db
    -rw-r----- 1 bsm23 bsm23 256 Jan 17 10:02 ./student.db
> du -h ./student.db
    4.0K    ./student.db
> ./sdbsc -a 63 jim doe 285 
> du -h ./student.db
    4.0K    ./student.db
> ./sdbsc -a 64 janet doe 310
> du -h ./student.db
    8.0K    ./student.db
> ls -l ./student.db
    -rw-r----- 1 bsm23 bsm23 4160 Jan 17 10:03 ./student.db
For this question I am asking you to perform some online research to investigate why there is a difference between the size of the file reported by the ls command and the actual storage used on the disk reported by the du command. Understanding why this happens by design is important since all good systems programmers need to understand things like how linux creates sparse files, and how linux physically stores data on disk using fixed block sizes. Some good google searches to get you started: "lseek syscall holes and sparse files", and "linux file system blocks". After you do some research please answer the following:

Please explain why the file size reported by the ls command was 128 bytes after adding student with ID=1, 256 after adding student with ID=3, and 4160 after adding the student with ID=64?

- The difference would be due to sparse files. ls shows the logical size which is the total file size including empty areas, while du shows the actual disk usage which shows only used space. So sparse files save disk space by not allocating space for empty areas, so du shows less usage until enough data fills the gaps.

Why did the total storage used on the disk remain unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64?

- The disk storage remained unchanged for IDs 1, 3, and 63 because sparse files only allocate physical space for actual data, leaving empty parts unallocated. When ID=64 was added, it filled the gap and required physical storage, increasing the disk usage from 4K to 8K.

Now lets add one more student with a large student ID number and see what happens:

> ./sdbsc -a 99999 big dude 205 
> ls -l ./student.db
-rw-r----- 1 bsm23 bsm23 6400000 Jan 17 10:28 ./student.db
> du -h ./student.db
12K     ./student.db
We see from above adding a student with a very large student ID (ID=99999) increased the file size to 6400000 as shown by ls but the raw storage only increased to 12K as reported by du. Can provide some insight into why this happened?

- The large increase in the file size is due to the logical file size increasing as the student with ID 99999 is added, which requires space at a large offset. However, du only increased by 12K because sparse files are used, which don't allocate physical space for empty areas between records. The file size grows logically, but physical storage is only allocated for actual data, which causes minimal storage usage.

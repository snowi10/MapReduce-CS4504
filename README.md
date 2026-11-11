# MapReduce-CS4504
A demonstration of a sequential MapReduce for solving the WordCount example. 
## Setup
- Make sure you have a C compiler installed.
- Compile MapReduce.c and linked_list.c.\
    Example:
    ```
    gcc MapReduce.c linked_list.c -o MapReduce -lm
    ```
- Execute file with arguments (file name, number of mappers, number of reducers).
    - The file name is the the name of the text file that you want to call MapReduce on.
    - The number of mappers is the number of "threads" you want to use to "parallelize" the Map function.
    - The number of reducers is the number of "threads" you want to use to "parallelize" the Reduce function.

    Example:
    ```
    ./MapReduce demo1.txt 10 10
    ```
## Output
- Output should be a new line of each unique word in the text file and the number of times it appears in the file.
- Example input file:
    ```
    Hello world
    Goodbye world
    Hello Hadoop
    Goodbye Hadoop
    ```
- Example output file:
    ```
    Hello 2
    world 2
    Goodbye 2
    Hadoop 2
    ```
    Output file is named "reduce_output.txt" by default.

- The list of intermediate values is also output as a text file.\
    Example output of intermediate values:
    ```
    0 Hello 5 1 1
    2 world 5 1 1
    3 Goodbye 7 1 1
    2 world 5 1 1
    0 Hello 5 1 1
    3 Hadoop 6 1 1
    3 Goodbye 7 1 1
    3 Hadoop 6 1 1
    0 Hello 5 1 1
    ```
    - First item is the ID of the reducer "thread" that the word will be sent to.
    - Second item is the word in the text file.
    - Third item is the length of the word.
    - Fourth item is the value of the word.
    - Fifth item is the number of digits in the value.

    Output file of intermediate values is named "interm_vals.txt" by default.
## Limitations
- The program is specific to the WordCount example and, therefore, only considers words as keys and the number 1 as values in which the operation on a key's list of values is a summation.
- The program can only read data in text files assuming that it only contains ASCII characters.
- The program is case-sensitive (i.e. "Dog" would be considered a different word/key than "dog").
- Punctuation characters are not considered as their own unique keys if there are no spaces between the preceding or proceeding words that follow them (i.e., in the sentence, "This is a dog; the dog is cute.", ";" would be attached to the word "dog" and "." would be attached to the word "cute". Therefore, two of the resulting keys from this sentence would be "dog;" and "cute.").
- No actual parallelization occurs within the program.
## Future Implementations
- Applying pthreads to parallelize the Map and Reduce functions.
- Allowing more control over specifying keys and values, file type, and operations on the values.
## Resources
- https://hadoop.apache.org/docs/current/hadoop-mapreduce-client/hadoop-mapreduce-client-core/MapReduceTutorial.html
- https://www.baeldung.com/cs/mapreduce-algorithm
- https://www.geeksforgeeks.org/software-engineering/mapreduce-architecture/
- https://en.wikipedia.org/wiki/MapReduce

![Alt text](https://i.sstatic.net/aIGRQ.png)

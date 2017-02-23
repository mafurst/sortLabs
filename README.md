# sortLabs
C++ program intended to sort labs submitted to the Canvas site.

When downloading file submissions as a batch from canvas, I noticed that they always followed the same naming structure: "lastfirst_numbers_morenumbers_filename.extension". I figured that there must be a way to easily sort these so I wrote up this program.

The program operates by taking a single argument, the path to a directory containing all the submissions. Ex: "/Users/Michael/Submissions". It then loads each file from the directory using the dirent.h header file. This file comes included on MacOS and Linux, so I had to just find a solution for Windows. The solution was pretty easy. Toni Ronko has a dirent.h file obtainable from his [github](https://github.com/tronkko/dirent) that can just be added to the same directory as the cpp file. This solved the issue.

My next step was to cut down the file name, a few loops later and we get the user's name as "lastfirst" and the submission name as "filename.extension". It even rips out the added -1, -2, etc. that canvas adds in when there are multiple submissions, by the user, of the same file. This makes it a lot easier to just import the files over and over again into the same project when grading them. 

The program takes the user's name and creates a new folder for it, so we would have a structure like: 

Submissions

->doejohn

->suemary

->smithsmith

With this structure we can easily navigate to each user's submission. It then copies each file into the the appropriate directory for the user so something like:

Submissions

->doejohn_..._Circle.java

->doejohn_..._Square.java

->doejohn_..._Triangle-1.java

->suemary_..._Circle.java

->suemary_..._Square.java

->suemary_..._Triangle.java

becomes:

Submissions

->doejohn

-->Circle.java

-->Square.java

-->Triangle.java

->suemary

-->Circle.java

-->Square.java

-->Triangle.java

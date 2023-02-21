# Upload Daemon

This daemon process will iterate over the files of the provided directory and extract out all the .jpg, .jpeg, .png files and archive them in a zip file which will then be automatically uploaded to the google drive.

The daemon process will check the directory every 6 hours.

All the files in the provided folder will be deleted in each iteration. So, make sure that you have the copy of those files [.png, .jpg, .jpeg]

### To compile:

You will need to two other libraries:

1. JSON for Modern C++ : [link](https://github.com/nlohmann/json)
2. C++ Requests: Curl for People, a spiritual port of Python Requests. : [link](https://github.com/libcpr/cpr)

Clone the above repositories in a folder named as 'external' in the root directory
Create a folder named 'build' in the root directory.

`$ cmake -S . -B build/`

### To build:

`$ cmake --build build/`

If everything was successful, now the executable have been generated in the 'build/driver/' folder


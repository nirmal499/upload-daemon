#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <request_srv/request.hpp>
#include <chrono>

FILE *fp;
const char pidfile[] = "/var/run/my-daemon.pid";
struct sigaction action; /* for sigaction */

void sigHandler(int sig)
{
    int status = 0;

    if (sig == SIGTERM || sig == SIGINT || sig == SIGQUIT || sig == SIGABRT)
    {
        /** remove the pidfile */
        if ((unlink(pidfile)) == -1)
        {
            status = 1;
        }

        if ((fclose(fp)) == EOF)
        {
            status = 1;
        }

        exit(status);
        /** exit with the status set */
    }
    else
    {
        /* some other signal */
        exit(1);
    }
}

int main()
{
    time_t now;
    const char daemon_error_file[] = "/home/nbaskey/Pictures/for_daemon/daemon-error-details.txt";
    std::ofstream fout_for_error;
    fout_for_error.open(daemon_error_file, std::ios::app);
    if (!fout_for_error)
    {
        return EXIT_FAILURE;
    }

    try
    {
        pid_t pid;
        const char daemonfile[] = "/home/nbaskey/Pictures/for_daemon/daemon-details.txt";

        if ((pid = fork()) == -1)
        {
            throw std::runtime_error("Can't Fork");
        }
        else if (pid != 0)
        {
            exit(EXIT_SUCCESS);
        }
        /* The parent process has exited, so this is the
         * child. create a new session to lose the
         * controlling terminal */

        /**
         * In the child process, we will create a new session with setsid().
         * Creating a new session will free the process from the controlling terminal
         */
        setsid();

        /**
         * forking again, creating a grandchild, the actual daemon
         */
        if ((pid = fork()) == -1)
        {
            throw std::runtime_error("Can't fork");
        }
        else if (pid > 0)
        {
            /**
             * open pid-file for writing and error check it
             */

            /* the child process which will exit */
            std::ofstream fpout;
            fpout.open(pidfile, std::ios::out);
            // want to open a file in write mode and want to truncate it in case that already exists

            if (!fpout)
            {
                throw std::runtime_error("Can't open file for writing");
            }

            fpout << pid << "\n";

            fpout.close();

            exit(EXIT_SUCCESS);
        }

        // For directories, the base permissions are (rwxrwxrwx) 0777 and for files they are 0666 (rw-rw-rw)
        // The default umask 002 used for normal user. With this mask default directory permissions are 775 and default file permissions are 664
        //  0 : read, write and execute
        // 1 : read and write
        // 2 : read and execute
        // 3 : read only
        // 4 : write and execute
        // 5 : write only
        // 6 : execute only
        // 7 : no permissions
        umask(022); /** set the umask to something ok */

        (void)chdir("/"); /** change working directory to '/' */

        /** open the "daemonfile" for writing */
        std::ofstream fout;
        fout.open(daemonfile, std::ios::app);

        if (!fout)
        {
            throw std::runtime_error("Can't open daemonfile");
        }

        close(STDIN_FILENO);  // The stdin attached to keyboard will get closed
        close(STDOUT_FILENO); // The stdout attached to console will get closed
        close(STDERR_FILENO); // The stderr attached to console will get closed

        open("/dev/null", O_RDONLY); /** stdin; fd0 will be attached to /dev/null */
        open("/dev/null", O_WRONLY); /** stdout; fd1 will be attached to /dev/null */
        open("/dev/null", O_RDWR);   /** stderr; fd2 will be attached to /dev/null */

        /** prepare for sigaction */
        action.sa_handler = sigHandler;
        sigfillset(&action.sa_mask);

        action.sa_flags = SA_RESTART;

        /** register the signals we want to handle */
        sigaction(SIGTERM, &action, NULL); // terminate
        sigaction(SIGINT, &action, NULL);  // interrupt
        sigaction(SIGQUIT, &action, NULL); // Quit
        sigaction(SIGABRT, &action, NULL); // Abort

        /** here we start the daemons "work" */
        int count = 1;
        while (true)
        {

            // SKIP sleep for first time running
            if (count != 1)
            {
                sleep(21600); // It takes seconds
                // 21600 seconds -> 6hours
            }

            ++count;

            using namespace RQH;

            RequestCls obj(
                "https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable",
                "ya29.a0AVvZVso3HX9CU8fDP0q2n1rJZNpkDo_k6xFzekF1K3MbWOg4tgqzOqVSrZBCSbb6qDbtmVpmZfu4rRFp-JRPrl_-cG-S007mXEKXgo98ZlqgpWv0JVv7Dm3DoA-xK1nwBMNcNg_4fQxKhEdIr_ZKwb3Y7kPiEFIUaCgYKAdkSAQASFQGbdwaIGGm3C3OUrl_LnYiXir-caw0167",
                "1//04XPHBxX1XljjCgYIARAAGAQSNwF-L9IrxH-c4XgiM-bqC6UTczeuuxZfyQSZ7Hk3Hj22-DOWnCg_oR9vL_j_lOszu0Wujy4yL98",
                "854009361838-jbp08qs877jefp4ecni65ft79dt7raq7.apps.googleusercontent.com",
                "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",
                "/home/nbaskey/Pictures/test_folder");

            std::vector<std::string> file_list_vec = obj.checkIfDirectoryHasJPG();
            if (file_list_vec.size() == 0)
            {
                // std::cout << "No image files present\n";
                // return 1;
                fout << ctime(&now) << " : No image files present\n";
                fout.flush();
            }
            else
            {
                std::string zip_file_name = obj.zipAndRemove(file_list_vec);
                std::cout << zip_file_name << "\n";

                std::string location_header_value = obj.get_location_header_value(zip_file_name);
                if (location_header_value.empty())
                {
                    // std::cout << "Location header value got is EMPTY....\n";
                    // return 1;

                    throw std::runtime_error("Location header value got is EMPTY");
                }

                std::string returned_response_id = obj.upload_file(location_header_value, zip_file_name);
                // std::cout << "Uploaded successfully the id we got is : " << returned_response_id << "\n";
                fout << ctime(&now) << " : Uploaded successfully the id we got is -> " << returned_response_id << "\n";
                fout.close();
            }
        }
    }
    catch (std::runtime_error &e)
    {
        fout_for_error << ctime(&now) << " : Runtime Error -> " << e.what() << "\n";
        fout_for_error.close();
    }

    return 0;
}

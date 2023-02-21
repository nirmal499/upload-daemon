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

FILE *fp;
const char pidfile[] = "/var/run/my-daemon.pid";
struct sigaction action; /* for sigaction */

void sigHandler(int sig){
    int status = 0;

    if(sig == SIGTERM || sig == SIGINT || sig == SIGQUIT || sig == SIGABRT){
        /** remove the pidfile */
        if((unlink(pidfile)) == -1){
            status = 1;
        }

        if((fclose(fp)) == EOF){
            status = 1;
        }

        exit(status);
        /** exit with the status set */
    }else{
        /* some other signal */
        exit(1);
    }
}


int main()
{
    try
    {
        pid_t pid;
        FILE *fp;
        time_t now;
        const char daemonfile[] = "/tmp/my-daemon-is-alive.txt";

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
            fpout.open(pidfile,std::ios::out);
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
        fout.open(daemonfile,std::ios::app);

        if(!fout){
            throw std::runtime_error("Can't open daemonfile");
        }

        close(STDIN_FILENO); // The stdin attached to keyboard will get closed
        close(STDOUT_FILENO); // The stdout attached to console will get closed
        close(STDERR_FILENO); // The stderr attached to console will get closed

        open("/dev/null",O_RDONLY); /** stdin; fd0 will be attached to /dev/null */
        open("/dev/null",O_WRONLY); /** stdout; fd1 will be attached to /dev/null */
        open("/dev/null",O_RDWR); /** stderr; fd2 will be attached to /dev/null */

        /** prepare for sigaction */
        action.sa_handler = sigHandler;
        sigfillset(&action.sa_mask);

        action.sa_flags = SA_RESTART;

        /** register the signals we want to handle */
        sigaction(SIGTERM,&action,NULL); // terminate
        sigaction(SIGINT,&action,NULL); // interrupt
        sigaction(SIGQUIT,&action,NULL); // Quit
        sigaction(SIGABRT,&action,NULL); // Abort

        /** here we start the daemons "work" */
        do
        {

        } while (true);

    }
    catch (std::runtime_error &e)
    {
        std::cout << "Runtime Error: " << e.what() << "\n";
    }

    return 0;
}

#include "Headers/define.h"

string filepath;
int threadnum = 0;
vector<string> files;
vector<int> threadid;
vector<bool> threadfree;
vector<struct target> threadtarget;
vector<string> threadoutfile;
vector<long> threadtime;
vector<int> threadtimes;
pthread_mutex_t mutex;
ofstream logout("log.txt");
int main(int argc, char* argv[])
{
    open("log.txt", (O_CREAT|O_WRONLY|O_TRUNC));
    for (int i = 1; i <= argc-1; i ++) {
        string s = argv[i];
        if (s == "-n") {
            filepath = argv[++ i];
        }
        else if (s == "-t") {
            threadnum = atoi(argv[++ i]);
        }
        else {
            cout << "Please use arguments -n with your file's absolute path and -t with the number of threads to init." << endl;
            return 0;
        }
    }
    for (int i = 1; i <= threadnum; i ++) {
        threadid.push_back(i);
        threadfree.push_back(true);
        struct target temptar;
        threadtarget.push_back(temptar);
        threadoutfile.push_back(filepath+"/result/");
        threadtime.push_back(time((time_t*)NULL));
        threadtimes.push_back(0);
    }
    pthread_t id[threadnum];
    readFileList((char*)filepath.c_str(), &files);
    vector<string>::iterator it;
    int busy = 0;
    for (it = files.begin(); it != files.end(); it ++) {
        ifstream in(filepath + *it);
        ofstream out(filepath + "result/" + *it);
        if (!out.is_open()) {
            string outfilepath = filepath + "result/" + *it;
            open(outfilepath.c_str(), (O_CREAT|O_WRONLY|O_TRUNC));
            ofstream(filepath + "result/" + *it);
        }
        out.close();
        while (!in.eof()) {
            int i = 0;
            char line[256] = {'\0'};
            char host[20] = {'\0'};
            struct target tar;
            tar.port = 0;
            in.getline(line, 100);
            if (line[0] == '\0') {
                break;
            }
            for (i = 0; line[i] != '\t'; i ++) {
                host[i] = line[i];
            }
            tar.host = host;
            for (i += 2; line[i] != '\0'; i ++) {
                tar.port = tar.port*10 + (line[i] - '0');
            }
            logout << tar.host << " start!!!" << endl;
            cout << tar.host << " start!!!" << endl;
            while (true) {
                int flag = 0;
                vector<int>::iterator threadidit;
                for (threadidit = threadid.begin(); threadidit != threadid.end(); threadidit ++) {
                    if (threadfree.at(*threadidit-1)) {
                        threadtarget.at(*threadidit-1) = tar;
                        threadfree.at(*threadidit-1) = false;
                        threadoutfile.at(*threadidit-1) = filepath + "result/" + *it;
                        int *paraid = (int*)malloc(sizeof(int));
                        *paraid = *threadidit;
                        threadtime.at(*threadidit-1) = time((time_t*)NULL);
                        threadtimes.at(*threadidit-1) = 0;
                        int res = pthread_create(&id[*threadidit-1], NULL, mythread, (void*)paraid);
                        if (res) {
                            string s = "create the thread " + to_string(*threadidit) + " failed!";
                            cout << s << endl;
                            logout << s << endl;
                        }
                        else {
                            flag = 1;
                            break;
                        }
                    }
                    else {
                        threadtimes.at(*threadidit-1) ++;
                        if (threadtimes.at(*threadidit-1) >= 1000*threadnum) {
                            cout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                            logout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                            pthread_cancel(id[*threadidit-1]);
                            threadfree.at(*threadidit-1) = true;
                            threadtimes.at(*threadidit-1) = 0;
                        }
                        //if (time((time_t*)NULL) - threadtime.at(*threadidit-1) > threadnum) {
                            //cout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                            //logout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                            //pthread_cancel(id[*threadidit-1]);
                            //threadfree.at(*threadidit-1) = true;
                            //threadtimes.at(*threadidit-1) = 0;
                        //}
                    }
                }
                if (flag == 1) {
                    busy = 0;
                    break;
                }
                else {
                    busy ++;
                }
                if (busy >= 1000*threadmaxruntime) {
                    busy = 0;
                    for (threadidit = threadid.begin(); threadidit != threadid.end(); threadidit ++) {
                        cout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                        logout << threadtarget.at(*threadidit-1).host << " overtime!" << endl;
                        pthread_cancel(id[*threadidit-1]);
                        threadfree.at(*threadidit-1) = true;
                        threadtimes.at(*threadidit-1) = 0;
                    }
                }
                usleep(1000);
            }
        }
        in.close();

    }
    sleep(10);
    cout << "All the threads have finished!!!" << endl;
    logout << "All the threads have finished!!!" << endl;
    logout.close();
    return 0;
}
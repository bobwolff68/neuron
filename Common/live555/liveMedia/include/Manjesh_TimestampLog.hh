#ifndef MANJESH_TIMESTAMPLOG_H_
#define MANJESH_TIMESTAMPLOG_H_

#ifndef V4RTENC_H_
#define LOG_TIMESTAMPS
#ifdef LOG_TIMESTAMPS
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

class TimestampsLog
{
private:
    int log_fd;
public:
    TimestampsLog(const char* log_file_name)
    {
	std::cout << "Trying to open log file: " << log_file_name << std::endl;
    	log_fd = open(log_file_name, O_WRONLY|O_CREAT);
    	if(log_fd == -1)
    	{
            std::cerr << "Unable to open log file " << log_file_name << std::endl;
            throw -1;
    	}
	std::cout << "Successful..." << std::endl;
    }
    ~TimestampsLog()
    {
	close(log_fd);
    }
    void WriteEntry(struct timeval* p_tod,const int frm_num,const int64_t apple_ts,const int64_t rtp_ts)
    {
	char entry[4][100];
    
    	if(p_tod != NULL)
        {
            sprintf(entry[0], "%ld.%06d,",p_tod->tv_sec,p_tod->tv_usec);
        }
    	else
    	{
            sprintf(entry[0], " ,");
    	}
    
    	if (frm_num >= 0) 
    	{
            sprintf(entry[1], "%d,", frm_num);
    	}
    	else
    	{
            sprintf(entry[1], " ,");        
    	}
    
    	if (apple_ts > 0) 
    	{
            sprintf(entry[2], "%lld,", apple_ts);
    	}
    	else
    	{
            sprintf(entry[2], " ,");        
    	}
    
    	if (rtp_ts > 0) 
    	{
            sprintf(entry[3], "%lld\n", rtp_ts);
    	}
    	else
    	{
            sprintf(entry[3], " \n");        
    	}
    
    	for (int i=0; i<4; i++) 
    	{
            write(log_fd, entry[i], strlen(entry[i]));
    	}
    }
};
#else
class TimestampsLog
{
public:
    TimestampsLog(const char* log_file_name)
    {
        (void)log_file_name;
    }
    ~TimestampsLog()
    {
        
    }
    void WriteEntry(struct timeval* p_tod,const int frm_num,const int64_t apple_ts,const int64_t rtp_ts)
    {
        (void)p_tod;
        (void)frm_num;
        (void)apple_ts;
        (void)rtp_ts;
    }
};
#endif
#endif
#endif

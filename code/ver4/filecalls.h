int send_file(int sockfd, char* file_path);
int recv_file(int sockfd, char* file_path);
extern const int BUFFER_SIZE;
extern const int MAX_FILE_SIZE_BYTES;
extern const int MAX_TRIES;

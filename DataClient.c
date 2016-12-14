#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum messageTypes { data_message = 'd', heartbeat_message = 'h' };

/* Structure of Data Message
 * | 1 Byte      | X Bytes ...|
 * | messageType | message ...|
 */
int serializeDataMessage(char* _buffer, int _buffer_length){

  // Set Message Type as Data
  _buffer[0] = data_message;

  // Read Data-File and store in Buffer
  char c;
  int i= 1;
  FILE *file;
  file = fopen("Data.txt", "r");
  if (file) {
    while ((c = getc(file)) != EOF && i < _buffer_length){
      _buffer[i++] = c;
    }
    fclose(file);
  }

  //printf("Serialized DataMessage:%s\n",_buffer);
  //printf("Serialized DataMessage Length:%d\n", i+1);

  //return Length of Serialized Messsage (Length of _message + 1 for messageType)
  return i;
}

int main(int argc, char* argv[])
{
  // Variables needed for the network / socket usage
  int32_t socket_descriptor;
  int32_t return_code;

  int32_t port = 4000;

  struct sockaddr_in remote_address;
  struct hostent* host;

  int32_t bufferLength = 32000;
  char buffer[bufferLength];

  // Get server IP address (no check if input is IP address or DNS name)
  host = gethostbyname("127.0.0.1");
  if (host == NULL)
  {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    return(-1);
  }

  printf("Sending data to '%s:%d' (IP: %s) \n", host->h_name,
      port, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));

  remote_address.sin_family = host->h_addrtype;
  memcpy((char*) &remote_address.sin_addr.s_addr,
      host->h_addr_list[0], host->h_length);
  remote_address.sin_port = htons(port);

  // Socket creation
  socket_descriptor = (int32_t)socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_descriptor < 0)
  {
    printf("%s: cannot open socket \n", argv[0]);
    return(-1);
  }

  // Send Data Message
  int32_t bytes_to_send = serializeDataMessage(buffer,bufferLength);
  return_code = sendto(socket_descriptor, buffer,
      bytes_to_send, 0, (struct sockaddr*) &remote_address,
      sizeof(remote_address));
  if (return_code < 0)
  {
    printf("%s: cannot send data message.\n", argv[0]);
    return(-1);
  }
  free(host);
  // No response from Data Message needed.
  return 0;
}

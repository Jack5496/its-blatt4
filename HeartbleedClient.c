#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h> /* Damit ich Signale abfangen kann */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum messageTypes { data_message = 'd', heartbeat_message = 'h' };
struct hostent* host;

/**
* Letzer aufruf um alles wichtige zu schließen
*/
void last_wish(int i){
  if(host!=NULL){
   free(host); 
  }
  printf("\nManuelles Beenden\n");
  exit(1);
}


/* Structure of Heartbeat Message
 * | 1 Byte      | 4 Byte                   | requested_message_length Bytes ...|
 * | messageType | requested_message_length | message                        ...|
 */
int serializeHeartbeatMessage(char* _buffer, int32_t _requested_message_length, char* _message){

  // Set Message Type as Heartbeat
  _buffer[0] = heartbeat_message;

  int extra_length = 1337;
  *(int*)(_buffer+1)=_requested_message_length+extra_length;
  int fixed_header_length = 5;

  int32_t i = fixed_header_length;
  memcpy(_buffer+i,_message,_requested_message_length);
  printf("Serialized HeartbeatMessage, %d Bytes with fixed header, %ld Bytes variable message.\n",i,strlen(_message));
  return fixed_header_length + _requested_message_length;
}


int main(int argc, char* argv[])
{
  //Handlet aktivierung für STRG+C
  //http://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event-c
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = last_wish;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0; //setze sa flags 0

  sigaction(SIGINT, &sigIntHandler, NULL);
  // Ende für STRG+C
  
  
  
  // Variables needed for the network / socket usage
  int32_t socket_descriptor;
  int32_t return_code;


  int32_t port = 4000;

  struct sockaddr_in remote_address;
  socklen_t remote_address_size;

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

  // Send Heartbeat Message
  char* heartbeat_message = "IamAlive";
  int32_t heartbeat_length = strlen(heartbeat_message);
  int bytes_to_send = serializeHeartbeatMessage(buffer, heartbeat_length, heartbeat_message);

  return_code = sendto(socket_descriptor, buffer,
      bytes_to_send, 0, (struct sockaddr*) &remote_address,
      sizeof(remote_address));
  if (return_code < 0)
  {
    printf("%s: cannot send heartbeat message.\n", argv[0]);
    return(-1);
  }
  // Receive Heartbeat Echo
  remote_address_size = sizeof(remote_address);
  int bytes_received = recvfrom(
      socket_descriptor, buffer, bufferLength, 0,
      (struct sockaddr*) &remote_address, &remote_address_size);
  if(bytes_received < 0){
    printf("%s: cannot receive heartbeat echo.\n", argv[0]);
    free(host);
    return(-1);
  }
  printf("Echoed Heartbeat, %d bytes:\n",bytes_received);
  for(int i =0; i<bytes_received; i++){
    printf("%c", buffer[i]);
  }printf("\n");

  free(host);
  return 0;
}

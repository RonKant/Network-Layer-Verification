#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // TODO: implement by ourselves
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../ip.h"
#include "network_manager.h"

#define TERMINATE_FIFO_PREFIX "vnetwork_terminate_"
#define IN_PACKET_FIFO_PREFIX "vnetwork_in_packets_"
#define BIND_REQUEST_FIFO_PREFIX "vnetwork_bind_requests_"
#define CONNECT_REQUEST_FIFO_PREFIX "vnetwork_connect_requests_"

#define MAX_SOCKETS 32768
#define DEFAULT_FIFO_MODE 0666

#define MAX_HANDLES_PER_EPOCH 10

const char* ip_str_repr = "%01X%01X%02X%04X%04X%04X%02X%02X%04X%08s%08s%";

// Use tomer's version in ip.c later
IPPacket str_to_ip(char* s) {
    IPPacket result = (IPPacket)malloc(sizeof(*result));
    int scanned = sscanf(s, ip_str_repr,
                         &(result->version),
                         &(result->ihl),
                         &(result->dscp_and_ecn),
                         &(result->total_length),
                         &(result->id),
                         &(result->flags_and_offset),
                         &(result->ttl),
                         &(result->protocol),
                         &(result->header_checksum),
                         &(result->src_ip),
                         &(result->dst_ip));

    if (scanned != 11) { // if scan failed -- error
        free(result);
        return NULL;
    }
    char* s_data = s + (result->ihl * 32);      //s_data is the pointer of the data section in s. ihl is the length of the header
    result->data = (char*)malloc(strlen(s_data) + 1);

    if (result -> data == NULL) {
        free(result);
        return NULL;
    }

    strcpy(result->data, s_data);

    return result;
}

/******************************************
 * Utility Functions For fifo creation
 * ****************************************/

/**
 * Returns a new string, containing the concatenation of the two strings.
 * Used in translating ip/ports to fifo names.
 */
char* add_as_prefix(const char* prefix, const char* s) {
    char* result = (char*)malloc(strlen(s) + strlen(prefix) + 1);
    if (result == NULL) return NULL;
    result[0] = '\0';

    result = strcat(result, prefix);
    result = strcat(result, s);

    return result;
}

char* get_terminate_fifo_name(const char* ip) {
    return add_as_prefix(TERMINATE_FIFO_PREFIX, ip);
}

char* get_in_packet_fifo_name(const char* ip) {
    return add_as_prefix(IN_PACKET_FIFO_PREFIX, ip);
}

char* get_bind_requests_fifo_name(const char* ip) {
    return add_as_prefix(BIND_REQUEST_FIFO_PREFIX, ip);
}

char* get_connect_requests_fifo_name(const char* ip) {
    return add_as_prefix(CONNECT_REQUEST_FIFO_PREFIX, ip);
}

int initialize_network_manager_fifos(NetworkManager manager) {
    manager->terminate_fifo_name = get_terminate_fifo_name(manager->ip);
    manager->in_packet_fifo_name = get_in_packet_fifo_name(manager->ip);
    manager->bind_request_fifo_name = get_bind_requests_fifo_name(manager->ip);
    manager->connect_request_fifo_name = get_connect_requests_fifo_name(manager->ip);

    if (manager->terminate_fifo_name == NULL
        || manager->in_packet_fifo_name == NULL
        || manager->in_packet_fifo_name == NULL
        || manager->connect_request_fifo_name == NULL) {
            return -1;
        }
    if (mkfifo(manager->terminate_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->in_packet_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->bind_request_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->connect_request_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            printf("Failed creating manager fifos (probably already exist).\n");
            return -1;
        }

    return 0;
}

/******************************************
 * Utility Functions For manager loop
 * ****************************************/

/**
 * Reads from fd into buf.
 * If fd is empty - returns 0.
 * If fd contains anything - will block and read into buf until entire len has been reached.
 * On success - returns 0 or length. On failre returns -1.
 */
int read_entire_message(int fd, char* buf, int len) {
    if (len < 0) {
        printf("Invalid length provided - must be non-negative.\n");
        return -1;
    }
    int bytes_left = len;
    int read_result = read(fd, buf, len);
    if (read_result == 0) {
        return 0; // fd empty.
    }

    if (read_result == -1) {
        printf("An error has occurred while reading from file.\n");
        return -1;
    }

    bytes_left -= read_result;
    while (bytes_left > 0) {
        
        read_result = read(fd, ((char*)buf) + len - bytes_left, bytes_left);
        if (read_result == -1) {
            printf("An error has occurred while reading from file.\n");
            return -1;
        }

        bytes_left -= read_result;
    }

    return len;
}

/**
 * Like read_entire_message, but will not return 0 (will keep blocking).
 */
int read_nonzero_entire_message(int fd, char* buf, int len) {
    while (1) {
        int read_result = read_entire_message(fd, buf, len);
        if (read_result == -1) {
            return -1;
        }
        if (read_result > 0) {
            return read_result;
        }
    }
}

/**
 * With having already read version and header size, completes the reading of a given ip packet.
 * Returns NULL on error.
 */
IPPacket read_ip_packet_from_stream(int out_packet_fd, char version, char header_size) {
    int header_size_bytes = 4 * ((int)header_size);
    char* header = (char*)malloc(header_size_bytes);
    if (header == NULL) {
        printf("Failed allocating ip header of size %d.\n", header_size_bytes);
        return NULL;
    }

    // try to read header
    header[0] = (version << 4) | header_size;
    int read_result = read_nonzero_entire_message(out_packet_fd, header + 1, header_size - 1);
    if (read_result == -1) {
        printf("Failed reading header from fifo.\n");
        free(header);
        return NULL;
    }

    int total_packet_length = (((int)header[2]) << 8) + ((int)header[3]);

    char* ip_packet_raw = (char*)malloc(total_packet_length);
    if (ip_packet_raw == NULL) {
        printf("Failed allocating ip packet of size %d.\n", total_packet_length);
        free(header);
        return NULL;
    }

    if (memcpy(ip_packet_raw, header, header_size_bytes) == NULL) {
        printf("Failed copying memory header into ip packet raw.\n");
        free(header);
        free(ip_packet_raw);
        return NULL;
    }

    // try to read entire packet
    read_result = read_nonzero_entire_message(
        out_packet_fd, ip_packet_raw + header_size_bytes,
        total_packet_length - header_size_bytes);

    if (read_result == -1) {
        printf("Failed reading ip packet of size %d.\n", total_packet_length);
        free(header);
        free(ip_packet_raw);
        return NULL;
    }

    IPPacket packet = str_to_ip(ip_packet_raw);
    free(header);
    free(ip_packet_raw);
    if (packet == NULL) {
        printf("Failed to convert raw pakcet.\n");
        return NULL;
    }

    return packet;
}

int handle_incoming_ip_packet(IPPacket packet) {
    // if not meant for self - decrease TTL, maybe discard, maybe send onward.

    // otherwise, check other IP fields, then strip IP header, check tcp header.

    // if socket destination exists - pass the message to socket anc handle with already existing functions.
    // otherwise - ? (discard? send something back?)
    return 0; // mock
}

int send_string(char* data_to_send) {
    return 0; //mock
}

/******************************************
 * Stages of manager loop
 * ****************************************/

/**
 * Goes over the incoming packets, handling them with the corresponding sockets.
 * 0 on success, otherwise - failure.
 */
int handle_in_packets_fifo(NetworkManager manager) {
    char* fifo_name = manager->in_packet_fifo_name;
    if (fifo_name == NULL) {
        printf("Error: outgoing packets fifo is NULL.\n");
        return -1;
    }

    int in_packet_fd = open(fifo_name, O_RDONLY);
    if (in_packet_fd == -1) {
        printf("Error: failed to open outgoing packets fifo (path: %s).\n", fifo_name);
        return -1;
    }

    for (int i = 0; i < MAX_HANDLES_PER_EPOCH; ++i) {
        char version_and_header_size[1];
        int read_result = read_entire_message(in_packet_fd, version_and_header_size, 1);

        if (read_result == -1) {
            printf("Error while trying to read packet from outgoing fifo.\n");
            return -1;
        }
        if (read_result == 0) {
            break; // no more messages to read in this epoch.
        }

        char version = (version_and_header_size[1] >> 4);
        char header_size = (version_and_header_size[1] << 4) >> 4;

        IPPacket packet = read_ip_packet_from_stream(in_packet_fd, version, header_size);
        if (packet == NULL) {
            return -1;
        }

        if (handle_incoming_ip_packet(packet) != 0) {
            printf("Error sending packet.\n");
            free(packet);
            return -1;
        }
        
        free(packet);
    }

    if (close(in_packet_fd) != 0) {
        printf("Failed to close outgoing packets fifo.\n");
        return -1;
    }
    return 0;
}

/**
 * Go over outgoing messages (not packets) fifo and send each to it's target.
 */
int handle_out_requests_fifo(NetworkManager manager) {
    return 0; // mock
}

/******************************************
 * Interface
 * ****************************************/

NetworkManager createNetworkManager(const char* ip) {
    NetworkManager manager = (NetworkManager)malloc(sizeof(*manager));
    if (manager == NULL) return NULL;

    // so we can destroy them later if something fails.
    manager->terminate_fifo_name = NULL;
    manager->in_packet_fifo_name = NULL;
    manager->bind_request_fifo_name = NULL;
    manager->connect_request_fifo_name = NULL;
    manager->sockets = NULL;

    manager->ip = ip;

    if (initialize_network_manager_fifos(manager) != 0) {
            destroyNetworkManager(manager);
            return NULL;
        }

    manager->sockets = createHashMap(MAX_SOCKETS);
    if (manager->sockets == NULL) {
        destroyNetworkManager(manager);
        return NULL;
    }
    return manager;
}


void destroyNetworkManager(NetworkManager manager) {
    // should: free all memory in struct, unlink all fifos (?)
    // if clients exist - notify them about shutdown?
    hashDestroy(manager->sockets, NULL);
    unlink(manager->terminate_fifo_name);
    unlink(manager->in_packet_fifo_name);
    unlink(manager->bind_request_fifo_name);
    unlink(manager->connect_request_fifo_name);

    free(manager->terminate_fifo_name);
    free(manager->in_packet_fifo_name);
    free(manager->bind_request_fifo_name);
    free(manager->connect_request_fifo_name);

    free(manager);

    printf("Manager memory freed.\n");
}

int managerLoop(NetworkManager manager) { 
    while (1) {
        
        // go over bind requests fifo, handle them

        // go over connect requests fifo, handle them
        
        if (handle_in_packets_fifo(manager) != 0) {
            return -1;
        }


        // go over socket hashmap, for every socket check out fifo, send message to destination.
        // this ^ should probably combine in the same iteration loop with V.
        // go over socket hasmap, for every socket check request fifo, handle them
        // also, for every socket who has not received ack - try to resend message if enough time passed
    }
}

int stopManager(char* ip);
#include "ModemDetect.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int main() {

    usbColdplugDetect();
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, hotplugEventDetect, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread_id, NULL);

    return 0;
}
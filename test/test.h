#include <unistd.h>
#include <ucontext.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <signal.h>
//#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/if_link.h>
#include <type_traits>
#include <vector>
#include <map>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include "../seaice/iomanager.h"
#include "../seaice/address.h"
#include "../seaice/timer.h"
#include "../seaice/hook.h"
#include "ipcdefv01.h"
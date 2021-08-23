# BSAT Example Server

This is a simple example which illustrates libbsat's usage for idle timeouts.

> :information_source: Documentation is generated using
> [pomd4c](https://github.com/andrew-canaday/pomd4c).
>
> If you're viewing this source as markdown, you can see the the source
> code by clicking the "►" next to `View Source` in each section.


## Overview

The gist is: we spin up a basic TCP echo server which just `write()`'s back
whatever it `read()`'s from a client. It uses libev for readiness
notifications and libbsat for idle timeouts.

> :information_source: **NOTE**: the general pattern here is as follows:
> - The _server_ gets a `bsat_toq_t` to monitor a set of timeouts that all
>   share a common timeout delta and callback.
> - _Each client_ gets a `bsat_timeout_t` which are used to start, stop,
>   and reset the client's idle disconnect timer.
>
> Afterwards:
> - When a client connects: `bsat_timeout_start` it's idle timeout.
> - When we read from/write to the client: `bsat_timeout_reset` it.
> - If the idle timeout fires for a client: close it!

For more details, see **[the walkthrough](#walkthrough)**.

## Building and Usage
You should be able to build it like so:

```bash
# from your build directory:
make -C example bsat_example

# run it:
./example/bsat_example
```

To interact with it, simply `telnet 127.0.0.1 8080`, type some stuff, hit
enter, see it come back, and wait a few seconds to see the idle disconnect
in action:

```bash
telnet 127.0.0.1 8080
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Hello, world!
Hello, world!
Connection closed by foreign host.
```

On the server side, you should see:

```
BSAT Example server for 0.0.0
Accepted new connection from client 1!
Client 1 says: "Hello, world!
"
Successfully sent 15 bytes to client 1!
Client 1 has been idle for 5.0 seconds. Closing!
```



---

## Walkthrough



### Define types for our client and server.

Here, we create two types — one for the server and one for the client:

1. `my_server_t`, has an `ev_io_watcher` for the server `listen()/accept()`
    fd and a `bsat_toq_t` to manage timeouts for all client connections.
2. `my_client_t`, has an `ev_io_watcher` for the connection `read()` fd
    and a `bsat_timeout_t` to mange the timeout for a single connection

<details><summary>View Source</summary>


We'll just use a rolling counter for client ID. 

```C
typedef size_t my_id_t;
```


Struct type we'll use to store information about the server. 

```C
typedef struct my_server {
    struct ev_loop*  loop;
    struct ev_io     w_accept;
    bsat_toq_t       idle_toq;
    char             recv_buf[RECV_BUFFER_SIZE];
} my_server_t;
```


Struct type we'll use to store per-connection client information. 

```C
typedef struct my_client {
    int             fd;
    my_id_t         id;
    ev_io           w_read;
    bsat_timeout_t  idle_timeout;
    my_server_t*    server;
} my_client_t;
```


</details> 


### The Server

Here, we initialize an `ev_io_watcher` for the listen file descriptor and a
`bsat_toq_t` to store client timeouts.

There are two callbacks we care about:
 - `accept_cb`: invoked by the `ev_io_watcher` when we get a new connection.
 - `idle_cb`: invoked by the `bsat_toq_t` when a connection has timed out.


The relevant bit here is initializing the timeout queue (a.k.a. "toq"):

```C
bsat_toq_init(loop, &(server->idle_toq), idle_cb, IDLE_TIMEOUT);
```

> **NOTE**: we also store the server in the EV watcher and BSAT toq so we
> can get a pointer to the data structure in the callbacks!
>
> ```C
> server->idle_toq.data = server;
> ```

<details><summary>View Source</summary>

```C
static void my_server_init(
        struct ev_loop* loop,
        my_server_t* server,
        int listen_fd)
{
    server->loop = loop;

    /* Listen for read events and invoked accept_cb when we get one. */
    ev_io_init(&(server->w_accept), accept_cb, listen_fd, EV_READ);
    server->w_accept.data = server;
    ev_io_start(loop, &server->w_accept);

    /* Initialize the bsat timeout queue: */
    bsat_toq_init(loop, &(server->idle_toq), idle_cb, IDLE_TIMEOUT);
    server->idle_toq.data = server;
};
```


</details> 


#### accept_cb

When the server `accept()` fd watcher fires:

    1. Create a new client
    2. Initialize and start the read ev_io_watcher on the client fd.
    3. Initialize and start the idle connection timer for this client.

The relevant bit here is the timeout initialization and start:

```C
bsat_timeout_init(&(client->idle_timeout));
bsat_timeout_start(&(server->idle_toq), &(client->idle_timeout));
```

> **NOTE**: we also store the client in the EV watcher and BSAT timeout so
> we can get a pointer to the data structure in the callbacks!
>
> ```C
> client->idle_timeout.data = client;
> client->w_read.data = client;
> ```

<details><summary>View Source</summary>

```C
static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;
    my_client_t* client = NULL;
    my_server_t* server = watcher->data;

    if(EV_ERROR & revents) {
        /* In practice, this shouldn't happen. It usually indicates a bug: */
        return;
    }

    errno = 0;
    client_fd = accept(
            watcher->fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_fd < 0) {
        perror("Error accepting client connection.");
        return;
    }

    /* Create a client: */
    client = malloc(sizeof(my_client_t));
    if( client ) {
        client->server = server;
        client->id = next_id++;
        client->fd = client_fd;
    } else {
        perror("Error allocating my_client_t.");
        return;
    }

    /* Now, initialize and start the client idle timeout: */
    bsat_timeout_init(&(client->idle_timeout));
    bsat_timeout_start(&(server->idle_toq), &(client->idle_timeout));

    /* Watch for read readiness on the client socket: */
    ev_io_init(&client->w_read, read_cb, client_fd, EV_READ);
    ev_io_start(loop, &client->w_read);

    /* Associate our client data with both the ev watcher and bsat timeout: */
    client->idle_timeout.data = client;
    client->w_read.data = client;

    printf("Accepted new connection from client %zu!\n", client->id);
    return;
};
```


</details> 


#### idle_cb

The idle timeout just invokes `ev_io_stop` on the client read watcher and
closes and frees the connection.

<details><summary>View Source</summary>

```C
static void idle_cb(bsat_toq_t* toq, bsat_timeout_t* timeout)
{
    my_server_t* server = toq->data;
    my_client_t* client = timeout->data;
    printf("Client %zu has been idle for %0.1f seconds. Closing!\n",
        client->id, IDLE_TIMEOUT);

    ev_io_stop(server->loop, &client->w_read);
    close(client->fd);
    free(client);
    return;
};
```


</details> 


### The Client

The client has a single callback, `read_cb`, which is invoked when we get
data on the clients socket.


#### read_cb

When the `read()` watcher fires:

    1. Reset the timeout associated with the client.
    2. Echo the received payload back to the client.

The most relevant bit here is:

```C
bsat_timeout_reset(&server->idle_toq, &client->idle_timeout);
```

**NOTE**: If the client disconnects, we stop the associated `bsat_timeout_t`,
like so:

```C
bsat_timeout_stop(&server->idle_toq, &client->idle_timeout);
```

<details><summary>View Source</summary>

```C
static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[RECV_BUFFER_SIZE];
    ssize_t no_recv;
    ssize_t no_sent;

    /* Get the pointer to our client object out of the watcher data: */
    my_client_t* client = watcher->data;
    my_server_t* server = client->server;

    if(EV_ERROR & revents) {
        /* In practice, this shouldn't happen. It usually indicates a bug: */
        return;
    }

    /* Since we got data from the client, reset the idle timeout: */
    bsat_timeout_reset(&server->idle_toq, &client->idle_timeout);

    errno = 0;
    no_recv = recv(watcher->fd, buffer, RECV_BUFFER_SIZE, 0);
    if(no_recv < 0) {
        /* Log read errors and move on. */
        int client_err = errno;
        fprintf(stderr, "Socket read error for client %zu: %s\n",
            client->id, strerror(client_err));
        return;
    }

    /* If the client closed, stop the IO watcher and idle timeout: */
    if(no_recv == 0) {
        printf("Got close from client: %zu\n", client->id);
        ev_io_stop(loop, &client->w_read);
        bsat_timeout_stop(&server->idle_toq, &client->idle_timeout);
        free(client);
        return;
    }
    else {
        printf("Client %zu says: \"%*s\"\n", client->id, (int)no_recv, buffer);
    }

    /* This is a demo, so we just try to write immediately.
     * (Normally, you might ensure all your sockets are nonblocking, attempt
     * to write, buffer whatever wasn't sent, and start an ev_io_watcher for
     * write readiness events on the client socket.
     */
    no_sent = send(watcher->fd, buffer, no_recv, 0);
    if( no_sent >= 0 ) {
        printf("Successfully sent %li bytes to client %zu!\n",
            no_sent, client->id);
    }
    else {
        int client_err = errno;
        fprintf(stderr, "Error sending data to client %zu: %s!\n",
            client->id, strerror(client_err));
    };
};
```


</details> 


### Entrypoint

All we do here is:

    1. Get a hold of the libev default loop (ev_default_loop()).
    2. Create a listening socket on LISTEN_PORT.
    3. Initialize our my_server_t.
    4. Invoke ev_run() to get things rolling.

<details><summary>View Source</summary>

```C
int main(int argc, char** argv)
{
    printf("BSAT Example server for %s\n", BSAT_VERSION_STR);
    struct ev_loop* loop = ev_default_loop(0);

    /* Create a new accept socket, bound to our listening port. */
    int listen_fd = bind_and_listen(LISTEN_PORT);
    if( listen_fd < 0 ) {
        return -1;
    }

    /* Initialize and start server: */
    my_server_t server;
    my_server_init(loop, &server, listen_fd);

    ev_run(loop, 0);
    return 0;
};
```


</details> 


The actual port binding/socket creation is pretty routine:

<details><summary>View Source</summary>

```C
static int bind_and_listen(uint16_t listen_port)
{
    int listen_fd;
    struct sockaddr_in listen_addr;
    size_t len = sizeof(listen_addr);

    /* Create the socket. */
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if( listen_fd < 0 ) {
        perror("Unable to create listen socket");
        return -1;
    }

    /* Bind to the port. */
    memset(&listen_addr, 0, len);
    listen_addr.sin_port = htons(LISTEN_PORT);
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(listen_fd, (struct sockaddr*)&listen_addr, len) ) {
        fprintf(stderr, "Unable to bind to port %i: %s\n",
                LISTEN_PORT, strerror(errno));
        return -1;
    }

    /* Wait for incoming connections. */
    if( listen(listen_fd, LISTEN_BACKLOG) < 0 ) {
        fprintf(stderr, "Unable to listen on port %i: %s\n",
                LISTEN_PORT, strerror(errno));
        return -1;
    }

    return listen_fd;
};
```


</details> 



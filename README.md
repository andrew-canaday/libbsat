# libbsat

This is a small C utility library which provides timeout management for
[libev](http://software.schmorp.de/pkg/libev.html)-based projects.

It implements
[be smart about timeouts](http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts)
strategy `#4` — which allows you to manage an arbitrary number of timeouts in
`O(1)` time.

This makes it easy to implement things like idle disconnect and graceful
shutdown timeouts, regardless of workload concurrency.

## License

It's licensed under the [MIT License](./COPYING).<sup><b>1</b></sup>

## How does it work?

This library is used to manage _sets_ of timeouts which share a common timeout
_delta_.

There are two basic types, `bsat_toq_t` (a timeout queue) and `bsat_timeout_t`
(an individual timeout item).

A server get's one timeout queue (`bsat_toq_t`), which has an associated timeout
delta and callback function.

Individual connections have a _timeout_ (`bsat_timeout_t`) which is started on
connect, reset whenever there is any activity, and stopped on disconnect.

The timeout callback will be invoked for any connections which have timeouts
that haven't been reset within the specified timeout period.


### See Also
 - [The example documentation](./example/README.md)
 - [The api reference](./API.md)
 - [libev POD: Be Smart About Timeouts](http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts)


## Building

This project uses:
 - [autoconf](https://www.gnu.org/software/autoconf/) for configuration
 - [automake](https://www.gnu.org/software/automake/) for makefile generation
 - [libtool](https://www.gnu.org/software/libtool/) to make linking easier
 - [pomd4c](https://github.com/andrew-canaday/pomd4c) for documentation generation
 - [ymo_assert](https://github.com/andrew-canaday/ymo_assert) (included here) for the check targets

> **NOTE**: the whole library is one `.c` file and one `.h` file. It should be
> pretty trivial to integrate it into a different build system or statically
> compile it into another program or library.

Configuration, build, and installation follows the classic pattern:

```bash
# NOTE: this assumes you're in the source root directory.

# Generate the configure script and ancillary build files:
./autogen.sh

# Create a build directory and cd into it:
mkdir -p ./build ; cd ./build

# configure, make, make check, and make install!
../configure --prefix=/my/installation/prefix \
    && make \
    && make check \
    && make install
```

### Example Server
The [example server](./example) is an automake "extra" target. You can build it like so:

```bash
# NOTE: assumes you are in the "build" directory above.

# Build it:
make -C ./example bsat_example

# Run it:
./example/bsat_example
```

---

<sub><b>1</b> "Wait a minute! Aren't you one of those GPL nuts?"<br />Yes, but this library is <i>very</i> small and it's just a naive implementation of the strategy documented in the link above.</sub>

# API Ref: libbsat 


## Library Info 


Release version as a 32-bit unsigned integer 

```C
#define BSAT_VERSION @BSAT_VERSION@
```


Release version as a string. 

```C
extern const char* BSAT_VERSION_STR;
```


## Types 


### bsat_toq_t

Timeout queue — a set of items sharing a common timeout DELTA.

> **NOTE**: this structure has a `void* data` member which you can set
> _after_ `bsat_toq_init` in order to associate program data with a given
> timeout queue in a way that is accessible during the callback invocation.

```C
typedef struct bsat_toq bsat_toq_t;
```


### bsat_timeout_t

An individual item in a timeout set

> **NOTE**: this structure has a `void* data` member which you can set
> _after_ `bsat_timeout_init` in order to associate program data with a given
> timeout in a way that is accessible during the callback invocation.

```C
typedef struct bsat_timeout bsat_timeout_t;
```


### bsat_callback_t

Callback type used when an individual item in a set times out.

> **NOTE**: once the callback has been invoked for a given item, it _will not
> be invoked again, unless you call `bsat_timeout_reset` to reintroduce the
> item into the timeout set._

```C
typedef void (*bsat_callback_t)(bsat_toq_t* toq, bsat_timeout_t* item);
```


## Timeout Queue Functions 


### bsat_toq_init

Initialize a timeout queue.

- `loop` (if EV_MULTIPLICITY is set) libev loop
- `cb` the callback invoked when the timeout fires
- `after` the timeout period, in seconds (ev-style)
- `pool_size` the number of timeouts to preallocate
- `data` optional user data to associate with this timeout queue

Returns a new `bsat_toq_t*` on success; `NULL` (with `errno` set) on failure.

```C
void bsat_toq_init(EV_P_ bsat_toq_t* toq, bsat_callback_t cb, ev_tstamp after);
```


### bsat_toq_stop

Stop a timeout queue.

> **NOTE**: Active items remain in the queue after stop _with their original
> deadlines intact!_

```C
void bsat_toq_stop(bsat_toq_t* toq);
```


## Timeout Functions 


### bsat_timeout_init

Initialize a timeout item.

```C
void bsat_timeout_init(bsat_timeout_t* item);
```


### bsat_timeout_start

Start a timeout item. Absent any calls to `bsat_timeout_reset` in the
interim, this means that the `bsat_callback_t` registered with `toq`
will be invoked for this timeout in `ev_now()` + `after` seconds.

(See `bsat_toq_init` for details)

```C
void bsat_timeout_start(bsat_toq_t* toq, bsat_timeout_t* item);
```


### bsat_timeout_reset

Reset a timeout — i.e. it didn't time out, so restart the counter as if it
had been started right `ev_now()`.

```C
void bsat_timeout_reset(bsat_toq_t* toq, bsat_timeout_t* item);
```


### bsat_timeout_stop

Cancel a timeout item — i.e. unschedule it for execution.

> **NOTE**: after invoking `bsat_timeout_stop`, the given `toq` no longer
> keeps track of the timeout item until it's reset.

```C
void bsat_timeout_stop(bsat_toq_t* toq, bsat_timeout_t* item);
```


### bsat_timeout_is_active

Returns 1 if the timeout is active; 0 otherwise.

> **NOTE**: You usually _don't need to worry about this_; it's completely
> safe to stop a stopped timeout or start a started timeout — both operations
> are no-ops.

```C
int bsat_timeout_is_active(bsat_timeout_t* item);
```



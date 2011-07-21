

===============================================
Broccoli: The Bro Client Communications Library
===============================================

.. image:: images/logo.jpg
   :alt: Broccoli Logo

This is documentation for release *1.5.3*
of Broccoli, compatible with Bro IDS releases of *1.5*
or newer. Broccoli is free software under terms of the BSD license as given
in the `License <http://www.icir.org/christian/broccoli/manual/a3638.html#LICENSE>`_
section. This documentation is always available on the web for download
and online browsing at
http://www.icir.org/christian/broccoli.

Feedback, patches and bug reports are all welcome, please
`join the Bro mailing list <http://mailman.icsi.berkeley.edu/mailman/listinfo/bro>`_.

Yet Another SRG/ICIR Production --
http://www.cl.cam.ac.uk/Research/SRG --
http://www.icir.org

Introduction
############

Welcome! You're looking at the Broccoli manual. Thanks for reading this.

What is Broccoli?
=================

Broccoli is the BRO Client
COmmunications LIbrary. It allows
you to write applications that speak the communication protocol of the
`Bro intrusion detection system <http://www.bro-ids.org>`_.
In this document, we assume that you are familiar with the basic concepts
of Bro. If you need a refresher, please have a look at the
`original paper <http://www.icir.org/vern/papers/bro-CN99.html>`_,
the `user manual <http://www.bro-ids.org/Bro-user-manual/index.html>`_,
and the material on `bro-ids.org <http://www.bro-ids.org>`_ in general.

Why do I care?
==============

Having a single IDS on your network is good, but things become a lot more interesting
when you can communicate information among multiple vantage points in your network.
Bro agents can communicate with other Bro agents, sending and receiving events and
other state information. In the Bro context this is particularly interesting because
it means that you can build sophisticated policy-controlled distributed event
management systems.

Broccoli enters the picture when it comes to integrating components that are not
Bro agents themselves. Broccoli lets you create applications that can speak the Bro
communication protocol. You can compose, send, request, and receive events.
You can register your own event handlers. You can talk to other Broccoli
applications or Bro agents -- Bro agents cannot tell whether they are talking
to another Bro or a Broccoli application. Broccoli allows you to integrate applications
of your choosing into a distributed policy-controlled event management system.
Broccoli is intended to be portable: it should build on Linux, the BSDs, Solaris, and Windows
(in the `MinGW <http://www.mingw.org>`_ environment).

Unlike other distributed IDSs, Bro does not assume a strict sensor-manager
hierarchy in the information flow. Instead, Bro agents can request delivery of
arbitrary *events*
from other instances. When an event is triggered in a Bro agent, it checks
whether any connected agents have requested notification of this event,
and send a *copy* of the event, including the *event arguments*.
Recall that in Bro, an event handler is essentially a function defined in
the Bro language, and an event materializes through invocation of an event handler.
Each remote agent can define its own event handlers.

Broccoli applications will typically do one or more of the following:

- *Configuration/Management Tasks:* the Broccoli application
  is used to configure remotely running Bros without the need for a restart.

- *Interfacing other Systems:* the Broccoli application
  is used to convert Bro events to other alert/notice formats, for into
  syslogd entries.

- *Host-based Sensor Feeds into Bro:* the Broccoli
  application reports events based on host-based activity generated in
  kernel space or user space applications.

Installing Broccoli
###################

The installation process will hopefully be painless: Broccoli is installed from source
using the usual **./configure <options> && make && make install**
routine after extraction of the tarball. Or if you're on a Linux systems supporting
RPMs, we provide those as well.

The relevant configuration options to pass to configure are:

- **--prefix=<DIR>**: sets the installation root to DIR.
  The default is to install below ``/usr/local``.

- **--enable-debug**: enables debugging output.
  Please refer to the `Broccoli debugging <http://www.icir.org/christian/broccoli/manual/c84.html#AEN819>`_
  section for details on configuring and using debugging output.

- **--with-configfile=<FILE>**: use FILE as location of configuration file.
  See the section on `configuration files <http://www.icir.org/christian/broccoli/manual/c84.html#AEN665>`_
  below for more on this.

- **--with-openssl=<DIR>**: use the OpenSSL installation below DIR.

- **--with-kerberos=<DIR>**: use the Kerberos installation below DIR.

After installation, you'll find the library in shared and static versions in ``<prefix>/lib``
the header file for compilation in ``<prefix>/include``, and the
manual in HTML below ``<prefix>/share/gtk-doc/html/broccoli``.

When installing from source, you can get rid of the installation using **make uninstall**.

Using Broccoli
##############

Obtaining information about your build using ``broccoli-config``
====================================================================

Similarly to many other software packages, the Broccoli distribution
provides a script that you can use to obtain details about your
Broccoli setup. The script currently provides the following flags:

- **--build** prints the name of the machine the build was
  made on, when, and whether debugging support was enabled or not.

- **--prefix** prints the directory in the filesystem
  below which Broccoli was installed.

- **--version** prints the version of the distribution
  you have installed.

- **--libs** prints the flags to pass to the
  linker in order to link in the Broccoli library.

- **--cflags** prints the flags to pass to the
  compiler in order to properly include Broccoli's header file.

- **--config** prints the location of the system-wide
  config file your installation will use.

The ``--cflags`` and ``--libs`` flags
are the suggested way of obtaining the necessary information for integrating
Broccoli into your build environment. It is generally recommended to use
``broccoli-config`` for this purpose, rather than, say,
develop new **autoconf** tests.
If you use the **autoconf/automake**
tools, we recommend something along the following lines for your
``configure`` script:

::

	dnl ##################################################
	dnl # Check for Broccoli
	dnl ##################################################
	AC_ARG_WITH(broccoli-config,
		AC_HELP_STRING(\[--with-broccoli-config=FILE], \[Use given broccoli-config]),
		[ brocfg="$withval" ],
		[ AC_PATH_GENERIC(broccoli,,
			brocfg="broccoli-config",
			AC_MSG_ERROR(Cannot find Broccoli: Is broccoli-config in path? Use more fertilizer?)) ])
  
	broccoli_libs=`$brocfg --libs`
	broccoli_cflags=`$brocfg --cflags`
	AC_SUBST(broccoli_libs)
	AC_SUBST(broccoli_cflags)``

You can then use the compiler/linker flags in your Makefile.in/ams by
substituting in the values accordingly, which might look as follows:

::

	CFLAGS = -W -Wall -g -DFOOBAR @broccoli_cflags@
	LDFLAGS = -L/usr/lib/foobar @broccoli_libs@

Suggestions for instrumenting applications
==========================================

Often you will want to make existing applications Bro-aware,
that is, *instrument* them so that they can send and
receive Bro events at appropriate moments in the execution flow.
This will involve modifying an existing code tree, so care needs to
be taken to avoid unwanted side effects. By protecting the instrumented
code with
``#ifdef``/``#endif``
statements you can still build the original application, using the
instrumented source tree. The ``broccoli-config`` script helps you in doing so because
it already adds ``-DBROCCOLI`` to the compiler flags
reported when run with the ``--cflags`` option:

cpk25@localhost:/home/cpk25 > broccoli-config --cflags
-I/usr/local/include -I/usr/local/include -DBROCCOLI

So simply surround all inserted code with a preprocessor check
for ``BROCCOLI`` and you will be able to
build the original application as soon as ``BROCCOLI``
is not defined.

The Broccoli API
================

Time for some code. In the code snippets below we will introduce variables
whenever context requires them and not necessarily when C requires them.
The library does not require calling a global initialization function.
In order to make the API known, include ``broccoli.h``:

::

	#ifdef BROCCOLI
	#include <broccoli.h>
	#endif

.. note:: *A note on Broccoli's memory management philosophy:*
   Broccoli generally does not release objects you allocate.
   The approach taken is "you clean up what you allocate."

Initialization
--------------

Broccoli requires global initialization before most of its
other other functions can be used. Generally, the way to
initialize Broccoli is as follows:

::

	bro_init(NULL);

The argument to
``bro_init()``
provides optional initialization context, and may be kept
``NULL`` for normal use. If required, you may
allocate a ``BroCtx`` structure locally,
initialize it using
``bro_ctx_init()``,
fill in additional values as required and and subsequently pass it to
``bro_init()``:

::

	BroCtx ctx;
	bro_ctx_init(&ctx);
	/* Make adjustments to the context structure as required... */
	bro_init(&ctx);

.. caution:: The ``BroCtx`` structure currently contains
   a set of five different callback function pointers.  These
   are *required* for thread-safe operation
   of OpenSSL (Broccoli itself is thread-safe).  If you intend
   to use Broccoli in a multithreaded environment, you need to
   implement functions and register them via the
   ``BroCtx`` structure.  The O'Reilly book
   "Network Security with OpenSSL" by Viega et al. shows how to
   implement these callbacks.

.. caution:: You *must* call
   ``bro_init()``
   at the start of your application. Undefined behavior may result
   if you don't.

Data types in Broccoli
----------------------

Broccoli declares a number of data types in ``broccoli.h`` that
you should know about. The more complex ones are kept opaque, while you do get
access to the fields in the simpler ones. The full list is as follows:

- Simple signed and unsigned types: int, uint,
  uint32, uint16 and uchar.

- Connection handles: BroConn, kept opaque.

- Bro events: BroEvent, kept opaque.

- Buffer objects: BroBuf, kept opaque. See the separate
  `section on buffer management <http://www.icir.org/christian/broccoli/manual/c84.html#AEN696>`_ for details.

- Ports: BroPort for network ports, defined as follows:

::

	typedef struct bro_port {
	uint16       port_num;   /* port number in host byte order \*/
	int          port_proto; /* IPPROTO_xxx \*/
	} BroPort;

- Records: BroRecord, kept opaque. See the separate
  `section on record handling <http://www.icir.org/christian/broccoli/manual/c84.html#AEN554>`_ for details.

- Strings (character and binary): BroString, defined as follows:

::

	typedef struct bro_string {
	int          str_len;
	char        \*str_val;
	} BroString;

- BroStrings are mostly kept transparent for convenience; please have a look at the
  string API: `bro_string_init() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-INIT>`_, `bro_string_set() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-SET>`_, `bro_string_set_data() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-SET-DATA>`_, `bro_string_copy() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-COPY>`_, `bro_string_cleanup() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-CLEANUP>`_, and `bro_string_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-STRING-FREE>`_.

- Tables: BroTable, kept opaque. See the separate
  `section on table handling <http://www.icir.org/christian/broccoli/manual/c84.html#AEN599>`_ for details.

- Sets: BroSet, kept opaque. See the separate
  `section on handling sets <http://www.icir.org/christian/broccoli/manual/c84.html#AEN636>`_ for details.

- Subnets: BroSubnet, defined as follows:

::

	typedef struct bro_subnet
	{
	uint32       sn_net;     /* IP address in network byte order \*/
	uint32       sn_width;   /* Length of prefix to consider. \*/
	} BroSubnet;

Managing connections
--------------------

You can use Broccoli to establish a connection to a remote Bro, or to create a
Broccoli-enabled server application that other Bros will connect to. (This
means that in principle, you can also use Broccoli purely as middleware and
have multiple Broccoli applications communicate directly.)

In order to establish a connection to a remote Bro, you first obtain a connection
handle. You then use this connection handle to request events, connect to the
remote Bro, send events, etc. Connection handles are pointers to ``BroConn``
structures, which are kept opaque. Use
`bro_conn_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-NEW>`_ or
`bro_conn_new_str() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-NEW-STR>`_
to obtain a handle, depending on what parameters are more convenient for
you: the former accepts the IP address and port number as separate numerical
arguments, the latter uses a single string to encode both, in "hostname:port"
format.

To write a Broccoli-enabled server, you first need to implement the usual
``socket()`` / ``bind()`` /
``listen()`` / ``accept()`` routine.
Once you have obtained a file descriptor for the new connection from ``accept()``,
you pass it to the third function that returns a ``BroConn``
handle,
`bro_conn_new_socket() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-NEW-SOCKET>`_.
The rest of the connection handling then proceeds as in the client scenario.

All three calls accept additional flags for fine-tuning connection behaviour.
These flags are:

- ``BRO_CFLAG_NONE``: no functionality. Use when
  no flags are desired.

- ``BRO_CFLAG_RECONNECT``:
  When using this option, Broccoli will attempt to reconnect to the peer
  after lost connectivity transparently. Essentially whenever you try to
  read from or write to the peer and its connection broke down, a
  full reconnect including complete handshaking is attempted. You can check
  whether the connection to a peer is alive at any time using
  `bro_conn_alive() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-ALIVE>`_.

- ``BRO_CFLAG_ALWAYS_QUEUE``:
  When using this option, Broccoli will queue any events you send for
  later transmission when a connection is currently down. Without using this
  flag, any events you attempt to send while a connection is down get dropped
  on the floor. Note that Broccoli maintains a maximum queue size per connection
  so if you attempt to send lots of events while the connection is down, the
  oldest events may start to get dropped nonetheless. Again, you can check
  whether the connection is currently okay by using
  `bro_conn_alive() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-ALIVE>`_.

- ``BRO_CFLAG_DONTCACHE``:
  When using this option, Broccoli will ask the peer not to use caching on
  the objects it sends to us. This is the default, and the flag need not
  normally be used. It is kept to maintain backward compatibility.

- ``BRO_CFLAG_CACHE``:
  When using this option, Broccoli will ask the peer to use caching on
  the objects it sends to us. Caching is normally disabled.

- ``BRO_CFLAG_YIELD``: When
  using this option,
  ``bro_conn_process_input()``
  processes at most one event at a time and then
  returns.

By obtaining a connection handle, you do not also establish a connection right
away. This is done using
`bro_conn_connect() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-CONNECT>`_.
The main reason for this is to allow you to subscribe to events
(using
`bro_event_registry_add() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD>`_,
see `Receiving Events`_, below)
before establishing the connection. Upon returning from
`bro_conn_connect() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-CONNECT>`_
you are guaranteed to receive all instances of the event types you have
requested, while later on during the connection some time may elapse between
the issuing of a request for events and the processing of that request at the
remote end.
Connections are established via TCP, optionally using SSL encryption. See
"`Configuring encrypted communication`_", below, for more
information on setting up enncryption.
The port numbers Bro agents and Broccoli applications listen on can vary from peer
to peer.

Finally, `bro_conn_delete() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-DELETE>`_
terminates a connection and releases all resources associated with it.
You can create as many connections as you like, to one or more peers.
You can obtain the file descriptor of a connection using
`bro_conn_get_fd() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-GET-FD>`_.

::

	char host_str = "bro.yourorganization.com";
	int port      = 1234;
	struct hostent *host;
	BroConn *bc;

	if (! (host = gethostbyname(host_str)) ||
	! (host->h_addr_list[0])) {
	/* Error handling -- could not resolve host */
	}

	/* In this example, we obtain a connection handle, then register event handlers,
	* and finally connect to the remote Bro.
	*
	* First obtain a connection handle:
	*/
	if (! (bc = bro_conn_new((struct in_addr*) host->h_addr_list[0], htons(port), BRO_CFLAG_NONE))) {
	/* Error handling  - could not get connection handle */
	}

	/* Register event handlers:
	*/
	bro_event_registry_add(bc, "foo", bro_foo_handler, NULL);
	/* ... */

	/* Now connect to the peer:
	*/
	if (! bro_conn_connect(bc)) {
	/* Error handling - could not connect to remote Bro. */
	}

	/* Send and receive events ... */

	/* Disconnect from Bro and clean up connection */
	bro_conn_delete(bc);

Or simply use the string-based version:

::

	char host_str = "bro.yourcompany.com:1234";
	BroConn \*bc;

	/* In this example we don't request any events from the peer, but
	* we ask it not to use the serialization cache.
	*
	* Again, first obtain a connection handle:
	*/
	if (! (bc = bro_conn_new_str(host_str, BRO_CFLAG_DONTCACHE))) {
	/* Error handling  - could not get connection handle */
	}

	/* Now connect to the peer:
	*/
	if (! bro_conn_connect(bc)) {
	/* Error handling - could not connect to remote Bro. */
	}

	/* ... */

Connection classes
------------------

When you want to establish connections from multiple Broccoli applications
with different purposes, the peer needs a means to understand what kind of
application each connection belongs to. The real meaning of "kind of application"
here is "sets of event types to request", because depending on the class of
an application, the peer will likey want to receive different types of events.

Broccoli lets you set the class of a connection using
`bro_conn_set_class() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-SET-CLASS>`_.
When using this feature, you need to call that function before issuing a
`bro_conn_connect() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-CONNECT>`_,
since the class of a connection is determined at connection startup.

::

	if (! (bc = bro_conn_new_str(host_str, BRO_CFLAG_DONTCACHE))) {
	/* Error handling  - could not get connection handle */
	}

	/* Set class of this connection: */
	bro_conn_set_class(bc, "syslog");

	if (! bro_conn_connect(bc)) {
	/* Error handling - could not connect to remote Bro. */
	}

If your peer is a Bro node, you need to match the chosen connection class in
the remote Bro's ``Remote::destinations`` configuration.
See `Configuring event reception in Bro policies`_, below, for how to do this.
Finally, in order to obtain the class of a connection as indicated by the remote side, use
`bro_conn_get_peer_class() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-GET-PEER-CLASS>`_.

Composing and sending events
----------------------------

In order to send an event to the remote Bro agent, you first create
an empty event structure with the name of the event, then add parameters
to pass to the event handler at the remote agent, and then send off the
event.

.. note:: *Bro peers ignore unrequested events.*
   You need to make sure that the remote Bro agent is interested in receiving
   the events you send. This interest is expressed in policy configuration.
   We'll explain this in more detail in `Configuring event reception in Bro policies`_, below,
   and for now assume that our remote peer is configured to receive the
   events we send.

Let's assume we want to request a report of all connections a remote
Bro currently keeps state for that match a given destination port and
host name and that have amassed more than a certain number of bytes.
The idea is to send an event to the remote Bro that contains the
query, identifiable through a request ID, and have the remote Bro
answer us with ``remote_conn`` events containing the information we asked for. The definition of our
requesting event could look as follows in the Bro policy:

::

	event report_conns(req_id: int, dest_host: string, dest_port: port, min_size: count);

First, create a new event:

::

	BroEvent *ev;

	if (! (ev = bro_event_new("report_conns"))) {
	/* Error handling - could not allocate new event. */
	}

Now we need to add parameters to the event. The sequence and types must
match the event handler declaration -- check the Bro policy to make
sure they match. The function to use for adding parameter values is
`bro_event_add_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-ADD-VAL>`_
All values are passed as *pointer arguments* and are copied internally,
so the object you're pointing to stays unmodified at all times. You clean
up what you allocate. In order to indicate the type of the value passed into the
function, you need to pass a numerical type identifier along as well.
Table-1_ lists the value types that Broccoli supports along with
the type identifier and data structures to point to.

.. _Table-1:

Types, type tags, and data structures for event parameters in Broccoli
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
==============================  =================  ====================
Type                            Type tag           Data type pointed to
==============================  =================  ====================
Boolean                         BRO_TYPE_BOOL      int
Integer value                   BRO_TYPE_INT       int
Counter (nonnegative integers)  BRO_TYPE_COUNT     uint32
Enums (enumerated values)       BRO_TYPE_ENUM      int (see also the description of `bro_event_add_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-ADD-VAL>`_'s type_name argument, below)
Floating-point number           BRO_TYPE_DOUBLE    double
Timestamp                       BRO_TYPE_TIME      double (see also `bro_util_timeval_to_double() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-UTIL-TIMEVAL-TO-DOUBLE>`_ and `bro_util_current_time() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-UTIL-CURRENT-TIME>`_)
Time interval                   BRO_TYPE_INTERVAL  double
Strings (text and binary)       BRO_TYPE_STRING    BroString (see also the family of bro_string_xxx() functions)
Network ports                   BRO_TYPE_PORT      BroPort, with the port number in host byte order
IPv4 address                    BRO_TYPE_IPADDR    uint32, in network byte order
IPv4 network                    BRO_TYPE_NET       uint32, in network byte order
IPv4 subnet                     BRO_TYPE_SUBNET    BroSubnet, with the sn_net member in network byte order
Record                          BRO_TYPE_RECORD    BroRecord (see also the family of bro_record_xxx() functions and their explanation below)
Table                           BRO_TYPE_TABLE     BroTable (see also the family of bro_table_xxx() functions and their explanation below)
Record                          BRO_TYPE_SET       BroSet (see also the family of bro_set_xxx() functions and their explanation below)
==============================  =================  ====================


Knowing these, we can now compose a
``request_connections`` event:

::

	BroString dest_host;
	BroPort dest_port;
	uint32 min_size;
	int req_id = 0;

	bro_event_add_val(ev, BRO_TYPE_INT, NULL, &req_id);
	req_id++;

	bro_string_set(&dest_host, "desthost.destdomain.com");
	bro_event_add_val(ev, BRO_TYPE_STRING, NULL, &dest_host);
	bro_string_cleanup(&dest_host);

	dest_port.dst_port = 80;
	dest_port.dst_proto = IPPROTO_TCP;
	bro_event_add_val(ev, BRO_TYPE_PORT, NULL, &dest_port);

	min_size = 1000;
	bro_event_add_val(ev, BRO_TYPE_COUNT, NULL, &min_size);

The third argument to
`bro_event_add_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-ADD-VAL>`_
lets you specify a specialization of the types listed in
Table-1_. This is generally not necessary
except for one situationn: When using ``BRO_TYPE_ENUM``. You currently
cannot define
a Bro-level enum type in Broccoli, and thus when sending an enum value, you
have to specify the type of the enum along with the value. For example, in order
to add an instance of enum ``transport_type`` defined in
Bro's ``bro.init``, you would use

::

	int transport_proto = 2;
	/* ... */
	bro_event_add_val(ev, BRO_TYPE_ENUM, "transport_proto", &transport_proto);

to get the equivalent of "udp" on the remote side. The same system is used
to point out type names when calling
`bro_event_set_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-SET-VAL>`_,
`bro_record_add_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-ADD-VAL>`_,
`bro_record_set_nth_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-SET-NTH-VAL>`_, and
`bro_record_set_named_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-SET-NAMED-VAL>`_.

All that's left to do now is to send off the event. For this, use
`bro_event_send() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-SEND>`_
and pass it the connection handle and the event. The function returns
``TRUE`` when the event could be sent right away or if
it was queued for later delivery. ``FALSE`` is returned
on error. If the event get queued, this does not indicate an error --
likely the connection was just not
ready to send the event at this point. Whenever you call
`bro_event_send() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-SEND>`_,
Broccoli attempts to send as much of an existing event queue as possible.
Again, the event is copied internally to make it easier for you to
send the same event repeatedly. You clean up what you allocate.

::

	bro_event_send(bc, ev);
	bro_event_free(ev);

Two other functions may be useful to you:
`bro_event_queue_length() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-QUEUE-LENGTH>`_
tells you how many events are currently queued, and
`bro_event_queue_flush() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-QUEUE-FLUSH>`_
attempts to flush the current event queue and returns the number of events that do remain
in the queue after the flush. *Note:* you do not normally need
to call this function, queue flushing is attempted every time you send an event.

.. _receiving-events:

Receiving events
---------------------------------------

Receiving events is a little more work because you need to

1. tell Broccoli what to do when requested events arrive,

#. let the remote Bro agent know that you would like to receive those events,

#. find a spot in the code path suitable for extracting and processing arriving events.

Each of these steps is explained in the following sections.

Implementing event callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When Broccoli receives an event, it tries to dispatch the event to callbacks
registered for that event type. The place where callbacks get registered is
called the callback registry. Any callbacks registered for the arriving
event's name are invoked with the parameters shipped with the event. There
are two styles of argument passing to the event callbacks.
Which one is better suited depends on your application.

- *Expanded argument passing.* Each event argument
  is passed via a pointer to the callback. This makes best sense when you
  know the type of the event and of its arguments, because it provides you
  immediate access to arguments as when using a normal C function.

  In order to register a callback with expanded argument passing, use
  `bro_event_registry_add() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD>`_
  and pass it the connection handle, the name of the event for which you
  register the callback, the callback itself that matches the signature
  of the ``BroEventFunc`` type, and any user data (or
  ``NULL``) you want to see passed to the callback on
  each invocation. The callback's type is defined rather generically as follows:

::

	typedef void (*BroEventFunc) (BroConn *bc, void *user_data, ...);

..

  It requires a connection handle as its first argument
  and a pointer to user-provided callback data as the second argument.
  Broccoli will pass the connection handle of the connection on which the event
  arrived through to the callback. ``BroEventFunc``
  are variadic, because each callback you provide is directly invoked with
  pointers to the parameters of the event, in a format directly usable in C.
  All you need to know is what type to point to in order to receive the
  parameters in the right layout. Refer to Table-1_
  again for a summary of those types. Record types are more involved and are
  addressed in more detail in `Handling Records`_, below 

  .. note:: Note that *all* parameters are passed to the
     callback as pointers, even elementary types such as ``int``
     that would normally be passed directly.
     Also note that Broccoli manages the lifecycle of event parameters
     and therefore you do *not* have to clean them up inside
     the event handler.

  Continuing our example, we will want to process the connection reports
  that contain the responses to our ``report_conns``
  event. Let's assume those look as follows:

::

	event remote_conn(req_id: int, conn: connection);

..

  The reply events contain the request ID so we can associate requests
  with replies, and a connection record (defined in ``bro.init``
  in Bro. (It'd be nicer to report all replies in a single event but we'll
  ignore that for now.) For this event, our callback would look like this:

::

	void remote_conn_cb(BroConn *bc, void *user_data, int *req_id, BroRecord *conn);

..

  Once more, you clean up what you allocate, and since you never allocated the
  space these arguments point to, you also don't clean them up. Finally, we register
  the callback using
  `bro_event_registry_add() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD>`_:

::

	bro_event_registry_add(bc, "remote_conn", remote_conn_cb, NULL);

..

  In this case we have no additional data to be passed into the
  callback, so we use ``NULL`` for the last argument.
  If you have multiple events you are interested in, register
  each one in this fashion.

- *Compact argument passing.* This is designed for
  situations when you have to determine how to handle different types of
  events at runtime, for example when writing language bindings or when
  implementing generic event handlers for multiple event types.
  The callback is passed a connection handle and the
  user data as above but is only passed one additional pointer, to a
  BroEvMeta structure. This structure contains all metadata
  about the event, including its name, timestamp (in UTC) of creation,
  number of arguments, the arguments'
  types (via type tags as listed in Table-1_),
  and the arguments themselves.

  In order to register a callback with compact argument passing, use
  `bro_event_registry_add_compact() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD-COMPACT>`_
  and pass it similar arguments as you'd use with
  `bro_event_registry_add() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD>`_.
  The callback's type is defined as follows:

::

	typedef void (*BroCompactEventFunc) (BroConn *bc, void *user_data, BroEvMeta *meta);

..

  .. note:: As before, Broccoli manages the lifecycle of event parameters.
     You do not  have to clean up the BroEvMeta
     structure or any of its contents.

  Below is sample code for extracting the arguments form the BroEvMeta
  structure, using our running example. This is still written with the assumption
  that we know the types of the arguments, but note that this is not a requirement
  for this style of callback.

::

	void remote_conn_cb(BroConn *bc, void *user_data, BroEvMeta *meta)
	{
		int *req_id;
		BroRecord *rec;

		/* For demonstration, print out the event's name: */

		printf("Handling a %s event.\n", meta->ev_name);

		/* Sanity-check the number of arguments: */

		if (meta->ev_numargs != 2)
			{ /* error */ }

		/* Sanity-check the argument types: */

		if (meta->ev_args[0].arg_type != BRO_TYPE_INT)
			{ /* error */ }

		if (meta->ev_args[1].arg_type != BRO_TYPE_RECORD)
			{ /* error */ }

		req_id = (int *) meta->ev_args[0].arg_data;
		rec = (BroRecord *) meta->ev_args[1].arg_data;

		/* ... */
	}

..

  Finally, register the callback using
  `bro_event_registry_add_compact() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-EVENT-REGISTRY-ADD-COMPACT>`_:

::

	bro_event_registry_add_compact(bc, "remote_conn", remote_conn_cb, NULL);

Requesting event delivery
~~~~~~~~~~~~~~~~~~~~~~~~~

At this point, Broccoli knows what to do with the requested events upon
arrival.  What's left to do is to let the remote Bro know that you
would like to receive the events for which you registered. If you haven't
yet called :ref:```bro_conn_connect()`` <bro-conn-connect>`,
then there is nothing to do, since that function will request the registered
events anyway. Once connected, you can still request events. To do so, call
:ref:```bro_event_registry_request()`` <bro-event-registry-request>`:

bro_event_registry_request(bc);

This mechanism also implies that no unrequested events will be delivered
to us (and if that happened for whatever reason, the event would simply
be dropped on the floor).

.. note:: *Note that at the moment you cannot unrequest events, nor
   can you request events based on predicates on the values of the
   events' arguments.*

Reading events from the connection handle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At this point the remote Bro will start sending you the requested events
once they are triggered. What is left to do is to read the arriving events
from the connection and trigger dispatching them to the registered callbacks.

If you are writing a new Bro-enabled application, this is easy, and you can
choose among two approaches: polling explicitly via Broccoli's API, or using
``select()`` on the file handle associated with a BroConn.
The former case is particularly straightforward; all you need to do is
call
`bro_conn_process_input() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-PROCESS-INPUT>`_,
which will go off and check if any events have arrived and if so, dispatch
them accordingly. This function does not block -- if no events have
arrived, then the call will return immediately. For more fine-grained control
over your I/O handling, you will probably want to use
`bro_conn_get_fd() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-GET-FD>`_
to obtain the file descriptor of your connection and then incorporate that
in your standard ``FD_SET``/``select()``
code. Once you have determined that data in fact are ready to be read from
the obtained file descriptor, you can then try another
`bro_conn_process_input() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-PROCESS-INPUT>`_,
this time knowing that it'll find something to dispatch.

As a side note, if you don't process arriving events frequently enough,
then TCP's flow control will start to slow down the sender until eventually
events will queue up and be dropped at the sending end.

.. _records:

Handling records
------------------------------

Broccoli supports record structures, i.e., types that pack a set of values
together, placing each value into its own field. In Broccoli, the way you handle
records is somewhat similar to events:
after creating an empty record (of opaque type BroRecord, you can
iteratively add fields and values to it. The main difference is that you must specify a
field name with the value; each value in a record can be identified both by position
(a numerical index starting from zero), and by field name. You can retrieve vals
in a record by field index or field name. You can also reassign values.
There is no explicit, IDL-style definition of record types. You define the type of
a record implicitly by the sequence of field names and the sequence of the types
of the values you put into the record.

Note that all fields in a record must be assigned before it can be shipped.

The API for record composition consists of
`bro_record_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-NEW>`_,
`bro_record_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-FREE>`_,
`bro_record_add_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-ADD-VAL>`_,
`bro_record_set_nth_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-SET-NTH-VAL>`_, and
`bro_record_set_named_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-SET-NAMED-VAL>`_.

On records that use field names, the names of individual fields can be extracted using
`bro_record_get_nth_name() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-GET-NTH-NAME>`_.
Extracting values from a record is done using
`bro_record_get_nth_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-GET-NTH-VAL>`_ and
`bro_record_get_named_val() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-RECORD-GET-NAMED-VAL>`_.
The former allows numerical indexing of the fields in the record, the latter provides
name-based lookups. Both need to be passed the record you want to extract a value from,
the index or name of the field, and either a pointer to an int holding a
BRO_TYPE_xxx value (see again Table-1_ for a
summary of those types) or ``NULL``. The pointer, if not
``NULL``, serves two purposes: type checking and type retrieval.
Type checking is performed if the value of the int upon calling the
functions is not BRO_TYPE_UNKNOWN. The type tag of the requested record
field then has to match the type tag stored in the int, otherwise
``NULL`` is returned. If the int stores BRO_TYPE_UNKNOWN
upon calling, no type-checking is performed. In *both* cases,
the *actual* type of the
requested record field is returned in the int pointed to upon
return from the function. Since you have no guarantees of the type of the value
upon return if you pass ``NULL`` as the int pointer,
this is a bad idea and either BRO_TYPE_UNKNOWN or another type value
should always be used.

For example, you could extract the value of the record field "label", which
we assume should be a string, in the following ways:

::

	BroRecord *rec = /* obtained somehow */
	BroString *string;
	int type;

	/* --- Example 1 --- */

	type = BRO_TYPE_STRING; /* Use type-checking, will not accept other type */

	if (! (string = bro_record_get_named_val(rec, "label", &type))) {
		/* Error handling, either there's no field of that value,
		* or the value is not of BRO_TYPE_STRING. The actual
		* type is now stored in "type".
		*/
	}

	/* --- Example 2 --- */

	type = BRO_TYPE_UNKNOWN; /* No type checking, just report the existant type */

	if (! (string = bro_record_get_named_val(rec, "label", &type))) {
		/* Error handling, no field of that name exists. */
	}

	printf("The type of the value in field 'label' is %i\n", type);

	/* --- Example 3 --- */

	if (! (string = bro_record_get_named_val(rec, "label", NULL))) {
		/* Error handling, no field of that name exists. */
	}

	/* We now have a value, but we can't really be sure of its type */

Record fields can be records, for example in the case of Bro's standard
connection record type. In this case, in order to get to a nested record, you
use ``BRO_TYPE_RECORD``:

::

	void remote_conn_cb(BroConn *bc, int *req_id, BroRecord *conn) {
		BroRecord *conn_id;
		int type = BRO_TYPE_RECORD;
		if (! (conn_id = bro_record_get_named_val(conn, "id", &type))) {
			/* Error handling */
		}
	}

.. _tables:

Handling tables
---------------

Broccoli supports Bro-style tables, i.e., associative containers that map
instances of a key type to an instance of a value type. A given key
can only ever point to a single value. The key type can be
*composite*, i.e., it may consist of an ordered
sequence ofdifferent types, or it can be *direct*,
i.e., consisting of a single type (such as an integer, a string, or
a record).

The API for table manipulation consists of
`bro_table_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-NEW>`_,
`bro_table_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-FREE>`_,
`bro_table_insert() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-INSERT>`_,
`bro_table_find() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-FIND>`_,
`bro_table_get_size() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-GET-SIZE>`_,
`bro_table_get_types() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-GET-TYPES>`_,
and
`bro_table_foreach() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-FOREACH>`_.

Tables are handled similarly to records in that typing is determined
dynamically by the initial key/value pair inserted. The resulting types
can be obtained via
`bro_table_get_types() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-GET-TYPES>`_.
Should the types not have been determined yet, ``BRO_TYPE_UNKNOWN``
will result. Also, as with records,
values inserted into the table are copied internally, and the ones passed
to the insertion functions remain unaffected.

In contrast to records, table entries can be iterated. By passing a function
of signature
`BroTableCallback() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BROTABLECALLBACK>`_
and a pointer to data of your choosing,
`bro_table_foreach() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-TABLE-FOREACH>`_
will invoke the given function for each key/value pair stored in the table.
Return ``TRUE`` to keep the iteration going, or ``FALSE``
to stop it.

.. caution:: The main thing to know about Broccoli's tables is how to use composite key
   types. To avoid additional API calls, you may treat composite key types
   exactly as records, though you do not need to use field names when assigning
   elements to individual fields. So in order to insert a key/value pair, you
   create a record with the needed items assigned to its slots, and use this
   record as the key object. In order to differentiate composite index types
   from direct ones consisting of a single record, use ``BRO_TYPE_LIST``
   as the type of the record, as opposed to ``BRO_TYPE_RECORD``.
   Broccoli will then know to interpret the record
   as an ordered sequence of items making up a composite element, not a regular
   record.

``brotable.c`` in the test subdirectory of the Broccoli tree
contains an extensive example of using tables with composite as well as direct
indexing types.

.. _sets:

Handling sets
------------------------

Sets are essentially tables with void value types.
The API for set manipulation consists of
`bro_set_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-NEW>`_,
`bro_set_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-FREE>`_,
`bro_set_insert() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-INSERT>`_,
`bro_set_find() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-FIND>`_,
`bro_set_get_size() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-GET-SIZE>`_,
`bro_set_get_type() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-GET-TYPE>`_,
and
`bro_set_foreach() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-SET-FOREACH>`_.

Associating data with connections
---------------------------------

You will often find that you would like to connect data with
a ``BroConn``. Broccoli provides an API that
lets you associate data items with a connection handle through
a string-based key-value registry. The functions of interest
are
`bro_conn_data_set() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-DATA-SET>`_,
`bro_conn_data_get() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-DATA-GET>`_, and
`bro_conn_data_del() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-DATA-DEL>`_.
You need to provide a string identifier for a data item and can then use
that string to register, look up, and remove the associated data item.
Note that there is currently no mechanism to trigger a destructor
function for registered data items when the Bro connection is terminated.
You therefore need to make sure that all data items that you do
not have pointers to via some other means are properly released before
calling
``bro_disconnect()``.

.. _config-files:

Configuration files
-------------------

Imagine you have instrumented the mother of all server applications.
Building it takes forever, and every now and then you need to change
some of the parameters that your Broccoli code uses, such as the host names
of the Bro agents to talk to.
To allow you to do this quickly, Broccoli comes with support for
configuration files. All you need to do is change the settings in the
file and restart the application (we're considering adding support
for volatile configuration items that are read from the file every
time they are requested).

A configuration is read from a single configuration file.
This file can be read from two different locations:

- The system-wide configuration file. You can obtain the location
  of this config file by running ``broccoli-config --config``.

- Alternatively, a per-user configuration file stored in ``~/.broccoli.conf``
  can be used.

If a user has a configuration file in ``~/.broccoli.conf``,
it is used exclusively, otherwise the global one is used.

.. caution:: ``~/.broccoli.conf`` *will only be used if
   it is a regular file, not executable, and neither group nor others have
   any permissions on the file. That is, the file's permissions must look
   like* ``-rw-------`` *or* ``-r--------``.

In the configuration file, a "#" anywhere starts a comment that runs
to the end of the line. Configuration items are specified as key-value
pairs:

::

	# This is the Broccoli system-wide configuration file.
	#
	# Entries are of the form <identifier> <value>, where the identifier
	# is a sequence of letters, and value can be a string (including
	# whitespace), and floating point or integer numbers. Comments start
	# with a "#" and go to the end of the line. For boolean values, you
	# may also use "yes", "on", "true", "no", "off", or "false".
	# Strings may contain whitespace, but need to be surrounded by
	# double quotes '"'.
	#
	# Examples:
	#
	Foo/PeerName          mybro.securesite.com
	Foo/PortNum           123
	Bar/SomeFloat         1.23443543
	Bar/SomeLongStr       "Hello World"

You can also have multiple sections in your configuration. Your application can
select a section as the current one, and queries for configuration settings will
then only be answered with values specified in that section. A section is started
by putting its name (no whitespace please) between square brackets. Configuration
items positioned before the first section title are in the default domain and
will be used by default.

::

	# This section contains all settings for myapp.
	[ myapp ]

You can name identifiers any way you like, but to keep things
organized it is recommended to keep a namespace hierarchy similar
to the file system. In the code, you can query configuration
items using
`bro_conf_get_str() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONF-GET-STR>`_,
`bro_conf_get_int() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONF-GET-INT>`_, and
`bro_conf_get_dbl() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONF-GET-DBL>`_.
You can switch between sections using
`bro_conf_set_domain() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONF-SET-DOMAIN>`_.

.. _buffers:

Using dynamic buffers
-----------------------------------

Broccoli provides an API for dynamically allocatable, growable, shrinkable,
and consumable buffers with ``BroBuf``. You may or may
not find this useful -- Broccoli mainly provides this feature in
``broccoli.h`` because these buffers are used internally
anyway and because they are typical case of something that people implement
themselves over and over again, for example to collect a set of data before
sending it through a file descriptor, etc.

The buffers work as follows. The structure implementing a buffer is
called BroBuf. BroBufs are initialized to a default size when created via
`bro_buf_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-NEW>`_,
and released using
`bro_buf_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-FREE>`_.
Each BroBuf has a content
pointer that points to an arbitrary location between the start of the
buffer and the first byte after the last byte currently
used in the buffer (see buf_off in the illustration below). The content
pointer can seek to arbitrary locations, and data can be copied from and
into the buffer, adjusting the content pointer accordingly.
You can repeatedly append data to end of the buffer's used contents using
`bro_buf_append() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-APPEND>`_.

::

	<---------------- allocated buffer space ------------>
	<======== used buffer space ========>
	^              ^                    ^               ^
	|              |                    |               |
	`buf           `buf_ptr             `buf_off        `buf_len

Have a look at the following functions for the details:
`bro_buf_new() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-NEW>`_,
`bro_buf_free() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-FREE>`_,
`bro_buf_append() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-APPEND>`_,
`bro_buf_consume() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-CONSUME>`_,
`bro_buf_reset() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-RESET>`_,
`bro_buf_get() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-GET>`_,
`bro_buf_get_end() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-GET-END>`_,
`bro_buf_get_size() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-GET-SIZE>`_,
`bro_buf_get_used_size() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-GET-USED-SIZE>`_,
`bro_buf_ptr_get() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-PTR-GET>`_,
`bro_buf_ptr_tell() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-PTR-TELL>`_,
`bro_buf_ptr_seek() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-PTR-SEEK>`_,
`bro_buf_ptr_check() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-PTR-CHECK>`_, and
`bro_buf_ptr_read() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-BUF-PTR-READ>`_.

.. _encryption:

Configuring encrypted communication
====================================================

Encrypted communication between Bro peers takes place over an SSL connection in
which both endpoints of the connection are authenticated. This requires at least
some PKI in the form of a certificate authority (CA) which you use to issue and sign
certificates for your Bro peers. To facilitate the SSL setup, each peer requires
three documents: a certificate signed by the CA and containing the public key, the
corresponding private key, and a copy of the CA's certificate.

The OpenSSL command line tool **openssl** can be used to create all
files neccessary, but its unstructured arguments and poor documentation make it
a pain to use and waste lots of people a lot of time [#]_
. Therefore, the Bro distribution comes with two scripts,
**ca-create** and **ca-issue**. You use the former
once to set up your CA, and the latter to create a certificate for each of the Bro
peers in your infrastructure.

- **ca-create** lets you choose a directory in which the
  CA maintains its files. If you set ``BRO_CA_DIR`` in your
  environment, it will be used for this purpose. After entering a passphrase
  for the private key of your CA, you can find the self-signed certificate
  of your CA in ``$BRO_CA_DIR/ca_cert.pem``.

- **ca-issue** first requires the directory of the CA,
  offering ``$BRO_CA_DIR`` if that is found. It asks you for
  a prefix for the certificate to be generated, for the passphrase of the
  private key of the CA so the new certificate can be signed, for the
  passphrase for the new private key, and for a few parameters that make up
  the "distinguished name" of the certificate. This name (i.e., the combination
  of all the fields you enter a value for) must be unique among all your Bro
  peers. Once that is done, you find a new certificate named
  ``<prefix>.pem`` in your current
  directory. This file actually consists of two of the three cryptographic
  documents mentioned above, namely the new certificate and the private key.
  We refer to it as "certificate" for simplicity.

In order to enable encrypted communication for your Broccoli application, you
need to put the CA certificate and the peer certificate in the
``/broccoli/ca_cert`` and
``/broccoli/host_cert`` keys, respectively, in the configuration file.
Optionally, you can store the private key in a separate file specified by
``/broccoli/host_key``.
To quickly enable/disable a certificate configuration, the
``/broccoli/use_ssl`` key can be used.

.. caution:: *This is where you configure whether to use encrypted or unencrypted
   connections.*

   If the ``/broccoli/use_ssl`` key is present and set to one of
   "yes", "true", "on", or 1, then SSL will be used and an incorrect or
   missing certificate configuration will cause connection attempts to fail.
   If the key's value is one of "no", "false", "off", or 0, then in no case
   will SSL be used and connections will always be cleartext.

   If the ``/broccoli/use_ssl`` key is *not*
   present, then SSL will be used if a certificate configuration is
   found, and invalid certificates will cause the connection to fail.
   If no certificates are configured, cleartext connections will be used.

   In no case does an SSL-enabled setup ever fall back to a cleartext one.

::

	/broccoli/use_ssl           yes
	/broccoli/ca_cert          <path>/ca_cert.pem
	/broccoli/host_cert        <path>/bro_cert.pem
	/broccoli/host_key         <path>/bro_cert.key

In a Bro policy, you need to load the ``listen-ssl.bro`` policy and
redef ``ssl_ca_certificate`` and ``ssl_private_key``,
defined in ``bro.init``:

::

	@load listen-ssl

	redef ssl_ca_certificate   = "<path>/ca_cert.pem";
	redef ssl_private_key      = "<path>/bro.pem";

By default, you will be prompted for the passphrase for the private key matching
the public key in your agent's certificate. Depending on your application's user
interface and deployment, this may be inappropriate. You can store the passphrase
in the config file as well, using the following identifier:

::

	/broccoli/host_pass        foobar

.. caution:: *Make sure that access to your configuration is restricted.*

   If you provide the passphrase this way, it is obviously essential to have
   restrictive permissions on the configuration file. Broccoli partially enforces
   this. Please refer to the section on
   `Configuration files`_ for details.

.. _bro-event-config:

Configuring event reception in Bro policies
==================================================================

Before a remote Bro will accept your connection and your events, it needs to have its
policy configured accordingly:

1. Load either ``listen-ssl`` or ``listen-clear``,
   depending on whether you want to have encrypted or cleartext communication. Obviously,
   encrypting the event exchange is recommended and cleartext should only be used for
   early experimental setups. See below for details on how to set up encrypted
   communication via SSL.

2. You need to find a port to use for the Bros and Broccoli applications that will
   listen for connections. Every such agent can use a different port, though default
   ports are provided in the Bro policies.
   To change the port the Bro agent will be listening on from its default
   redefine the ``listen_port_ssl`` or
   ``listen_port_clear`` variables from ``listen-clear.bro``
   or ``listen-ssl.bro``, respectively. Have a look at these policies as well
   as ``remote.bro`` for the default values. Here is the policy
   for the unencrypted case:

::

	@load listen-clear
	redef listen_port_clear = 12345/tcp;

   Including the settings for the cryptographic files introduced in the previous section,
   here is the encrypted one:

::

	@load listen-ssl
	redef listen_port_ssl = 12345/tcp;
	redef ssl_ca_certificate   = "<path>/ca_cert.pem";
	redef ssl_private_key      = "<path>/bro.pem";

3. The policy controlling which peers a Bro agent will communicate with and how this
   communication will happen are defined in the ``destinations`` table
   defined in ``remote.bro``. This table contains entries of type
   Destination, whose members mostly provide default values so you do not
   need to define everything. You need to come up with a tag for the connection
   under which it can be found in the table (a creative one would be "broccoli"),
   the IP address of the peer, the pattern of names of the events the Bro will accept
   from you, whether you want Bro to connect to your
   machine on startup or not, if so, a port to connect to (defaults are
   ``default_port_ssl`` and ``default_port_clear``, also
   defined in ``remote.bro``), a retry timeout, whether to use SSL,
   and the class of a connection as set on the Broccoli side via
   `bro_conn_set_class() <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-CONN-SET-CLASS>`_.

   An example could look as follows:

::

	redef Remote::destinations += {
		["broping"] = \[$host = 127.0.0.1, $class="broping", $events = /ping/, $connect=F, $ssl=F]
	};

..

   This example is taken from ``broping.bro``, the policy the
   remote Bro must run when you want to use the **broping** tool
   explained in the section on `test programs`_ below.
   It will allow an agent on the local host to connect and send "ping" events.
   Our Bro will not attempt to connect, and incoming connections will be expected
   in cleartext.

.. _broccoli-debugging:

Configuring debugging output
=====================================================

If your Broccoli installation was configured with **--enable-debug**,
Broccoli will report two kinds of debugging information: (*i*)
function call traces and
(*ii*) individual debugging messages. Both are enabled by default, but can be adjusted
in two ways.

- In the configuration file: in the appropriate section of the configuration
  file, you can set the keys ``/broccoli/debug_messages`` and
  ``/broccoli/debug_calltrace`` to ``on``/``off``
  to enable/disable the corresponding output.

- In code: you can set the variables
  `bro_debug_calltrace <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-DEBUG-CALLTRACE>`_ and
  `bro_debug_messages <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-DEBUG-MESSAGES>`_
  to 1/0 at any time to enable/disable the corresponding output.

By default, debugging output is inactive (even with debug support compiled in).
You need to enable it explicitly either in your code by assigning 1 to
`bro_debug_calltrace <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-DEBUG-CALLTRACE>`_ and
`bro_debug_messages <http://www.icir.org/christian/broccoli/manual/broccoli-broccoli.html#BRO-DEBUG-MESSAGES>`_,
or by enabling it in the configuration file.

.. _testing:

Test programs
=============

The Broccoli distribution comes with a few small test programs,
located in the ``test/`` directory of the tree.
The most notable one is ``broping`` [#]_, a mini-version of ping.
It sends "ping" events to a remote Bro agent, expecting "pong" events
in return. It operates in two flavours: one uses atomic types for sending
information across, and the other one uses records. The Bro agent you want
to ping needs to run either the ``broping.bro`` or
``broping-record.bro`` policies. You can find these
in the ``test/`` directory of the source tree, and
in ``<prefix>/share/broccoli`` in the installed
version. ``broping.bro`` is shown below. By default,
pinging a Bro on the same machine is configured. If you want your Bro
to be pinged from another machine, you need to update the
``destinations`` variable accordingly.

::

	@load listen-clear;

	global ping_log = open_log_file("ping");

	redef Remote::destinations += {
		["broping"] = [$host = 127.0.0.1, $events = /ping/, $connect=F, $retry = 60 secs, $ssl=F]
	};

	event ping(src_time: time, seq: count)
	{
		event pong(src_time, current_time(), seq);
	}

	event pong(src_time: time, dst_time: time, seq: count)
	{
		print ping_log, fmt("ping received, seq %d, %f at src, %f at dest, one-way: %f",
		seq, src_time, dst_time, dst_time-src_time);
	}

``broping`` sends ping events to Bro. Bro accepts those because they are configured
accordingly in the destinations table. As shown in the policy, ping events
trigger pong events, and ``broccoli`` requests delivery of all pong events back to it.
When running ``broping``, you'll see something like this:

::

	cpk25@localhost:/home/cpk25/devel/broccoli > ./test/broping
	pong event from 127.0.0.1: seq=1, time=0.004700/1.010303 s
	pong event from 127.0.0.1: seq=2, time=0.053777/1.010266 s
	pong event from 127.0.0.1: seq=3, time=0.006435/1.010284 s
	pong event from 127.0.0.1: seq=4, time=0.020278/1.010319 s
	pong event from 127.0.0.1: seq=5, time=0.004563/1.010187 s
	pong event from 127.0.0.1: seq=6, time=0.005685/1.010393 s

Notes
=====

.. [#] In other documents and books on OpenSSL you will find this 
   expressed more politely, using terms such as "daunting to the 
   uninitiated", "challenging", "complex", "intimidating".

.. [#] Pronunciation is said to be somewhere on the continuum between
   "brooping" and "burping".

Broccoli API Reference
######################

Appendix
########

.. _license:

License
=======

Copyright (C) 2004-2008 Christian Kreibich and various contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

About this document
===================

This documentation is maintained in SGML `DocBook <http://www.docbook.org>`_,
API documentation is extracted from the code using the
`**gtk-doc** <http://www.gtk.org/gtk-doc/>`_ tools.



# Depending on whether you want to use encryption or not,
# include "listen-clear" or "listen-ssl":
#
# @load frameworks/communication/listen-ssl
@load frameworks/communication/listen-clear

# Let's make sure we use the same port no matter whether we use encryption or not:
#
@ifdef (Communication::listen_port_clear)
redef Communication::listen_port_clear    = 47758/tcp;
@endif

# If we're using encrypted communication, redef the SSL port and hook in
# the necessary certificates:
#
@ifdef (Communication::listen_port_ssl)
redef Communication::listen_port_ssl      = 47758/tcp;
redef ssl_ca_certificate   = "<path>/ca_cert.pem";
redef ssl_private_key      = "<path>/bro.pem";
@endif

module enumtest;

type enumtype: enum { ENUM1, ENUM2, ENUM3, ENUM4 };

redef Communication::nodes += {
	["broenum"] = [$host = 127.0.0.1, $events = /enumtest/, $connect=F, $ssl=F]
};

event enumtest(e: enumtype)
	{
	print fmt("Received enum val %d/%s", e, e);
	}

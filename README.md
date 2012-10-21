			SpamHole Doc
		  http://www.spamhole.net

I)ruid [CAU]					druid@caughq.org

#################################################################
Theory & Method:

Many spammers constantly search for open SMTP relays, or mail
servers that will allow anyone to send e-mail through them.
Using these servers, they send mass amounts of unsolicited bulk
mail (SPAM).  Most dilligent mail admins on the Internet these
days do not run open relays, however there are many out there
that still do.  Many viruses and trojans are beginning to appear
that turn an unsuspecting cablemodem or DSL user's computer
into an open relay without the user's knowledge.  The most common
way for spammers to find these open relays is for the virus or
trojan to report it's existance directly to them, or by
methodically scanning netowrks looking for them.  This project
hopes to make the latter method a waste of time and effort.

To accomplish our goal, we take the chaff approach.  By creating
as many false 'open relays' on the Internet as possible, we hope
to make the detection and use of a real open relay as much of a
chore as we can.  To accomplish this, we take a rather simple
approach:

When an SMTP client connects to our spamhole, the spamhole will
emulate an SMTP open relay, happily accepting any email messages
that the client wishes to send to it, however rather than
actually delivering the messages, it will silently drop them.

This reference implementation is slightly different.  Rather than
silently drop the messages, we pass the connection through to a
real SMTP server while modifying key SMTP commands and email
message headers found in the connection stream being passed:

We translate:

  RCPT TO; <someone@somewhere.com> 
	into
  RCPT TO: HOLEADDR
	(where HOLEADDR is as configured in spamhole.h)

We also translate:

  To: Someone <someone@somewhere.com>
	into
  To: HOLEADDR
  X-SpamHole-Ident: IDENT
  X-SpamHole-OrigTo: Someone <someone@somewhere.com>
	(where HOLEADDR and IDENT is as configured in 
	spamhole.h)

This effectively redirects the spam to our spamhole target
address, while tagging it as modified by SpamHole via the IDENT,
and retaining the original To: address for reference.  These
new header entries can be used for filtering and sorting by any
post-processing you may do via the spamhole address.

The SMTP relay that we are redirecting the connection to should
accept messages addressed to our HOLEADDR, otherwise the real
SMTP server will reject the messages and not appear to the 
original SMTP client as an open relay.

This implementation also has a connection threshold configuration
parameter, which will allow a certain number of unmodified
connections to be passed to the real SMTP server per client
IP address.  This option is now depreciated and should usually
be set to 0.  We initially gave spammers way too much credit
and assumed that they would send a test email or two through to
verify that the 'open relay' really delivered messages.  After
having a spamhole running for a few days, it is apparent that
the vast majority of them do not.  This configuration option was
intended to accomodate test messages by allowing a few initial
emails to get through and be delivered, effectively being a
real open relay for a short time.  The risk of running even
a temporary open relay now outweighs the benefit of accomodating
any potential test messages, in our opinion.

If you do choose to use the connection threshold configuration
parameter, you must be sure that the SMTP relay that you are 
redirecting to should allow relaying from the network
address that the spamhole is running on, otherwise the
initial passthrough connections will not properly relay and the
spammer may decide not to use your 'open relay' due to non-
delivery of his verification email.  Setting the maximum good
connections value to 2 should be sufficient to satisfy any
validation tests (see configuration options for more details).


#################################################################
Notes:

This is a REFERENCE IMPLEMENTATION.  You should compile and
run at YOUR OWN RISK.  This code is most likely riddled with
bugs, is obviously un-elegant, and was coded very quickly to
provide a proof-of-concept.  You are highly encouraged to
develop your own spamhole code and share it with others through
this project.  The more platforms supported, the more chaff
the spammers will have to deal with to find the REAL open
SMTP relays.


#################################################################
To install and run:

1. Check your config options at the top of spamhole.h

2. type 'make'

3. Copy spamhole to wherever you would like it to live.

4. Execute spamhole as root, it will drop privs to the UID you 
defined in spamhole.h after binding to it's port (usually 25)


#################################################################
spamhole.h Config Options:

PWD - spamhole's present working directory (where it stores
	it's data.

UID - spamhole's running UID (after binding to the port, it
	drops privs to this user)

IDENT - A text string to uniquely identify this spamhole.
	This string will be added to modified emails via the
	X-SpamHole-Ident header.

LOCAL_IP - The local address you want spamhole to listen on (use
	"0.0.0.0" for all local addresses).

LOCAL_PORT - The local port for spamhole to listen to (usually
	port 25)

SMTP_RELAY - The remote SMTP relay or mail server that
	spamhole will redirect connections to for mail service.

SMTP_PORT - The port that SMTP_RELAY listens on for SMTP.

MAX_CON - The maximum number of unmodified 'passthrough' SMTP
	connections any given IP is allowed to have.  After
	this threshold is reached, incoming SMTP will be
	modified and tagged via X-SpamHole headers.

HOLEADDR - The email address that modified connections will
	have emails re-addressed to.  This should be a SPAM
	mailbox or spam registration address, such as the
	registration address for a Bayesian Filter.

